/*
 * Copyright (c) 2009, Microsoft Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place - Suite 330, Boston, MA 02111-1307 USA.
 *
 * Authors:
 *   Haiyang Zhang <haiyangz@microsoft.com>
 *   Hank Janssen  <hjanssen@microsoft.com>
 *
 */
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/hyperv.h>
#include <linux/version.h>
#include <linux/interrupt.h>
#include <linux/clockchips.h>
#include <asm/hyperv.h>
#include <asm/mshyperv.h>
#include <asm/nospec-branch.h>
#include "hyperv_vmbus.h"

#ifndef PKG_ABI
/*
 * Preserve the ability to 'make deb-pkg' since PKG_ABI is provided
 * by the Ubuntu build rules.
 */
#define PKG_ABI 0
#endif

/* The one and only */
struct hv_context hv_context = {
	.synic_initialized	= false,
};

#define HV_TIMER_FREQUENCY (10 * 1000 * 1000) /* 100ns period */
#define HV_MAX_MAX_DELTA_TICKS 0xffffffff
#define HV_MIN_DELTA_TICKS 1

/*
<<<<<<< HEAD
 * query_hypervisor_info - Get version info of the windows hypervisor
 */
unsigned int host_info_eax;
unsigned int host_info_ebx;
unsigned int host_info_ecx;
unsigned int host_info_edx;

static int query_hypervisor_info(void)
{
	unsigned int eax;
	unsigned int ebx;
	unsigned int ecx;
	unsigned int edx;
	unsigned int max_leaf;
	unsigned int op;

	/*
	* Its assumed that this is called after confirming that Viridian
	* is present. Query id and revision.
	*/
	eax = 0;
	ebx = 0;
	ecx = 0;
	edx = 0;
	op = HVCPUID_VENDOR_MAXFUNCTION;
	cpuid(op, &eax, &ebx, &ecx, &edx);

	max_leaf = eax;

	if (max_leaf >= HVCPUID_VERSION) {
		eax = 0;
		ebx = 0;
		ecx = 0;
		edx = 0;
		op = HVCPUID_VERSION;
		cpuid(op, &eax, &ebx, &ecx, &edx);
		host_info_eax = eax;
		host_info_ebx = ebx;
		host_info_ecx = ecx;
		host_info_edx = edx;
	}
	return max_leaf;
}

/*
 * hv_do_hypercall- Invoke the specified hypercall
 */
u64 hv_do_hypercall(u64 control, void *input, void *output)
{
	u64 input_address = (input) ? virt_to_phys(input) : 0;
	u64 output_address = (output) ? virt_to_phys(output) : 0;
	void *hypercall_page = hv_context.hypercall_page;
#ifdef CONFIG_X86_64
	u64 hv_status = 0;

	if (!hypercall_page)
		return (u64)ULLONG_MAX;

	__asm__ __volatile__("mov %0, %%r8" : : "r" (output_address) : "r8");
	__asm__ __volatile__(CALL_NOSPEC :
			     "=a" (hv_status) :
			     "c" (control), "d" (input_address),
			     THUNK_TARGET(hypercall_page));

	return hv_status;

#else

	u32 control_hi = control >> 32;
	u32 control_lo = control & 0xFFFFFFFF;
	u32 hv_status_hi = 1;
	u32 hv_status_lo = 1;
	u32 input_address_hi = input_address >> 32;
	u32 input_address_lo = input_address & 0xFFFFFFFF;
	u32 output_address_hi = output_address >> 32;
	u32 output_address_lo = output_address & 0xFFFFFFFF;

	if (!hypercall_page)
		return (u64)ULLONG_MAX;

	__asm__ __volatile__ (CALL_NOSPEC : "=d"(hv_status_hi),
			      "=a"(hv_status_lo) : "d" (control_hi),
			      "a" (control_lo), "b" (input_address_hi),
			      "c" (input_address_lo), "D"(output_address_hi),
			      "S"(output_address_lo),
			      THUNK_TARGET(hypercall_page));

	return hv_status_lo | ((u64)hv_status_hi << 32);
#endif /* !x86_64 */
}
EXPORT_SYMBOL_GPL(hv_do_hypercall);

