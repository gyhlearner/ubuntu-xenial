/*
 * net/tipc/bcast.c: TIPC broadcast code
 *
 * Copyright (c) 2004-2006, 2014-2016, Ericsson AB
 * Copyright (c) 2004, Intel Corporation.
 * Copyright (c) 2005, 2010-2011, Wind River Systems
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the names of the copyright holders nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <linux/tipc_config.h>
#include "socket.h"
#include "msg.h"
#include "bcast.h"
#include "link.h"
#include "name_table.h"

#define	BCLINK_WIN_DEFAULT	50	/* bcast link window size (default) */
#define	BCLINK_WIN_MIN	        32	/* bcast minimum link window size */

const char tipc_bclink_name[] = "broadcast-link";

/**
 * struct tipc_bc_base - base structure for keeping broadcast send state
 * @link: broadcast send link structure
 * @inputq: data input queue; will only carry SOCK_WAKEUP messages
 * @dest: array keeping number of reachable destinations per bearer
 * @primary_bearer: a bearer having links to all broadcast destinations, if any
 * @bcast_support: indicates if primary bearer, if any, supports broadcast
 * @rcast_support: indicates if all peer nodes support replicast
 * @rc_ratio: dest count as percentage of cluster size where send method changes
 * @bc_threshold: calculated drom rc_ratio; if dests > threshold use broadcast
 */
struct tipc_bc_base {
	struct tipc_link *link;
	struct sk_buff_head inputq;
	int dests[MAX_BEARERS];
	int primary_bearer;
	bool bcast_support;
	bool rcast_support;
	int rc_ratio;
	int bc_threshold;
};

static struct tipc_bc_base *tipc_bc_base(struct net *net)
{
	return tipc_net(net)->bcbase;
}

int tipc_bcast_get_mtu(struct net *net)
{
	return tipc_link_mtu(tipc_bc_sndlink(net)) - INT_H_SIZE;
}

void tipc_bcast_disable_rcast(struct net *net)
{
	tipc_bc_base(net)->rcast_support = false;
}

static void tipc_bcbase_calc_bc_threshold(struct net *net)
{
	struct tipc_bc_base *bb = tipc_bc_base(net);
	int cluster_size = tipc_link_bc_peers(tipc_bc_sndlink(net));

	bb->bc_threshold = 1 + (cluster_size * bb->rc_ratio / 100);
}

/* tipc_bcbase_select_primary(): find a bearer with links to all destinations,
 *                               if any, and make it primary bearer
 */
static void tipc_bcbase_select_primary(struct net *net)
{
	struct tipc_bc_base *bb = tipc_bc_base(net);
	int all_dests =  tipc_link_bc_peers(bb->link);
	int i, mtu, prim;

	bb->primary_bearer = INVALID_BEARER_ID;
	bb->bcast_support = true;

	if (!all_dests)
		return;

	for (i = 0; i < MAX_BEARERS; i++) {
		if (!bb->dests[i])
			continue;

		mtu = tipc_bearer_mtu(net, i);
		if (mtu < tipc_link_mtu(bb->link))
			tipc_link_set_mtu(bb->link, mtu);
		bb->bcast_support &= tipc_bearer_bcast_support(net, i);
		if (bb->dests[i] < all_dests)
			continue;

		bb->primary_bearer = i;

		/* Reduce risk that all nodes select same primary */
		if ((i ^ tipc_own_addr(net)) & 1)
			break;
	}
	prim = bb->primary_bearer;
	if (prim != INVALID_BEARER_ID)
		bb->bcast_support = tipc_bearer_bcast_support(net, prim);
}

void tipc_bcast_inc_bearer_dst_cnt(struct net *net, int bearer_id)
{
	struct tipc_bc_base *bb = tipc_bc_base(net);

	tipc_bcast_lock(net);
	bb->dests[bearer_id]++;
	tipc_bcbase_select_primary(net);
	tipc_bcast_unlock(net);
}

