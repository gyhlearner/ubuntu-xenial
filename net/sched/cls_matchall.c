/*
 * net/sched/cls_matchll.c		Match-all classifier
 *
 * Copyright (c) 2016 Jiri Pirko <jiri@mellanox.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

#include <net/sch_generic.h>
#include <net/pkt_cls.h>

<<<<<<< HEAD
struct cls_mall_filter {
	struct tcf_exts exts;
	struct tcf_result res;
	u32 handle;
	struct rcu_head	rcu;
};

struct cls_mall_head {
	struct cls_mall_filter *filter;
	struct rcu_head	rcu;
=======
struct cls_mall_head {
	struct tcf_exts exts;
	struct tcf_result res;
	u32 handle;
	u32 flags;
	union {
		struct work_struct work;
		struct rcu_head	rcu;
	};
>>>>>>> temp
};

static int mall_classify(struct sk_buff *skb, const struct tcf_proto *tp,
			 struct tcf_result *res)
{
	struct cls_mall_head *head = rcu_dereference_bh(tp->root);
<<<<<<< HEAD
	struct cls_mall_filter *f = head->filter;

	return tcf_exts_exec(skb, &f->exts, res);
=======

	if (tc_skip_sw(head->flags))
		return -1;

	*res = head->res;
	return tcf_exts_exec(skb, &head->exts, res);
>>>>>>> temp
}

static int mall_init(struct tcf_proto *tp)
{
<<<<<<< HEAD
	struct cls_mall_head *head;

	head = kzalloc(sizeof(*head), GFP_KERNEL);
	if (!head)
		return -ENOBUFS;

	rcu_assign_pointer(tp->root, head);

	return 0;
}

static void mall_destroy_filter(struct rcu_head *head)
{
	struct cls_mall_filter *f = container_of(head, struct cls_mall_filter, rcu);

	tcf_exts_destroy(&f->exts);
	kfree(f);
}

static bool mall_destroy(struct tcf_proto *tp, bool force)
{
	struct cls_mall_head *head = rtnl_dereference(tp->root);

	if (!force && head->filter)
		return false;

	if (head->filter)
		call_rcu(&head->filter->rcu, mall_destroy_filter);
	RCU_INIT_POINTER(tp->root, NULL);
	kfree_rcu(head, rcu);
	return true;
}

static unsigned long mall_get(struct tcf_proto *tp, u32 handle)
{
	struct cls_mall_head *head = rtnl_dereference(tp->root);
	struct cls_mall_filter *f = head->filter;

	if (f && f->handle == handle)
		return (unsigned long) f;
	return 0;
=======
	return 0;
}

static void __mall_destroy(struct cls_mall_head *head)
{
	tcf_exts_destroy(&head->exts);
	tcf_exts_put_net(&head->exts);
	kfree(head);
}

static void mall_destroy_work(struct work_struct *work)
{
	struct cls_mall_head *head = container_of(work, struct cls_mall_head,
						  work);
	rtnl_lock();
	__mall_destroy(head);
	rtnl_unlock();
}

static void mall_destroy_rcu(struct rcu_head *rcu)
{
	struct cls_mall_head *head = container_of(rcu, struct cls_mall_head,
						  rcu);

	INIT_WORK(&head->work, mall_destroy_work);
	tcf_queue_work(&head->work);
}

static void mall_destroy_hw_filter(struct tcf_proto *tp,
				   struct cls_mall_head *head,
				   unsigned long cookie)
{
	struct tc_cls_matchall_offload cls_mall = {};
	struct tcf_block *block = tp->chain->block;

	tc_cls_common_offload_init(&cls_mall.common, tp);
	cls_mall.command = TC_CLSMATCHALL_DESTROY;
	cls_mall.cookie = cookie;

	tc_setup_cb_call(block, NULL, TC_SETUP_CLSMATCHALL, &cls_mall, false);
}

static int mall_replace_hw_filter(struct tcf_proto *tp,
				  struct cls_mall_head *head,
				  unsigned long cookie)
{
	struct tc_cls_matchall_offload cls_mall = {};
	struct tcf_block *block = tp->chain->block;
	bool skip_sw = tc_skip_sw(head->flags);
	int err;

	tc_cls_common_offload_init(&cls_mall.common, tp);
	cls_mall.command = TC_CLSMATCHALL_REPLACE;
	cls_mall.exts = &head->exts;
	cls_mall.cookie = cookie;

	err = tc_setup_cb_call(block, NULL, TC_SETUP_CLSMATCHALL,
			       &cls_mall, skip_sw);
	if (err < 0) {
		mall_destroy_hw_filter(tp, head, cookie);
		return err;
	} else if (err > 0) {
		head->flags |= TCA_CLS_FLAGS_IN_HW;
	}

	if (skip_sw && !(head->flags & TCA_CLS_FLAGS_IN_HW))
		return -EINVAL;

	return 0;
}

static void mall_destroy(struct tcf_proto *tp)
{
	struct cls_mall_head *head = rtnl_dereference(tp->root);

	if (!head)
		return;

	if (!tc_skip_hw(head->flags))
		mall_destroy_hw_filter(tp, head, (unsigned long) head);

	if (tcf_exts_get_net(&head->exts))
		call_rcu(&head->rcu, mall_destroy_rcu);
	else
		__mall_destroy(head);
}

static void *mall_get(struct tcf_proto *tp, u32 handle)
{
	return NULL;
>>>>>>> temp
}

static const struct nla_policy mall_policy[TCA_MATCHALL_MAX + 1] = {
	[TCA_MATCHALL_UNSPEC]		= { .type = NLA_UNSPEC },
	[TCA_MATCHALL_CLASSID]		= { .type = NLA_U32 },
};

static int mall_set_parms(struct net *net, struct tcf_proto *tp,
<<<<<<< HEAD
			  struct cls_mall_filter *f,
			  unsigned long base, struct nlattr **tb,
			  struct nlattr *est, bool ovr)
{
	struct tcf_exts e;
	int err;

	tcf_exts_init(&e, TCA_MATCHALL_ACT, 0);
	err = tcf_exts_validate(net, tp, tb, est, &e, ovr);
=======
			  struct cls_mall_head *head,
			  unsigned long base, struct nlattr **tb,
			  struct nlattr *est, bool ovr)
{
	int err;

	err = tcf_exts_validate(net, tp, tb, est, &head->exts, ovr);
>>>>>>> temp
	if (err < 0)
		return err;

	if (tb[TCA_MATCHALL_CLASSID]) {
<<<<<<< HEAD
		f->res.classid = nla_get_u32(tb[TCA_MATCHALL_CLASSID]);
		tcf_bind_filter(tp, &f->res, base);
	}

	tcf_exts_change(tp, &f->exts, &e);

=======
		head->res.classid = nla_get_u32(tb[TCA_MATCHALL_CLASSID]);
		tcf_bind_filter(tp, &head->res, base);
	}
>>>>>>> temp
	return 0;
}

static int mall_change(struct net *net, struct sk_buff *in_skb,
		       struct tcf_proto *tp, unsigned long base,
		       u32 handle, struct nlattr **tca,
<<<<<<< HEAD
		       unsigned long *arg, bool ovr)
{
	struct cls_mall_head *head = rtnl_dereference(tp->root);
	struct cls_mall_filter *fold = (struct cls_mall_filter *) *arg;
	struct cls_mall_filter *f;
	struct nlattr *tb[TCA_MATCHALL_MAX + 1];
=======
		       void **arg, bool ovr)
{
	struct cls_mall_head *head = rtnl_dereference(tp->root);
	struct nlattr *tb[TCA_MATCHALL_MAX + 1];
	struct cls_mall_head *new;
	u32 flags = 0;
>>>>>>> temp
	int err;

	if (!tca[TCA_OPTIONS])
		return -EINVAL;

<<<<<<< HEAD
	if (head->filter)
		return -EBUSY;

	if (fold)
		return -EINVAL;

	err = nla_parse_nested(tb, TCA_MATCHALL_MAX,
			       tca[TCA_OPTIONS], mall_policy);
	if (err < 0)
		return err;

	f = kzalloc(sizeof(*f), GFP_KERNEL);
	if (!f)
		return -ENOBUFS;

	tcf_exts_init(&f->exts, TCA_MATCHALL_ACT, 0);

	if (!handle)
		handle = 1;
	f->handle = handle;

	err = mall_set_parms(net, tp, f, base, tb, tca[TCA_RATE], ovr);
	if (err)
		goto errout;

	*arg = (unsigned long) f;
	rcu_assign_pointer(head->filter, f);

	return 0;

errout:
	kfree(f);
	return err;
}

static int mall_delete(struct tcf_proto *tp, unsigned long arg)
{
	struct cls_mall_head *head = rtnl_dereference(tp->root);
	struct cls_mall_filter *f = (struct cls_mall_filter *) arg;

	RCU_INIT_POINTER(head->filter, NULL);
	tcf_unbind_filter(tp, &f->res);
	call_rcu(&f->rcu, mall_destroy_filter);
	return 0;
=======
	if (head)
		return -EEXIST;

	err = nla_parse_nested(tb, TCA_MATCHALL_MAX, tca[TCA_OPTIONS],
			       mall_policy, NULL);
	if (err < 0)
		return err;

	if (tb[TCA_MATCHALL_FLAGS]) {
		flags = nla_get_u32(tb[TCA_MATCHALL_FLAGS]);
		if (!tc_flags_valid(flags))
			return -EINVAL;
	}

	new = kzalloc(sizeof(*new), GFP_KERNEL);
	if (!new)
		return -ENOBUFS;

	err = tcf_exts_init(&new->exts, TCA_MATCHALL_ACT, 0);
	if (err)
		goto err_exts_init;

	if (!handle)
		handle = 1;
	new->handle = handle;
	new->flags = flags;

	err = mall_set_parms(net, tp, new, base, tb, tca[TCA_RATE], ovr);
	if (err)
		goto err_set_parms;

	if (!tc_skip_hw(new->flags)) {
		err = mall_replace_hw_filter(tp, new, (unsigned long) new);
		if (err)
			goto err_replace_hw_filter;
	}

	if (!tc_in_hw(new->flags))
		new->flags |= TCA_CLS_FLAGS_NOT_IN_HW;

	*arg = head;
	rcu_assign_pointer(tp->root, new);
	return 0;

err_replace_hw_filter:
err_set_parms:
	tcf_exts_destroy(&new->exts);
err_exts_init:
	kfree(new);
	return err;
}

static int mall_delete(struct tcf_proto *tp, void *arg, bool *last)
{
	return -EOPNOTSUPP;
>>>>>>> temp
}

static void mall_walk(struct tcf_proto *tp, struct tcf_walker *arg)
{
	struct cls_mall_head *head = rtnl_dereference(tp->root);
<<<<<<< HEAD
	struct cls_mall_filter *f = head->filter;

	if (arg->count < arg->skip)
		goto skip;
	if (arg->fn(tp, (unsigned long) f, arg) < 0)
=======

	if (arg->count < arg->skip)
		goto skip;
	if (arg->fn(tp, head, arg) < 0)
>>>>>>> temp
		arg->stop = 1;
skip:
	arg->count++;
}

<<<<<<< HEAD
static int mall_dump(struct net *net, struct tcf_proto *tp, unsigned long fh,
		     struct sk_buff *skb, struct tcmsg *t)
{
	struct cls_mall_filter *f = (struct cls_mall_filter *) fh;
	struct nlattr *nest;

	if (!f)
		return skb->len;

	t->tcm_handle = f->handle;
=======
static int mall_dump(struct net *net, struct tcf_proto *tp, void *fh,
		     struct sk_buff *skb, struct tcmsg *t)
{
	struct cls_mall_head *head = fh;
	struct nlattr *nest;

	if (!head)
		return skb->len;

	t->tcm_handle = head->handle;
>>>>>>> temp

	nest = nla_nest_start(skb, TCA_OPTIONS);
	if (!nest)
		goto nla_put_failure;

<<<<<<< HEAD
	if (f->res.classid &&
	    nla_put_u32(skb, TCA_MATCHALL_CLASSID, f->res.classid))
		goto nla_put_failure;

	if (tcf_exts_dump(skb, &f->exts))
=======
	if (head->res.classid &&
	    nla_put_u32(skb, TCA_MATCHALL_CLASSID, head->res.classid))
		goto nla_put_failure;

	if (head->flags && nla_put_u32(skb, TCA_MATCHALL_FLAGS, head->flags))
		goto nla_put_failure;

	if (tcf_exts_dump(skb, &head->exts))
>>>>>>> temp
		goto nla_put_failure;

	nla_nest_end(skb, nest);

<<<<<<< HEAD
	if (tcf_exts_dump_stats(skb, &f->exts) < 0)
=======
	if (tcf_exts_dump_stats(skb, &head->exts) < 0)
>>>>>>> temp
		goto nla_put_failure;

	return skb->len;

nla_put_failure:
	nla_nest_cancel(skb, nest);
	return -1;
}

<<<<<<< HEAD
=======
static void mall_bind_class(void *fh, u32 classid, unsigned long cl)
{
	struct cls_mall_head *head = fh;

	if (head && head->res.classid == classid)
		head->res.class = cl;
}

>>>>>>> temp
static struct tcf_proto_ops cls_mall_ops __read_mostly = {
	.kind		= "matchall",
	.classify	= mall_classify,
	.init		= mall_init,
	.destroy	= mall_destroy,
	.get		= mall_get,
	.change		= mall_change,
	.delete		= mall_delete,
	.walk		= mall_walk,
	.dump		= mall_dump,
<<<<<<< HEAD
=======
	.bind_class	= mall_bind_class,
>>>>>>> temp
	.owner		= THIS_MODULE,
};

static int __init cls_mall_init(void)
{
	return register_tcf_proto_ops(&cls_mall_ops);
}

static void __exit cls_mall_exit(void)
{
	unregister_tcf_proto_ops(&cls_mall_ops);
}

module_init(cls_mall_init);
module_exit(cls_mall_exit);

MODULE_AUTHOR("Jiri Pirko <jiri@mellanox.com>");
MODULE_DESCRIPTION("Match-all classifier");
MODULE_LICENSE("GPL v2");