#ifdef CONFIG_X86_64
static cycle_t read_hv_clock_tsc(struct clocksource *arg)
{
	cycle_t current_tick;
	struct ms_hyperv_tsc_page *tsc_pg = hv_context.tsc_page;

	if (tsc_pg->tsc_sequence != 0) {
		/*
		 * Use the tsc page to compute the value.
		 */

		while (1) {
			cycle_t tmp;
			u32 sequence = tsc_pg->tsc_sequence;
			u64 cur_tsc;
			u64 scale = tsc_pg->tsc_scale;
			s64 offset = tsc_pg->tsc_offset;

			rdtscll(cur_tsc);
			/* current_tick = ((cur_tsc *scale) >> 64) + offset */
			asm("mulq %3"
				: "=d" (current_tick), "=a" (tmp)
				: "a" (cur_tsc), "r" (scale));

			current_tick += offset;
			if (tsc_pg->tsc_sequence == sequence)
				return current_tick;

			if (tsc_pg->tsc_sequence != 0)
				continue;
			/*
			 * Fallback using MSR method.
			 */
			break;
		}
	}
	rdmsrl(HV_X64_MSR_TIME_REF_COUNT, current_tick);
	return current_tick;
}

static struct clocksource hyperv_cs_tsc = {
		.name           = "hyperv_clocksource_tsc_page",
		.rating         = 425,
		.read           = read_hv_clock_tsc,
		.mask           = CLOCKSOURCE_MASK(64),
		.flags          = CLOCK_SOURCE_IS_CONTINUOUS,
};
#endif

static cycle_t read_hv_clock_msr(struct clocksource *arg)
{
	cycle_t current_tick;
	/*
	 * Read the partition counter to get the current tick count. This count
	 * is set to 0 when the partition is created and is incremented in
	 * 100 nanosecond units.
	 */
	rdmsrl(HV_X64_MSR_TIME_REF_COUNT, current_tick);
	return current_tick;
}

static struct clocksource hyperv_cs_msr = {
	.name		= "hyperv_clocksource_msr",
	.rating		= 400, /* use this when running on Hyperv*/
	.read		= read_hv_clock_msr,
	.mask		= CLOCKSOURCE_MASK(64),
	.flags		= CLOCK_SOURCE_IS_CONTINUOUS,
};

struct clocksource *hyperv_cs;
EXPORT_SYMBOL_GPL(hyperv_cs);

/*
=======
>>>>>>> temp
 * hv_init - Main initialization routine.
 *
 * This routine must be called before any other routines in here are called
 */
int hv_init(void)
{
<<<<<<< HEAD
	int max_leaf;
	union hv_x64_msr_hypercall_contents hypercall_msr;
	void *virtaddr = NULL;

	memset(hv_context.synic_event_page, 0, sizeof(void *) * NR_CPUS);
	memset(hv_context.synic_message_page, 0,
	       sizeof(void *) * NR_CPUS);
	memset(hv_context.post_msg_page, 0,
	       sizeof(void *) * NR_CPUS);
	memset(hv_context.vp_index, 0,
	       sizeof(int) * NR_CPUS);
	memset(hv_context.event_dpc, 0,
	       sizeof(void *) * NR_CPUS);
	memset(hv_context.msg_dpc, 0,
	       sizeof(void *) * NR_CPUS);
	memset(hv_context.clk_evt, 0,
	       sizeof(void *) * NR_CPUS);

	max_leaf = query_hypervisor_info();

	/*
	 * Write our OS ID.
	 */
	hv_context.guestid = generate_guest_id(0x80 /*Canonical*/, LINUX_VERSION_CODE, PKG_ABI);
	wrmsrl(HV_X64_MSR_GUEST_OS_ID, hv_context.guestid);

	/* See if the hypercall page is already set */
	rdmsrl(HV_X64_MSR_HYPERCALL, hypercall_msr.as_uint64);

	virtaddr = __vmalloc(PAGE_SIZE, GFP_KERNEL, PAGE_KERNEL_RX);

	if (!virtaddr)
		goto cleanup;

	hypercall_msr.enable = 1;

	hypercall_msr.guest_physical_address = vmalloc_to_pfn(virtaddr);
	wrmsrl(HV_X64_MSR_HYPERCALL, hypercall_msr.as_uint64);

	/* Confirm that hypercall page did get setup. */
	hypercall_msr.as_uint64 = 0;
	rdmsrl(HV_X64_MSR_HYPERCALL, hypercall_msr.as_uint64);

	if (!hypercall_msr.enable)
		goto cleanup;

	hv_context.hypercall_page = virtaddr;

#ifdef CONFIG_X86_64
	if (ms_hyperv.features & HV_X64_MSR_REFERENCE_TSC_AVAILABLE) {
		union hv_x64_msr_hypercall_contents tsc_msr;
		void *va_tsc;

		va_tsc = __vmalloc(PAGE_SIZE, GFP_KERNEL, PAGE_KERNEL);
		if (!va_tsc)
			goto register_msr_cs;
		hv_context.tsc_page = va_tsc;

		hyperv_cs = &hyperv_cs_tsc;

		rdmsrl(HV_X64_MSR_REFERENCE_TSC, tsc_msr.as_uint64);

		tsc_msr.enable = 1;
		tsc_msr.guest_physical_address = vmalloc_to_pfn(va_tsc);

		wrmsrl(HV_X64_MSR_REFERENCE_TSC, tsc_msr.as_uint64);
		clocksource_register_hz(&hyperv_cs_tsc, NSEC_PER_SEC/100);
	}
register_msr_cs:
#endif
	/*
	 * For 32 bit guests just use the MSR based mechanism for reading
	 * the partition counter.
	 */

	hyperv_cs = &hyperv_cs_msr;
	if (ms_hyperv.features & HV_X64_MSR_TIME_REF_COUNT_AVAILABLE)
		clocksource_register_hz(&hyperv_cs_msr, NSEC_PER_SEC/100);

	return 0;

cleanup:
	if (virtaddr) {
		if (hypercall_msr.enable) {
			hypercall_msr.as_uint64 = 0;
			wrmsrl(HV_X64_MSR_HYPERCALL, hypercall_msr.as_uint64);
		}

		vfree(virtaddr);
	}

	return -ENOTSUPP;
}