void tipc_bcast_dec_bearer_dst_cnt(struct net *net, int bearer_id)
{
	struct tipc_bc_base *bb = tipc_bc_base(net);

	tipc_bcast_lock(net);
	bb->dests[bearer_id]--;
	tipc_bcbase_select_primary(net);
	tipc_bcast_unlock(net);
}

/* tipc_bcbase_xmit - broadcast a packet queue across one or more bearers
 *
 * Note that number of reachable destinations, as indicated in the dests[]
 * array, may transitionally differ from the number of destinations indicated
 * in each sent buffer. We can sustain this. Excess destination nodes will
 * drop and never acknowledge the unexpected packets, and missing destinations
 * will either require retransmission (if they are just about to be added to
 * the bearer), or be removed from the buffer's 'ackers' counter (if they
 * just went down)
 */
static void tipc_bcbase_xmit(struct net *net, struct sk_buff_head *xmitq)
{
	int bearer_id;
	struct tipc_bc_base *bb = tipc_bc_base(net);
	struct sk_buff *skb, *_skb;
	struct sk_buff_head _xmitq;

	if (skb_queue_empty(xmitq))
		return;

	/* The typical case: at least one bearer has links to all nodes */
	bearer_id = bb->primary_bearer;
	if (bearer_id >= 0) {
		tipc_bearer_bc_xmit(net, bearer_id, xmitq);
		return;
	}

	/* We have to transmit across all bearers */
	skb_queue_head_init(&_xmitq);
	for (bearer_id = 0; bearer_id < MAX_BEARERS; bearer_id++) {
		if (!bb->dests[bearer_id])
			continue;

		skb_queue_walk(xmitq, skb) {
			_skb = pskb_copy_for_clone(skb, GFP_ATOMIC);
			if (!_skb)
				break;
			__skb_queue_tail(&_xmitq, _skb);
		}
		tipc_bearer_bc_xmit(net, bearer_id, &_xmitq);
	}
	__skb_queue_purge(xmitq);
	__skb_queue_purge(&_xmitq);
}

static void tipc_bcast_select_xmit_method(struct net *net, int dests,
					  struct tipc_mc_method *method)
{
	struct tipc_bc_base *bb = tipc_bc_base(net);
	unsigned long exp = method->expires;

	/* Broadcast supported by used bearer/bearers? */
	if (!bb->bcast_support) {
		method->rcast = true;
		return;
	}
	/* Any destinations which don't support replicast ? */
	if (!bb->rcast_support) {
		method->rcast = false;
		return;
	}
	/* Can current method be changed ? */
	method->expires = jiffies + TIPC_METHOD_EXPIRE;
	if (method->mandatory || time_before(jiffies, exp))
		return;

	/* Determine method to use now */
	method->rcast = dests <= bb->bc_threshold;
}

/* tipc_bcast_xmit - broadcast the buffer chain to all external nodes
 * @net: the applicable net namespace
 * @pkts: chain of buffers containing message
 * @cong_link_cnt: set to 1 if broadcast link is congested, otherwise 0
 * Consumes the buffer chain.
 * Returns 0 if success, otherwise errno: -EHOSTUNREACH,-EMSGSIZE
 */
static int tipc_bcast_xmit(struct net *net, struct sk_buff_head *pkts,
			   u16 *cong_link_cnt)
{
	struct tipc_link *l = tipc_bc_sndlink(net);
	struct sk_buff_head xmitq;
	int rc = 0;

	skb_queue_head_init(&xmitq);
	tipc_bcast_lock(net);
	if (tipc_link_bc_peers(l))
		rc = tipc_link_xmit(l, pkts, &xmitq);
	tipc_bcast_unlock(net);
	tipc_bcbase_xmit(net, &xmitq);
	__skb_queue_purge(pkts);
	if (rc == -ELINKCONG) {
		*cong_link_cnt = 1;
		rc = 0;
	}
	return rc;
}

