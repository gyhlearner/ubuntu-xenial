/*
 * Copyright (c) 2002, 2007 Red Hat, Inc. All rights reserved.
 *
 * This software may be freely redistributed under the terms of the
 * GNU General Public License.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Authors: David Woodhouse <dwmw2@infradead.org>
 *          David Howells <dhowells@redhat.com>
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/circ_buf.h>
#include <linux/sched.h>
#include "internal.h"

/*
 * Set up an interest-in-callbacks record for a volume on a server and
 * register it with the server.
 * - Called with vnode->io_lock held.
 */
int afs_register_server_cb_interest(struct afs_vnode *vnode,
				    struct afs_server_list *slist,
				    unsigned int index)
{
	struct afs_server_entry *entry = &slist->servers[index];
	struct afs_cb_interest *cbi, *vcbi, *new, *old;
	struct afs_server *server = entry->server;

again:
	if (vnode->cb_interest &&
	    likely(vnode->cb_interest == entry->cb_interest))
		return 0;

	read_lock(&slist->lock);
	cbi = afs_get_cb_interest(entry->cb_interest);
	read_unlock(&slist->lock);

	vcbi = vnode->cb_interest;
	if (vcbi) {
		if (vcbi == cbi) {
			afs_put_cb_interest(afs_v2net(vnode), cbi);
			return 0;
		}

		/* Use a new interest in the server list for the same server
		 * rather than an old one that's still attached to a vnode.
		 */
		if (cbi && vcbi->server == cbi->server) {
			write_seqlock(&vnode->cb_lock);
			old = vnode->cb_interest;
			vnode->cb_interest = cbi;
			write_sequnlock(&vnode->cb_lock);
			afs_put_cb_interest(afs_v2net(vnode), old);
			return 0;
		}

		/* Re-use the one attached to the vnode. */
		if (!cbi && vcbi->server == server) {
			write_lock(&slist->lock);
			if (entry->cb_interest) {
				write_unlock(&slist->lock);
				afs_put_cb_interest(afs_v2net(vnode), cbi);
				goto again;
			}

			entry->cb_interest = cbi;
			write_unlock(&slist->lock);
			return 0;
		}
	}

	if (!cbi) {
		new = kzalloc(sizeof(struct afs_cb_interest), GFP_KERNEL);
		if (!new)
			return -ENOMEM;

		refcount_set(&new->usage, 1);
		new->sb = vnode->vfs_inode.i_sb;
		new->vid = vnode->volume->vid;
		new->server = afs_get_server(server);
		INIT_LIST_HEAD(&new->cb_link);

		write_lock(&server->cb_break_lock);
		list_add_tail(&new->cb_link, &server->cb_interests);
		write_unlock(&server->cb_break_lock);

		write_lock(&slist->lock);
		if (!entry->cb_interest) {
			entry->cb_interest = afs_get_cb_interest(new);
			cbi = new;
			new = NULL;
		} else {
			cbi = afs_get_cb_interest(entry->cb_interest);
		}
		write_unlock(&slist->lock);
		afs_put_cb_interest(afs_v2net(vnode), new);
	}

	ASSERT(cbi);

	/* Change the server the vnode is using.  This entails scrubbing any
	 * interest the vnode had in the previous server it was using.
	 */
	write_seqlock(&vnode->cb_lock);

	old = vnode->cb_interest;
	vnode->cb_interest = cbi;
	vnode->cb_s_break = cbi->server->cb_s_break;
	clear_bit(AFS_VNODE_CB_PROMISED, &vnode->flags);

	write_sequnlock(&vnode->cb_lock);
	afs_put_cb_interest(afs_v2net(vnode), old);
	return 0;
}

/*
 * Set a vnode's interest on a server.
 */
void afs_set_cb_interest(struct afs_vnode *vnode, struct afs_cb_interest *cbi)
{
	struct afs_cb_interest *old_cbi = NULL;

	if (vnode->cb_interest == cbi)
		return;

	write_seqlock(&vnode->cb_lock);
	if (vnode->cb_interest != cbi) {
		afs_get_cb_interest(cbi);
		old_cbi = vnode->cb_interest;
		vnode->cb_interest = cbi;
	}
	write_sequnlock(&vnode->cb_lock);
	afs_put_cb_interest(afs_v2net(vnode), cbi);
}

/*
 * Remove an interest on a server.
 */