/*
 * hv_cleanup - Cleanup routine.
 *
 * This routine is called normally during driver unloading or exiting.
 */
void hv_cleanup(bool crash)
{
	union hv_x64_msr_hypercall_contents hypercall_msr;

	/* Reset our OS id */
	wrmsrl(HV_X64_MSR_GUEST_OS_ID, 0);

	if (hv_context.hypercall_page) {
		hypercall_msr.as_uint64 = 0;
		wrmsrl(HV_X64_MSR_HYPERCALL, hypercall_msr.as_uint64);
		if (!crash)
			vfree(hv_context.hypercall_page);
		hv_context.hypercall_page = NULL;
	}

#ifdef CONFIG_X86_64
	/*
	 * Cleanup the TSC page based CS.
	 */
	if (ms_hyperv.features & HV_X64_MSR_REFERENCE_TSC_AVAILABLE) {
		/*
		 * Crash can happen in an interrupt context and unregistering
		 * a clocksource is impossible and redundant in this case.
		 */
		if (!oops_in_progress) {
			clocksource_change_rating(&hyperv_cs_tsc, 10);
			clocksource_unregister(&hyperv_cs_tsc);
		}

		hypercall_msr.as_uint64 = 0;
		wrmsrl(HV_X64_MSR_REFERENCE_TSC, hypercall_msr.as_uint64);
		if (!crash) {
			vfree(hv_context.tsc_page);
			hv_context.tsc_page = NULL;
		}
	}
#endif
=======
	if (!hv_is_hypercall_page_setup())
		return -ENOTSUPP;

	hv_context.cpu_context = alloc_percpu(struct hv_per_cpu_context);
	if (!hv_context.cpu_context)
		return -ENOMEM;

	return 0;
>>>>>>> temp
}

/*
 * hv_post_message - Post a message using the hypervisor message IPC.
 *
 * This involves a hypercall.
 */
int hv_post_message(union hv_connection_id connection_id,
		  enum hv_message_type message_type,
		  void *payload, size_t payload_size)
{
	struct hv_input_post_message *aligned_msg;
<<<<<<< HEAD
=======
	struct hv_per_cpu_context *hv_cpu;
>>>>>>> temp
	u64 status;

	if (payload_size > HV_MESSAGE_PAYLOAD_BYTE_COUNT)
		return -EMSGSIZE;

	hv_cpu = get_cpu_ptr(hv_context.cpu_context);
	aligned_msg = hv_cpu->post_msg_page;
	aligned_msg->connectionid = connection_id;
	aligned_msg->reserved = 0;
	aligned_msg->message_type = message_type;
	aligned_msg->payload_size = payload_size;
	memcpy((void *)aligned_msg->payload, payload, payload_size);

	status = hv_do_hypercall(HVCALL_POST_MESSAGE, aligned_msg, NULL);

<<<<<<< HEAD
	put_cpu();
=======
	/* Preemption must remain disabled until after the hypercall
	 * so some other thread can't get scheduled onto this cpu and
	 * corrupt the per-cpu post_msg_page
	 */
	put_cpu_ptr(hv_cpu);

>>>>>>> temp
	return status & 0xFFFF;
}

