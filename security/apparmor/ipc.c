/*
 * AppArmor security module
 *
 * This file contains AppArmor ipc mediation
 *
 * Copyright (C) 1998-2008 Novell/SUSE
<<<<<<< HEAD
 * Copyright 2009-2013 Canonical Ltd.
=======
 * Copyright 2009-2017 Canonical Ltd.
>>>>>>> temp
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 2 of the
 * License.
 */

#include <linux/gfp.h>
#include <linux/ptrace.h>

#include "include/audit.h"
#include "include/capability.h"
#include "include/context.h"
#include "include/policy.h"
#include "include/ipc.h"
#include "include/sig_names.h"

/**
 * audit_ptrace_mask - convert mask to permission string
 * @buffer: buffer to write string to (NOT NULL)
 * @mask: permission mask to convert
 */
static void audit_ptrace_mask(struct audit_buffer *ab, u32 mask)
{
	switch (mask) {
	case MAY_READ:
		audit_log_string(ab, "read");
		break;
	case MAY_WRITE:
		audit_log_string(ab, "trace");
		break;
	case AA_MAY_BE_READ:
		audit_log_string(ab, "readby");
		break;
	case AA_MAY_BE_TRACED:
		audit_log_string(ab, "tracedby");
		break;
	}
}

/* call back to audit ptrace fields */
static void audit_ptrace_cb(struct audit_buffer *ab, void *va)
{
	struct common_audit_data *sa = va;

	if (aad(sa)->request & AA_PTRACE_PERM_MASK) {
		audit_log_format(ab, " requested_mask=");
		audit_ptrace_mask(ab, aad(sa)->request);

		if (aad(sa)->denied & AA_PTRACE_PERM_MASK) {
			audit_log_format(ab, " denied_mask=");
			audit_ptrace_mask(ab, aad(sa)->denied);
		}
	}
	audit_log_format(ab, " peer=");
	aa_label_xaudit(ab, labels_ns(aad(sa)->label), aad(sa)->peer,
			FLAGS_NONE, GFP_ATOMIC);
}

<<<<<<< HEAD
/* TODO: conditionals */
static int profile_ptrace_perm(struct aa_profile *profile,
			       struct aa_profile *peer, u32 request,
			       struct common_audit_data *sa)
{
  struct aa_perms perms = { };

	/* need because of peer in cross check */
	if (profile_unconfined(profile) ||
	    !PROFILE_MEDIATES(profile, AA_CLASS_PTRACE))
                return 0;