void afs_put_cb_interest(struct afs_net *net, struct afs_cb_interest *cbi)
{
	if (cbi && refcount_dec_and_test(&cbi->usage)) {
		if (!list_empty(&cbi->cb_link)) {
			write_lock(&cbi->server->cb_break_lock);
			list_del_init(&cbi->cb_link);
			write_unlock(&cbi->server->cb_break_lock);
			afs_put_server(net, cbi->server);
		}
		kfree(cbi);
	}
}

/*
 * allow the fileserver to request callback state (re-)initialisation
 */
void afs_init_callback_state(struct afs_server *server)
{
	if (!test_and_clear_bit(AFS_SERVER_FL_NEW, &server->flags))
		server->cb_s_break++;
}

/*
 * actually break a callback
 */
void afs_break_callback(struct afs_vnode *vnode)
{
	_enter("");

	write_seqlock(&vnode->cb_lock);

	if (test_and_clear_bit(AFS_VNODE_CB_PROMISED, &vnode->flags)) {
		vnode->cb_break++;
		afs_clear_permits(vnode);

		spin_lock(&vnode->lock);

		_debug("break callback");

		if (list_empty(&vnode->granted_locks) &&
		    !list_empty(&vnode->pending_locks))
			afs_lock_may_be_available(vnode);
		spin_unlock(&vnode->lock);
	}

	write_sequnlock(&vnode->cb_lock);
}

/*
 * allow the fileserver to explicitly break one callback
 * - happens when
 *   - the backing file is changed
 *   - a lock is released
 */
static void afs_break_one_callback(struct afs_server *server,
				   struct afs_fid *fid)
{
	struct afs_cb_interest *cbi;
	struct afs_iget_data data;
	struct afs_vnode *vnode;
	struct inode *inode;

	read_lock(&server->cb_break_lock);

	/* Step through all interested superblocks.  There may be more than one
	 * because of cell aliasing.
	 */
	list_for_each_entry(cbi, &server->cb_interests, cb_link) {
		if (cbi->vid != fid->vid)
			continue;

		data.volume = NULL;
		data.fid = *fid;
		inode = ilookup5_nowait(cbi->sb, fid->vnode, afs_iget5_test, &data);
		if (inode) {
			vnode = AFS_FS_I(inode);
			afs_break_callback(vnode);
			iput(inode);
		}
	}

	read_unlock(&server->cb_break_lock);
}

/*
 * allow the fileserver to break callback promises
 */
void afs_break_callbacks(struct afs_server *server, size_t count,
			 struct afs_callback callbacks[])
{
	_enter("%p,%zu,", server, count);

	ASSERT(server != NULL);
	ASSERTCMP(count, <=, AFSCBMAX);

	for (; count > 0; callbacks++, count--) {
		_debug("- Fid { vl=%08x n=%u u=%u }  CB { v=%u x=%u t=%u }",
		       callbacks->fid.vid,
		       callbacks->fid.vnode,
		       callbacks->fid.unique,
		       callbacks->version,
		       callbacks->expiry,
		       callbacks->type
		       );
		afs_break_one_callback(server, &callbacks->fid);
	}

	_leave("");
	return;
}

/*
 * Clear the callback interests in a server list.
 */
void afs_clear_callback_interests(struct afs_net *net, struct afs_server_list *slist)
{
	int i;

	for (i = 0; i < slist->nr_servers; i++) {
		afs_put_cb_interest(net, slist->servers[i].cb_interest);
		slist->servers[i].cb_interest = NULL;
	}
<<<<<<< HEAD
	spin_unlock(&server->cb_lock);
	_leave("");
}

/*
 * give up the callback registered for a vnode on the file server when the
 * inode is being cleared
 */
void afs_give_up_callback(struct afs_vnode *vnode)
{
	struct afs_server *server = vnode->server;

	DECLARE_WAITQUEUE(myself, current);

	_enter("%d", vnode->cb_promised);

	_debug("GIVE UP INODE %p", &vnode->vfs_inode);

	if (!vnode->cb_promised) {
		_leave(" [not promised]");
		return;
	}

	ASSERT(server != NULL);

	spin_lock(&server->cb_lock);
	if (vnode->cb_promised && afs_breakring_space(server) == 0) {
		add_wait_queue(&server->cb_break_waitq, &myself);
		for (;;) {
			set_current_state(TASK_UNINTERRUPTIBLE);
			if (!vnode->cb_promised ||
			    afs_breakring_space(server) != 0)
				break;
			spin_unlock(&server->cb_lock);
			schedule();
			spin_lock(&server->cb_lock);
		}
		remove_wait_queue(&server->cb_break_waitq, &myself);
		__set_current_state(TASK_RUNNING);
	}

	/* of course, it's always possible for the server to break this vnode's
	 * callback first... */
	if (vnode->cb_promised)
		afs_do_give_up_callback(server, vnode);

	spin_unlock(&server->cb_lock);
	_leave("");
}