static int hv_ce_set_next_event(unsigned long delta,
				struct clock_event_device *evt)
{
	u64 current_tick;

	WARN_ON(!clockevent_state_oneshot(evt));

	current_tick = hyperv_cs->read(NULL);
	current_tick += delta;
	hv_init_timer(HV_X64_MSR_STIMER0_COUNT, current_tick);
	return 0;
}

static int hv_ce_shutdown(struct clock_event_device *evt)
{
	hv_init_timer(HV_X64_MSR_STIMER0_COUNT, 0);
	hv_init_timer_config(HV_X64_MSR_STIMER0_CONFIG, 0);

	return 0;
}

static int hv_ce_set_oneshot(struct clock_event_device *evt)
{
	union hv_timer_config timer_cfg;

	timer_cfg.enable = 1;
	timer_cfg.auto_enable = 1;
	timer_cfg.sintx = VMBUS_MESSAGE_SINT;
	hv_init_timer_config(HV_X64_MSR_STIMER0_CONFIG, timer_cfg.as_uint64);

	return 0;
}

static void hv_init_clockevent_device(struct clock_event_device *dev, int cpu)
{
	dev->name = "Hyper-V clockevent";
	dev->features = CLOCK_EVT_FEAT_ONESHOT;
	dev->cpumask = cpumask_of(cpu);
	dev->rating = 1000;
	/*
	 * Avoid settint dev->owner = THIS_MODULE deliberately as doing so will
	 * result in clockevents_config_and_register() taking additional
	 * references to the hv_vmbus module making it impossible to unload.
	 */

	dev->set_state_shutdown = hv_ce_shutdown;
	dev->set_state_oneshot = hv_ce_set_oneshot;
	dev->set_next_event = hv_ce_set_next_event;
}


