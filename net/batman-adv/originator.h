/* Copyright (C) 2007-2017  B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _NET_BATMAN_ADV_ORIGINATOR_H_
#define _NET_BATMAN_ADV_ORIGINATOR_H_

#include "main.h"

#include <linux/compiler.h>
#include <linux/if_ether.h>
#include <linux/jhash.h>
#include <linux/kref.h>
#include <linux/rculist.h>
#include <linux/rcupdate.h>
#include <linux/stddef.h>
#include <linux/types.h>

#include "hash.h"

struct netlink_callback;
struct seq_file;
struct sk_buff;

bool batadv_compare_orig(const struct hlist_node *node, const void *data2);
int batadv_originator_init(struct batadv_priv *bat_priv);
void batadv_originator_free(struct batadv_priv *bat_priv);
void batadv_purge_orig_ref(struct batadv_priv *bat_priv);
<<<<<<< HEAD
void batadv_orig_node_free_ref(struct batadv_orig_node *orig_node);
=======
void batadv_orig_node_put(struct batadv_orig_node *orig_node);
>>>>>>> temp
struct batadv_orig_node *batadv_orig_node_new(struct batadv_priv *bat_priv,
					      const u8 *addr);
struct batadv_hardif_neigh_node *
batadv_hardif_neigh_get(const struct batadv_hard_iface *hard_iface,
			const u8 *neigh_addr);
void
batadv_hardif_neigh_put(struct batadv_hardif_neigh_node *hardif_neigh);
struct batadv_neigh_node *
batadv_neigh_node_get_or_create(struct batadv_orig_node *orig_node,
				struct batadv_hard_iface *hard_iface,
				const u8 *neigh_addr);
void batadv_neigh_node_put(struct batadv_neigh_node *neigh_node);
struct batadv_neigh_node *
batadv_orig_router_get(struct batadv_orig_node *orig_node,
		       const struct batadv_hard_iface *if_outgoing);
struct batadv_neigh_ifinfo *
batadv_neigh_ifinfo_new(struct batadv_neigh_node *neigh,
			struct batadv_hard_iface *if_outgoing);
struct batadv_neigh_ifinfo *
batadv_neigh_ifinfo_get(struct batadv_neigh_node *neigh,
			struct batadv_hard_iface *if_outgoing);
void batadv_neigh_ifinfo_put(struct batadv_neigh_ifinfo *neigh_ifinfo);

int batadv_hardif_neigh_dump(struct sk_buff *msg, struct netlink_callback *cb);
int batadv_hardif_neigh_seq_print_text(struct seq_file *seq, void *offset);

struct batadv_orig_ifinfo *
batadv_orig_ifinfo_get(struct batadv_orig_node *orig_node,
		       struct batadv_hard_iface *if_outgoing);
struct batadv_orig_ifinfo *
batadv_orig_ifinfo_new(struct batadv_orig_node *orig_node,
		       struct batadv_hard_iface *if_outgoing);
void batadv_orig_ifinfo_put(struct batadv_orig_ifinfo *orig_ifinfo);

int batadv_orig_seq_print_text(struct seq_file *seq, void *offset);
int batadv_orig_dump(struct sk_buff *msg, struct netlink_callback *cb);
int batadv_orig_hardif_seq_print_text(struct seq_file *seq, void *offset);
int batadv_orig_hash_add_if(struct batadv_hard_iface *hard_iface,
			    int max_if_num);
int batadv_orig_hash_del_if(struct batadv_hard_iface *hard_iface,
			    int max_if_num);
struct batadv_orig_node_vlan *
batadv_orig_node_vlan_new(struct batadv_orig_node *orig_node,
			  unsigned short vid);
struct batadv_orig_node_vlan *
batadv_orig_node_vlan_get(struct batadv_orig_node *orig_node,
			  unsigned short vid);
void batadv_orig_node_vlan_put(struct batadv_orig_node_vlan *orig_vlan);

/* hashfunction to choose an entry in a hash table of given size
 * hash algorithm from http://en.wikipedia.org/wiki/Hash_table
 */
static inline u32 batadv_choose_orig(const void *data, u32 size)
{
	u32 hash = 0;

	hash = jhash(data, ETH_ALEN, hash);
	return hash % size;
}

static inline struct batadv_orig_node *
batadv_orig_hash_find(struct batadv_priv *bat_priv, const void *data)
{
	struct batadv_hashtable *hash = bat_priv->orig_hash;
	struct hlist_head *head;
	struct batadv_orig_node *orig_node, *orig_node_tmp = NULL;
	int index;

	if (!hash)
		return NULL;

	index = batadv_choose_orig(data, hash->size);
	head = &hash->table[index];

	rcu_read_lock();
	hlist_for_each_entry_rcu(orig_node, head, hash_entry) {
		if (!batadv_compare_eth(orig_node, data))
			continue;

		if (!kref_get_unless_zero(&orig_node->refcount))
			continue;

		orig_node_tmp = orig_node;
		break;
	}
	rcu_read_unlock();

	return orig_node_tmp;
}

#endif /* _NET_BATMAN_ADV_ORIGINATOR_H_ */