/* tipc_rcast_xmit - replicate and send a message to given destination nodes
 * @net: the applicable net namespace
 * @pkts: chain of buffers containing message
 * @dests: list of destination nodes
 * @cong_link_cnt: returns number of congested links
 * @cong_links: returns identities of congested links
 * Returns 0 if success, otherwise errno
 */
static int tipc_rcast_xmit(struct net *net, struct sk_buff_head *pkts,
			   struct tipc_nlist *dests, u16 *cong_link_cnt)
{
	struct tipc_dest *dst, *tmp;
	struct sk_buff_head _pkts;
	u32 dnode, selector;

	selector = msg_link_selector(buf_msg(skb_peek(pkts)));
	skb_queue_head_init(&_pkts);

	list_for_each_entry_safe(dst, tmp, &dests->list, list) {
		dnode = dst->node;
		if (!tipc_msg_pskb_copy(dnode, pkts, &_pkts))
			return -ENOMEM;

		/* Any other return value than -ELINKCONG is ignored */
		if (tipc_node_xmit(net, &_pkts, dnode, selector) == -ELINKCONG)
			(*cong_link_cnt)++;
	}
	return 0;
}

/* tipc_mcast_xmit - deliver message to indicated destination nodes
 *                   and to identified node local sockets
 * @net: the applicable net namespace
 * @pkts: chain of buffers containing message
 * @method: send method to be used
 * @dests: destination nodes for message.
 * @cong_link_cnt: returns number of encountered congested destination links
 * Consumes buffer chain.
 * Returns 0 if success, otherwise errno
 */
int tipc_mcast_xmit(struct net *net, struct sk_buff_head *pkts,
		    struct tipc_mc_method *method, struct tipc_nlist *dests,
		    u16 *cong_link_cnt)
{
	struct sk_buff_head inputq, localq;
	int rc = 0;

	skb_queue_head_init(&inputq);
	skb_queue_head_init(&localq);

	/* Clone packets before they are consumed by next call */
	if (dests->local && !tipc_msg_reassemble(pkts, &localq)) {
		rc = -ENOMEM;
		goto exit;
	}
	/* Send according to determined transmit method */
	if (dests->remote) {
		tipc_bcast_select_xmit_method(net, dests->remote, method);
		if (method->rcast)
			rc = tipc_rcast_xmit(net, pkts, dests, cong_link_cnt);
		else
			rc = tipc_bcast_xmit(net, pkts, cong_link_cnt);
	}

	if (dests->local)
		tipc_sk_mcast_rcv(net, &localq, &inputq);
exit:
	/* This queue should normally be empty by now */
	__skb_queue_purge(pkts);
	return rc;
}

/* tipc_bcast_rcv - receive a broadcast packet, and deliver to rcv link
 *
 * RCU is locked, no other locks set
 */
int tipc_bcast_rcv(struct net *net, struct tipc_link *l, struct sk_buff *skb)
{
	struct tipc_msg *hdr = buf_msg(skb);
	struct sk_buff_head *inputq = &tipc_bc_base(net)->inputq;
	struct sk_buff_head xmitq;
	int rc;

	__skb_queue_head_init(&xmitq);

	if (msg_mc_netid(hdr) != tipc_netid(net) || !tipc_link_is_up(l)) {
		kfree_skb(skb);
		return 0;
	}

	tipc_bcast_lock(net);
	if (msg_user(hdr) == BCAST_PROTOCOL)
		rc = tipc_link_bc_nack_rcv(l, skb, &xmitq);
	else
		rc = tipc_link_rcv(l, skb, NULL);
	tipc_bcast_unlock(net);

	tipc_bcbase_xmit(net, &xmitq);

	/* Any socket wakeup messages ? */
	if (!skb_queue_empty(inputq))
		tipc_sk_rcv(net, inputq);

	return rc;
}

/* tipc_bcast_ack_rcv - receive and handle a broadcast acknowledge
 *
 * RCU is locked, no other locks set
 */