int hv_synic_alloc(void)
{
	int cpu;

	hv_context.hv_numa_map = kzalloc(sizeof(struct cpumask) * nr_node_ids,
					 GFP_ATOMIC);
	if (hv_context.hv_numa_map == NULL) {
		pr_err("Unable to allocate NUMA map\n");
		goto err;
	}

	for_each_present_cpu(cpu) {
<<<<<<< HEAD
		hv_context.event_dpc[cpu] = kmalloc(size, GFP_ATOMIC);
		if (hv_context.event_dpc[cpu] == NULL) {
			pr_err("Unable to allocate event dpc\n");
			goto err;
		}
		tasklet_init(hv_context.event_dpc[cpu], vmbus_on_event, cpu);

		hv_context.msg_dpc[cpu] = kmalloc(size, GFP_ATOMIC);
		if (hv_context.msg_dpc[cpu] == NULL) {
			pr_err("Unable to allocate event dpc\n");
			goto err;
		}
		tasklet_init(hv_context.msg_dpc[cpu], vmbus_on_msg_dpc, cpu);

		hv_context.clk_evt[cpu] = kzalloc(ced_size, GFP_ATOMIC);
		if (hv_context.clk_evt[cpu] == NULL) {
=======
		struct hv_per_cpu_context *hv_cpu
			= per_cpu_ptr(hv_context.cpu_context, cpu);

		memset(hv_cpu, 0, sizeof(*hv_cpu));
		tasklet_init(&hv_cpu->msg_dpc,
			     vmbus_on_msg_dpc, (unsigned long) hv_cpu);

		hv_cpu->clk_evt = kzalloc(sizeof(struct clock_event_device),
					  GFP_KERNEL);
		if (hv_cpu->clk_evt == NULL) {
>>>>>>> temp
			pr_err("Unable to allocate clock event device\n");
			goto err;
		}
		hv_init_clockevent_device(hv_cpu->clk_evt, cpu);

		hv_cpu->synic_message_page =
			(void *)get_zeroed_page(GFP_ATOMIC);
		if (hv_cpu->synic_message_page == NULL) {
			pr_err("Unable to allocate SYNIC message page\n");
			goto err;
		}

		hv_cpu->synic_event_page = (void *)get_zeroed_page(GFP_ATOMIC);
		if (hv_cpu->synic_event_page == NULL) {
			pr_err("Unable to allocate SYNIC event page\n");
			goto err;
		}

		hv_cpu->post_msg_page = (void *)get_zeroed_page(GFP_ATOMIC);
		if (hv_cpu->post_msg_page == NULL) {
			pr_err("Unable to allocate post msg page\n");
			goto err;
		}

<<<<<<< HEAD
		INIT_LIST_HEAD(&hv_context.percpu_list[cpu]);
=======
		INIT_LIST_HEAD(&hv_cpu->chan_list);
>>>>>>> temp
	}

	return 0;
err:
	return -ENOMEM;
}

<<<<<<< HEAD
static void hv_synic_free_cpu(int cpu)
{
	kfree(hv_context.event_dpc[cpu]);
	kfree(hv_context.msg_dpc[cpu]);
	kfree(hv_context.clk_evt[cpu]);
	if (hv_context.synic_event_page[cpu])
		free_page((unsigned long)hv_context.synic_event_page[cpu]);
	if (hv_context.synic_message_page[cpu])
		free_page((unsigned long)hv_context.synic_message_page[cpu]);
	if (hv_context.post_msg_page[cpu])
		free_page((unsigned long)hv_context.post_msg_page[cpu]);
}
=======
>>>>>>> temp

void hv_synic_free(void)
{
	int cpu;

	for_each_present_cpu(cpu) {
		struct hv_per_cpu_context *hv_cpu
			= per_cpu_ptr(hv_context.cpu_context, cpu);

		if (hv_cpu->synic_event_page)
			free_page((unsigned long)hv_cpu->synic_event_page);
		if (hv_cpu->synic_message_page)
			free_page((unsigned long)hv_cpu->synic_message_page);
		if (hv_cpu->post_msg_page)
			free_page((unsigned long)hv_cpu->post_msg_page);
	}

	kfree(hv_context.hv_numa_map);
<<<<<<< HEAD
	for_each_present_cpu(cpu)
		hv_synic_free_cpu(cpu);
=======
>>>>>>> temp
}

/*
 * hv_synic_init - Initialize the Synthethic Interrupt Controller.
 *
 * If it is already initialized by another entity (ie x2v shim), we need to
 * retrieve the initialized message and event pages.  Otherwise, we create and
 * initialize the message and event pages.
 */
int hv_synic_init(unsigned int cpu)
{
	struct hv_per_cpu_context *hv_cpu
		= per_cpu_ptr(hv_context.cpu_context, cpu);
	union hv_synic_simp simp;
	union hv_synic_siefp siefp;
	union hv_synic_sint shared_sint;
	union hv_synic_scontrol sctrl;

	/* Setup the Synic's message page */
	hv_get_simp(simp.as_uint64);
	simp.simp_enabled = 1;
	simp.base_simp_gpa = virt_to_phys(hv_cpu->synic_message_page)
		>> PAGE_SHIFT;

	hv_set_simp(simp.as_uint64);

	/* Setup the Synic's event page */
	hv_get_siefp(siefp.as_uint64);
	siefp.siefp_enabled = 1;
	siefp.base_siefp_gpa = virt_to_phys(hv_cpu->synic_event_page)
		>> PAGE_SHIFT;

	hv_set_siefp(siefp.as_uint64);

	/* Setup the shared SINT. */
	hv_get_synint_state(HV_X64_MSR_SINT0 + VMBUS_MESSAGE_SINT,
			    shared_sint.as_uint64);

	shared_sint.as_uint64 = 0;
	shared_sint.vector = HYPERVISOR_CALLBACK_VECTOR;
	shared_sint.masked = false;
	if (ms_hyperv.hints & HV_X64_DEPRECATING_AEOI_RECOMMENDED)
		shared_sint.auto_eoi = false;
	else
		shared_sint.auto_eoi = true;

	hv_set_synint_state(HV_X64_MSR_SINT0 + VMBUS_MESSAGE_SINT,
			    shared_sint.as_uint64);

	/* Enable the global synic bit */
	hv_get_synic_state(sctrl.as_uint64);
	sctrl.enable = 1;

	hv_set_synic_state(sctrl.as_uint64);

	hv_context.synic_initialized = true;

	/*
<<<<<<< HEAD
	 * Setup the mapping between Hyper-V's notion
	 * of cpuid and Linux' notion of cpuid.
	 * This array will be indexed using Linux cpuid.
	 */
	rdmsrl(HV_X64_MSR_VP_INDEX, vp_index);
	hv_context.vp_index[cpu] = (u32)vp_index;

	/*
=======
>>>>>>> temp
	 * Register the per-cpu clockevent source.
	 */
	if (ms_hyperv.features & HV_X64_MSR_SYNTIMER_AVAILABLE)
		clockevents_config_and_register(hv_cpu->clk_evt,
						HV_TIMER_FREQUENCY,
						HV_MIN_DELTA_TICKS,
						HV_MAX_MAX_DELTA_TICKS);
	return 0;
}

/*
 * hv_synic_clockevents_cleanup - Cleanup clockevent devices
 */
void hv_synic_clockevents_cleanup(void)
{
	int cpu;

	if (!(ms_hyperv.features & HV_X64_MSR_SYNTIMER_AVAILABLE))
		return;

	for_each_present_cpu(cpu) {
		struct hv_per_cpu_context *hv_cpu
			= per_cpu_ptr(hv_context.cpu_context, cpu);

		clockevents_unbind_device(hv_cpu->clk_evt, cpu);
	}
}

/*
 * hv_synic_cleanup - Cleanup routine for hv_synic_init().
 */
int hv_synic_cleanup(unsigned int cpu)
{
	union hv_synic_sint shared_sint;
	union hv_synic_simp simp;
	union hv_synic_siefp siefp;
	union hv_synic_scontrol sctrl;
	struct vmbus_channel *channel, *sc;
	bool channel_found = false;
	unsigned long flags;

	if (!hv_context.synic_initialized)
		return -EFAULT;

	/*
	 * Search for channels which are bound to the CPU we're about to
	 * cleanup. In case we find one and vmbus is still connected we need to
	 * fail, this will effectively prevent CPU offlining. There is no way
	 * we can re-bind channels to different CPUs for now.
	 */
	mutex_lock(&vmbus_connection.channel_mutex);
	list_for_each_entry(channel, &vmbus_connection.chn_list, listentry) {
		if (channel->target_cpu == cpu) {
			channel_found = true;
			break;
		}
		spin_lock_irqsave(&channel->lock, flags);
		list_for_each_entry(sc, &channel->sc_list, sc_list) {
			if (sc->target_cpu == cpu) {
				channel_found = true;
				break;
			}
		}
		spin_unlock_irqrestore(&channel->lock, flags);
		if (channel_found)
			break;
	}
	mutex_unlock(&vmbus_connection.channel_mutex);

	if (channel_found && vmbus_connection.conn_state == CONNECTED)
		return -EBUSY;

	/* Turn off clockevent device */
	if (ms_hyperv.features & HV_X64_MSR_SYNTIMER_AVAILABLE) {
		struct hv_per_cpu_context *hv_cpu
			= this_cpu_ptr(hv_context.cpu_context);

		clockevents_unbind_device(hv_cpu->clk_evt, cpu);
		hv_ce_shutdown(hv_cpu->clk_evt);
		put_cpu_ptr(hv_cpu);
	}

	hv_get_synint_state(HV_X64_MSR_SINT0 + VMBUS_MESSAGE_SINT,
			    shared_sint.as_uint64);

	shared_sint.masked = 1;

	/* Need to correctly cleanup in the case of SMP!!! */
	/* Disable the interrupt */
	hv_set_synint_state(HV_X64_MSR_SINT0 + VMBUS_MESSAGE_SINT,
			    shared_sint.as_uint64);

	hv_get_simp(simp.as_uint64);
	simp.simp_enabled = 0;
	simp.base_simp_gpa = 0;

	hv_set_simp(simp.as_uint64);

	hv_get_siefp(siefp.as_uint64);
	siefp.siefp_enabled = 0;
	siefp.base_siefp_gpa = 0;

	hv_set_siefp(siefp.as_uint64);

	/* Disable the global synic bit */
	hv_get_synic_state(sctrl.as_uint64);
	sctrl.enable = 0;
	hv_set_synic_state(sctrl.as_uint64);

	return 0;
}