	aad(sa)->peer = &peer->label;
	aa_profile_match_label(profile, &peer->label, AA_CLASS_PTRACE, request,
=======
/* assumes check for PROFILE_MEDIATES is already done */
/* TODO: conditionals */
static int profile_ptrace_perm(struct aa_profile *profile,
			     struct aa_label *peer, u32 request,
			     struct common_audit_data *sa)
{
	struct aa_perms perms = { };

	aad(sa)->peer = peer;
	aa_profile_match_label(profile, peer, AA_CLASS_PTRACE, request,
>>>>>>> temp
			       &perms);
	aa_apply_modes_to_perms(profile, &perms);
	return aa_check_perms(profile, &perms, request, sa, audit_ptrace_cb);
}

<<<<<<< HEAD
static int cross_ptrace_perm(struct aa_profile *tracer,
			     struct aa_profile *tracee, u32 request,
			     struct common_audit_data *sa)
{
	if (PROFILE_MEDIATES(tracer, AA_CLASS_PTRACE))
		return xcheck(profile_ptrace_perm(tracer, tracee, request, sa),
			      profile_ptrace_perm(tracee, tracer,
						  request << PTRACE_PERM_SHIFT,
						  sa));
	/* policy uses the old style capability check for ptrace */
	if (profile_unconfined(tracer) || tracer == tracee)
		return 0;

	aad(sa)->label = &tracer->label;
	aad(sa)->peer = &tracee->label;
	aad(sa)->request = 0;
	aad(sa)->error = aa_capable(&tracer->label, CAP_SYS_PTRACE, 1);
=======
static int profile_tracee_perm(struct aa_profile *tracee,
			       struct aa_label *tracer, u32 request,
			       struct common_audit_data *sa)
{
	if (profile_unconfined(tracee) || unconfined(tracer) ||
	    !PROFILE_MEDIATES(tracee, AA_CLASS_PTRACE))
		return 0;

	return profile_ptrace_perm(tracee, tracer, request, sa);
}

static int profile_tracer_perm(struct aa_profile *tracer,
			       struct aa_label *tracee, u32 request,
			       struct common_audit_data *sa)
{
	if (profile_unconfined(tracer))
		return 0;

	if (PROFILE_MEDIATES(tracer, AA_CLASS_PTRACE))
		return profile_ptrace_perm(tracer, tracee, request, sa);

	/* profile uses the old style capability check for ptrace */
	if (&tracer->label == tracee)
		return 0;

	aad(sa)->label = &tracer->label;
	aad(sa)->peer = tracee;
	aad(sa)->request = 0;
	aad(sa)->error = aa_capable(&tracer->label, CAP_SYS_PTRACE, 1);

>>>>>>> temp
	return aa_audit(AUDIT_APPARMOR_AUTO, tracer, sa, audit_ptrace_cb);
}

/**
 * aa_may_ptrace - test if tracer task can trace the tracee
 * @tracer: label of the task doing the tracing  (NOT NULL)
 * @tracee: task label to be traced
 * @request: permission request
 *
 * Returns: %0 else error code if permission denied or error
 */
int aa_may_ptrace(struct aa_label *tracer, struct aa_label *tracee,
		  u32 request)
{
<<<<<<< HEAD
	DEFINE_AUDIT_DATA(sa, LSM_AUDIT_DATA_NONE, OP_PTRACE);

	return xcheck_labels_profiles(tracer, tracee, cross_ptrace_perm,
				      request, &sa);
=======
	struct aa_profile *profile;
	u32 xrequest = request << PTRACE_PERM_SHIFT;
	DEFINE_AUDIT_DATA(sa, LSM_AUDIT_DATA_NONE, OP_PTRACE);

	return xcheck_labels(tracer, tracee, profile,
			profile_tracer_perm(profile, tracee, request, &sa),
			profile_tracee_perm(profile, tracer, xrequest, &sa));
>>>>>>> temp
}


static inline int map_signal_num(int sig)
{
	if (sig > SIGRTMAX)
		return SIGUNKNOWN;
	else if (sig >= SIGRTMIN)
<<<<<<< HEAD
		return sig - SIGRTMIN + 128;	/* rt sigs mapped to 128 */
	else if (sig <= MAXMAPPED_SIG)
=======
		return sig - SIGRTMIN + SIGRT_BASE;
	else if (sig < MAXMAPPED_SIG)
>>>>>>> temp
		return sig_map[sig];
	return SIGUNKNOWN;
}

/**
 * audit_file_mask - convert mask to permission string
 * @buffer: buffer to write string to (NOT NULL)
 * @mask: permission mask to convert
<<<<<<< HEAD
 */
static void audit_signal_mask(struct audit_buffer *ab, u32 mask)
{
	if (mask & MAY_READ)
		audit_log_string(ab, "receive");
	if (mask & MAY_WRITE)
		audit_log_string(ab, "send");
}

/**
 * audit_cb - call back for signal specific audit fields
 * @ab: audit_buffer  (NOT NULL)
 * @va: audit struct to audit values of  (NOT NULL)
 */
static void audit_signal_cb(struct audit_buffer *ab, void *va)
{
	struct common_audit_data *sa = va;

	if (aad(sa)->request & AA_SIGNAL_PERM_MASK) {
		audit_log_format(ab, " requested_mask=");
		audit_signal_mask(ab, aad(sa)->request);
		if (aad(sa)->denied & AA_SIGNAL_PERM_MASK) {
			audit_log_format(ab, " denied_mask=");
			audit_signal_mask(ab, aad(sa)->denied);
		}
	}
	if (aad(sa)->signal <= MAXMAPPED_SIG)
		audit_log_format(ab, " signal=%s", sig_names[aad(sa)->signal]);
	else
		audit_log_format(ab, " signal=rtmin+%d",
				 aad(sa)->signal - 128);
	audit_log_format(ab, " peer=");
	aa_label_xaudit(ab, labels_ns(aad(sa)->label), aad(sa)->peer,
			FLAGS_NONE, GFP_ATOMIC);
}

/* TODO: update to handle compound name&name2, conditionals */
static void profile_match_signal(struct aa_profile *profile, const char *label,
				 int signal, struct aa_perms *perms)
{
	unsigned int state;
	/* TODO: secondary cache check <profile, profile, perm> */
	state = aa_dfa_next(profile->policy.dfa,
			    profile->policy.start[AA_CLASS_SIGNAL],
			    signal);
	state = aa_dfa_match(profile->policy.dfa, state, label);
	aa_compute_perms(profile->policy.dfa, state, perms);
}

static int profile_signal_perm(struct aa_profile *profile,
			       struct aa_profile *peer, u32 request,
			       struct common_audit_data *sa)
{
	struct aa_perms perms;
=======
 */
static void audit_signal_mask(struct audit_buffer *ab, u32 mask)
{
	if (mask & MAY_READ)
		audit_log_string(ab, "receive");
	if (mask & MAY_WRITE)
		audit_log_string(ab, "send");
}

/**
 * audit_cb - call back for signal specific audit fields
 * @ab: audit_buffer  (NOT NULL)
 * @va: audit struct to audit values of  (NOT NULL)
 */
static void audit_signal_cb(struct audit_buffer *ab, void *va)
{
	struct common_audit_data *sa = va;

	if (aad(sa)->request & AA_SIGNAL_PERM_MASK) {
		audit_log_format(ab, " requested_mask=");
		audit_signal_mask(ab, aad(sa)->request);
		if (aad(sa)->denied & AA_SIGNAL_PERM_MASK) {
			audit_log_format(ab, " denied_mask=");
			audit_signal_mask(ab, aad(sa)->denied);
		}
	}
	if (aad(sa)->signal == SIGUNKNOWN)
		audit_log_format(ab, "signal=unknown(%d)",
				 aad(sa)->unmappedsig);
	else if (aad(sa)->signal < MAXMAPPED_SIGNAME)
		audit_log_format(ab, " signal=%s", sig_names[aad(sa)->signal]);
	else
		audit_log_format(ab, " signal=rtmin+%d",
				 aad(sa)->signal - SIGRT_BASE);
	audit_log_format(ab, " peer=");
	aa_label_xaudit(ab, labels_ns(aad(sa)->label), aad(sa)->peer,
			FLAGS_NONE, GFP_ATOMIC);
}

static int profile_signal_perm(struct aa_profile *profile,
			       struct aa_label *peer, u32 request,
			       struct common_audit_data *sa)
{
	struct aa_perms perms;
	unsigned int state;
>>>>>>> temp

	if (profile_unconfined(profile) ||
	    !PROFILE_MEDIATES(profile, AA_CLASS_SIGNAL))
		return 0;

<<<<<<< HEAD
	aad(sa)->peer = &peer->label;
	profile_match_signal(profile, aa_peer_name(peer), aad(sa)->signal,
			     &perms);
=======
	aad(sa)->peer = peer;
	/* TODO: secondary cache check <profile, profile, perm> */
	state = aa_dfa_next(profile->policy.dfa,
			    profile->policy.start[AA_CLASS_SIGNAL],
			    aad(sa)->signal);
	aa_label_match(profile, peer, state, false, request, &perms);
>>>>>>> temp
	aa_apply_modes_to_perms(profile, &perms);
	return aa_check_perms(profile, &perms, request, sa, audit_signal_cb);
}

<<<<<<< HEAD
static int aa_signal_cross_perm(struct aa_profile *sender,
				struct aa_profile *target,
				struct common_audit_data *sa)
{
	return xcheck(profile_signal_perm(sender, target, MAY_WRITE, sa),
		      profile_signal_perm(target, sender, MAY_READ, sa));
}

int aa_may_signal(struct aa_label *sender, struct aa_label *target, int sig)
{
	DEFINE_AUDIT_DATA(sa, LSM_AUDIT_DATA_NONE, OP_SIGNAL);
	aad(&sa)->signal = map_signal_num(sig);
	return xcheck_labels_profiles(sender, target, aa_signal_cross_perm,
				      &sa);
=======
int aa_may_signal(struct aa_label *sender, struct aa_label *target, int sig)
{
	struct aa_profile *profile;
	DEFINE_AUDIT_DATA(sa, LSM_AUDIT_DATA_NONE, OP_SIGNAL);

	aad(&sa)->signal = map_signal_num(sig);
	aad(&sa)->unmappedsig = sig;
	return xcheck_labels(sender, target, profile,
			profile_signal_perm(profile, target, MAY_WRITE, &sa),
			profile_signal_perm(profile, sender, MAY_READ, &sa));
>>>>>>> temp
}