void tipc_bcast_ack_rcv(struct net *net, struct tipc_link *l,
			struct tipc_msg *hdr)
{
	struct sk_buff_head *inputq = &tipc_bc_base(net)->inputq;
	u16 acked = msg_bcast_ack(hdr);
	struct sk_buff_head xmitq;

	/* Ignore bc acks sent by peer before bcast synch point was received */
	if (msg_bc_ack_invalid(hdr))
		return;

	__skb_queue_head_init(&xmitq);

	tipc_bcast_lock(net);
	tipc_link_bc_ack_rcv(l, acked, &xmitq);
	tipc_bcast_unlock(net);

	tipc_bcbase_xmit(net, &xmitq);

	/* Any socket wakeup messages ? */
	if (!skb_queue_empty(inputq))
		tipc_sk_rcv(net, inputq);
}

/* tipc_bcast_synch_rcv -  check and update rcv link with peer's send state
 *
 * RCU is locked, no other locks set
 */
int tipc_bcast_sync_rcv(struct net *net, struct tipc_link *l,
			struct tipc_msg *hdr)
{
	struct sk_buff_head *inputq = &tipc_bc_base(net)->inputq;
	struct sk_buff_head xmitq;
	int rc = 0;

	__skb_queue_head_init(&xmitq);

	tipc_bcast_lock(net);
	if (msg_type(hdr) != STATE_MSG) {
		tipc_link_bc_init_rcv(l, hdr);
	} else if (!msg_bc_ack_invalid(hdr)) {
		tipc_link_bc_ack_rcv(l, msg_bcast_ack(hdr), &xmitq);
		rc = tipc_link_bc_sync_rcv(l, hdr, &xmitq);
	}
	tipc_bcast_unlock(net);

	tipc_bcbase_xmit(net, &xmitq);

	/* Any socket wakeup messages ? */
	if (!skb_queue_empty(inputq))
		tipc_sk_rcv(net, inputq);
	return rc;
}

/* tipc_bcast_add_peer - add a peer node to broadcast link and bearer
 *
 * RCU is locked, node lock is set
 */
void tipc_bcast_add_peer(struct net *net, struct tipc_link *uc_l,
			 struct sk_buff_head *xmitq)
{
	struct tipc_link *snd_l = tipc_bc_sndlink(net);

	tipc_bcast_lock(net);
	tipc_link_add_bc_peer(snd_l, uc_l, xmitq);
	tipc_bcbase_select_primary(net);
	tipc_bcbase_calc_bc_threshold(net);
	tipc_bcast_unlock(net);
}

/* tipc_bcast_remove_peer - remove a peer node from broadcast link and bearer
 *
 * RCU is locked, node lock is set
 */
void tipc_bcast_remove_peer(struct net *net, struct tipc_link *rcv_l)
{
	struct tipc_link *snd_l = tipc_bc_sndlink(net);
	struct sk_buff_head *inputq = &tipc_bc_base(net)->inputq;
	struct sk_buff_head xmitq;

	__skb_queue_head_init(&xmitq);

	tipc_bcast_lock(net);
	tipc_link_remove_bc_peer(snd_l, rcv_l, &xmitq);
	tipc_bcbase_select_primary(net);
	tipc_bcbase_calc_bc_threshold(net);
	tipc_bcast_unlock(net);

	tipc_bcbase_xmit(net, &xmitq);

	/* Any socket wakeup messages ? */
	if (!skb_queue_empty(inputq))
		tipc_sk_rcv(net, inputq);
}

<<<<<<< HEAD
static int __tipc_nl_add_bc_link_stat(struct sk_buff *skb,
				      struct tipc_stats *stats)
{
	int i;
	struct nlattr *nest;

	struct nla_map {
		__u32 key;
		__u32 val;
	};