/*
 * dispatch a deferred give up callbacks operation
 */
void afs_dispatch_give_up_callbacks(struct work_struct *work)
{
	struct afs_server *server =
		container_of(work, struct afs_server, cb_break_work.work);

	_enter("");

	/* tell the fileserver to discard the callback promises it has
	 * - in the event of ENOMEM or some other error, we just forget that we
	 *   had callbacks entirely, and the server will call us later to break
	 *   them
	 */
	afs_fs_give_up_callbacks(server, &afs_async_call);
}

/*
 * flush the outstanding callback breaks on a server
 */
void afs_flush_callback_breaks(struct afs_server *server)
{
	mod_delayed_work(afs_callback_update_worker, &server->cb_break_work, 0);
}

#if 0
/*
 * update a bunch of callbacks
 */
static void afs_callback_updater(struct work_struct *work)
{
	struct afs_server *server;
	struct afs_vnode *vnode, *xvnode;
	time64_t now;
	long timeout;
	int ret;

	server = container_of(work, struct afs_server, updater);

	_enter("");

	now = ktime_get_real_seconds();

	/* find the first vnode to update */
	spin_lock(&server->cb_lock);
	for (;;) {
		if (RB_EMPTY_ROOT(&server->cb_promises)) {
			spin_unlock(&server->cb_lock);
			_leave(" [nothing]");
			return;
		}

		vnode = rb_entry(rb_first(&server->cb_promises),
				 struct afs_vnode, cb_promise);
		if (atomic_read(&vnode->usage) > 0)
			break;
		rb_erase(&vnode->cb_promise, &server->cb_promises);
		vnode->cb_promised = false;
	}

	timeout = vnode->update_at - now;
	if (timeout > 0) {
		queue_delayed_work(afs_vnode_update_worker,
				   &afs_vnode_update, timeout * HZ);
		spin_unlock(&server->cb_lock);
		_leave(" [nothing]");
		return;
	}

	list_del_init(&vnode->update);
	atomic_inc(&vnode->usage);
	spin_unlock(&server->cb_lock);

	/* we can now perform the update */
	_debug("update %s", vnode->vldb.name);
	vnode->state = AFS_VL_UPDATING;
	vnode->upd_rej_cnt = 0;
	vnode->upd_busy_cnt = 0;

	ret = afs_vnode_update_record(vl, &vldb);
	switch (ret) {
	case 0:
		afs_vnode_apply_update(vl, &vldb);
		vnode->state = AFS_VL_UPDATING;
		break;
	case -ENOMEDIUM:
		vnode->state = AFS_VL_VOLUME_DELETED;
		break;
	default:
		vnode->state = AFS_VL_UNCERTAIN;
		break;
	}

	/* and then reschedule */
	_debug("reschedule");
	vnode->update_at = ktime_get_real_seconds() +
			afs_vnode_update_timeout;

	spin_lock(&server->cb_lock);

	if (!list_empty(&server->cb_promises)) {
		/* next update in 10 minutes, but wait at least 1 second more
		 * than the newest record already queued so that we don't spam
		 * the VL server suddenly with lots of requests
		 */
		xvnode = list_entry(server->cb_promises.prev,
				    struct afs_vnode, update);
		if (vnode->update_at <= xvnode->update_at)
			vnode->update_at = xvnode->update_at + 1;
		xvnode = list_entry(server->cb_promises.next,
				    struct afs_vnode, update);
		timeout = xvnode->update_at - now;
		if (timeout < 0)
			timeout = 0;
	} else {
		timeout = afs_vnode_update_timeout;
	}

	list_add_tail(&vnode->update, &server->cb_promises);

	_debug("timeout %ld", timeout);
	queue_delayed_work(afs_vnode_update_worker,
			   &afs_vnode_update, timeout * HZ);
	spin_unlock(&server->cb_lock);
	afs_put_vnode(vl);
}
#endif

/*
 * initialise the callback update process
 */
int __init afs_callback_update_init(void)
{
	afs_callback_update_worker =
		create_singlethread_workqueue("kafs_callbackd");
	return afs_callback_update_worker ? 0 : -ENOMEM;
}

/*
 * shut down the callback update process
 */
void afs_callback_update_kill(void)
{
	destroy_workqueue(afs_callback_update_worker);
=======
>>>>>>> temp
}