	struct nla_map map[] = {
		{TIPC_NLA_STATS_RX_INFO, stats->recv_info},
		{TIPC_NLA_STATS_RX_FRAGMENTS, stats->recv_fragments},
		{TIPC_NLA_STATS_RX_FRAGMENTED, stats->recv_fragmented},
		{TIPC_NLA_STATS_RX_BUNDLES, stats->recv_bundles},
		{TIPC_NLA_STATS_RX_BUNDLED, stats->recv_bundled},
		{TIPC_NLA_STATS_TX_INFO, stats->sent_info},
		{TIPC_NLA_STATS_TX_FRAGMENTS, stats->sent_fragments},
		{TIPC_NLA_STATS_TX_FRAGMENTED, stats->sent_fragmented},
		{TIPC_NLA_STATS_TX_BUNDLES, stats->sent_bundles},
		{TIPC_NLA_STATS_TX_BUNDLED, stats->sent_bundled},
		{TIPC_NLA_STATS_RX_NACKS, stats->recv_nacks},
		{TIPC_NLA_STATS_RX_DEFERRED, stats->deferred_recv},
		{TIPC_NLA_STATS_TX_NACKS, stats->sent_nacks},
		{TIPC_NLA_STATS_TX_ACKS, stats->sent_acks},
		{TIPC_NLA_STATS_RETRANSMITTED, stats->retransmitted},
		{TIPC_NLA_STATS_DUPLICATES, stats->duplicates},
		{TIPC_NLA_STATS_LINK_CONGS, stats->link_congs},
		{TIPC_NLA_STATS_MAX_QUEUE, stats->max_queue_sz},
		{TIPC_NLA_STATS_AVG_QUEUE, stats->queue_sz_counts ?
			(stats->accu_queue_sz / stats->queue_sz_counts) : 0}
	};

	nest = nla_nest_start(skb, TIPC_NLA_LINK_STATS);
	if (!nest)
		return -EMSGSIZE;

	for (i = 0; i <  ARRAY_SIZE(map); i++)
		if (nla_put_u32(skb, map[i].key, map[i].val))
			goto msg_full;

	nla_nest_end(skb, nest);

	return 0;
msg_full:
	nla_nest_cancel(skb, nest);

	return -EMSGSIZE;
}

int tipc_nl_add_bc_link(struct net *net, struct tipc_nl_msg *msg)
{
	int err;
	void *hdr;
	struct nlattr *attrs;
	struct nlattr *prop;
	struct tipc_net *tn = net_generic(net, tipc_net_id);
	struct tipc_link *bcl = tn->bcl;

	if (!bcl)
		return 0;

	tipc_bcast_lock(net);

	hdr = genlmsg_put(msg->skb, msg->portid, msg->seq, &tipc_genl_family,
			  NLM_F_MULTI, TIPC_NL_LINK_GET);
	if (!hdr) {
		tipc_bcast_unlock(net);
		return -EMSGSIZE;
	}

	attrs = nla_nest_start(msg->skb, TIPC_NLA_LINK);
	if (!attrs)
		goto msg_full;

	/* The broadcast link is always up */
	if (nla_put_flag(msg->skb, TIPC_NLA_LINK_UP))
		goto attr_msg_full;

	if (nla_put_flag(msg->skb, TIPC_NLA_LINK_BROADCAST))
		goto attr_msg_full;
	if (nla_put_string(msg->skb, TIPC_NLA_LINK_NAME, bcl->name))
		goto attr_msg_full;
	if (nla_put_u32(msg->skb, TIPC_NLA_LINK_RX, bcl->rcv_nxt))
		goto attr_msg_full;
	if (nla_put_u32(msg->skb, TIPC_NLA_LINK_TX, bcl->snd_nxt))
		goto attr_msg_full;

	prop = nla_nest_start(msg->skb, TIPC_NLA_LINK_PROP);
	if (!prop)
		goto attr_msg_full;
	if (nla_put_u32(msg->skb, TIPC_NLA_PROP_WIN, bcl->window))
		goto prop_msg_full;
	nla_nest_end(msg->skb, prop);

	err = __tipc_nl_add_bc_link_stat(msg->skb, &bcl->stats);
	if (err)
		goto attr_msg_full;

	tipc_bcast_unlock(net);
	nla_nest_end(msg->skb, attrs);
	genlmsg_end(msg->skb, hdr);

	return 0;

prop_msg_full:
	nla_nest_cancel(msg->skb, prop);
attr_msg_full:
	nla_nest_cancel(msg->skb, attrs);
msg_full:
	tipc_bcast_unlock(net);
	genlmsg_cancel(msg->skb, hdr);

	return -EMSGSIZE;
}

=======
>>>>>>> temp
int tipc_bclink_reset_stats(struct net *net)
{
	struct tipc_link *l = tipc_bc_sndlink(net);

	if (!l)
		return -ENOPROTOOPT;

	tipc_bcast_lock(net);
	tipc_link_reset_stats(l);
	tipc_bcast_unlock(net);
	return 0;
}

static int tipc_bc_link_set_queue_limits(struct net *net, u32 limit)
{
	struct tipc_link *l = tipc_bc_sndlink(net);

	if (!l)
		return -ENOPROTOOPT;
	if (limit < BCLINK_WIN_MIN)
		limit = BCLINK_WIN_MIN;
	if (limit > TIPC_MAX_LINK_WIN)
		return -EINVAL;
	tipc_bcast_lock(net);
	tipc_link_set_queue_limits(l, limit);
	tipc_bcast_unlock(net);
	return 0;
}

int tipc_nl_bc_link_set(struct net *net, struct nlattr *attrs[])
{
	int err;
	u32 win;
	struct nlattr *props[TIPC_NLA_PROP_MAX + 1];

	if (!attrs[TIPC_NLA_LINK_PROP])
		return -EINVAL;

	err = tipc_nl_parse_link_prop(attrs[TIPC_NLA_LINK_PROP], props);
	if (err)
		return err;

	if (!props[TIPC_NLA_PROP_WIN])
		return -EOPNOTSUPP;

	win = nla_get_u32(props[TIPC_NLA_PROP_WIN]);

	return tipc_bc_link_set_queue_limits(net, win);
}

int tipc_bcast_init(struct net *net)
{
	struct tipc_net *tn = tipc_net(net);
	struct tipc_bc_base *bb = NULL;
	struct tipc_link *l = NULL;

	bb = kzalloc(sizeof(*bb), GFP_ATOMIC);
	if (!bb)
		goto enomem;
	tn->bcbase = bb;
	spin_lock_init(&tipc_net(net)->bclock);

	if (!tipc_link_bc_create(net, 0, 0,
				 U16_MAX,
				 BCLINK_WIN_DEFAULT,
				 0,
				 &bb->inputq,
				 NULL,
				 NULL,
				 &l))
		goto enomem;
	bb->link = l;
	tn->bcl = l;
	bb->rc_ratio = 25;
	bb->rcast_support = true;
	return 0;
enomem:
	kfree(bb);
	kfree(l);
	return -ENOMEM;
}

void tipc_bcast_stop(struct net *net)
{
	struct tipc_net *tn = net_generic(net, tipc_net_id);

	synchronize_net();
	kfree(tn->bcbase);
	kfree(tn->bcl);
}

void tipc_nlist_init(struct tipc_nlist *nl, u32 self)
{
	memset(nl, 0, sizeof(*nl));
	INIT_LIST_HEAD(&nl->list);
	nl->self = self;
}

void tipc_nlist_add(struct tipc_nlist *nl, u32 node)
{
	if (node == nl->self)
		nl->local = true;
	else if (tipc_dest_push(&nl->list, node, 0))
		nl->remote++;
}

void tipc_nlist_del(struct tipc_nlist *nl, u32 node)
{
	if (node == nl->self)
		nl->local = false;
	else if (tipc_dest_del(&nl->list, node, 0))
		nl->remote--;
}

void tipc_nlist_purge(struct tipc_nlist *nl)
{
	tipc_dest_list_purge(&nl->list);
	nl->remote = 0;
	nl->local = 0;
}
