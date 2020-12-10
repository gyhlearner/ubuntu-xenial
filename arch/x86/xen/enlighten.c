<<<<<<< HEAD
/*
 * Core of Xen paravirt_ops implementation.
 *
 * This file contains the xen_paravirt_ops structure itself, and the
 * implementations for:
 * - privileged instructions
 * - interrupt flags
 * - segment operations
 * - booting and setup
 *
 * Jeremy Fitzhardinge <jeremy@xensource.com>, XenSource Inc, 2007
 */

#include <linux/cpu.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/smp.h>
#include <linux/preempt.h>
#include <linux/hardirq.h>
#include <linux/percpu.h>
#include <linux/delay.h>
#include <linux/start_kernel.h>
#include <linux/sched.h>
#include <linux/kprobes.h>
=======
#ifdef CONFIG_XEN_BALLOON_MEMORY_HOTPLUG
>>>>>>> temp
#include <linux/bootmem.h>
#endif
#include <linux/cpu.h>
#include <linux/kexec.h>
#include <linux/slab.h>

#include <xen/features.h>
#include <xen/page.h>
#include <xen/interface/memory.h>

#include <asm/xen/hypercall.h>
#include <asm/xen/hypervisor.h>
<<<<<<< HEAD
#include <asm/fixmap.h>
#include <asm/processor.h>
#include <asm/proto.h>
#include <asm/msr-index.h>
#include <asm/traps.h>
#include <asm/setup.h>
#include <asm/desc.h>
#include <asm/pgalloc.h>
#include <asm/pgtable.h>
#include <asm/tlbflush.h>
#include <asm/reboot.h>
#include <asm/stackprotector.h>
#include <asm/hypervisor.h>
#include <asm/mach_traps.h>
#include <asm/mwait.h>
#include <asm/pci_x86.h>
=======
>>>>>>> temp
#include <asm/cpu.h>
#include <asm/e820/api.h> 

#include "xen-ops.h"
#include "smp.h"
#include "pmu.h"

EXPORT_SYMBOL_GPL(hypercall_page);

/*
 * Pointer to the xen_vcpu_info structure or
 * &HYPERVISOR_shared_info->vcpu_info[cpu]. See xen_hvm_init_shared_info
 * and xen_vcpu_setup for details. By default it points to share_info->vcpu_info
 * but if the hypervisor supports VCPUOP_register_vcpu_info then it can point
 * to xen_vcpu_info. The pointer is used in __xen_evtchn_do_upcall to
 * acknowledge pending events.
 * Also more subtly it is used by the patched version of irq enable/disable
 * e.g. xen_irq_enable_direct and xen_iret in PV mode.
 *
 * The desire to be able to do those mask/unmask operations as a single
 * instruction by using the per-cpu offset held in %gs is the real reason
 * vcpu info is in a per-cpu pointer and the original reason for this
 * hypercall.
 *
 */
DEFINE_PER_CPU(struct vcpu_info *, xen_vcpu);

/*
 * Per CPU pages used if hypervisor supports VCPUOP_register_vcpu_info
 * hypercall. This can be used both in PV and PVHVM mode. The structure
 * overrides the default per_cpu(xen_vcpu, cpu) value.
 */
DEFINE_PER_CPU(struct vcpu_info, xen_vcpu_info);

/* Linux <-> Xen vCPU id mapping */
DEFINE_PER_CPU(uint32_t, xen_vcpu_id);
EXPORT_PER_CPU_SYMBOL(xen_vcpu_id);

enum xen_domain_type xen_domain_type = XEN_NATIVE;
EXPORT_SYMBOL_GPL(xen_domain_type);

unsigned long *machine_to_phys_mapping = (void *)MACH2PHYS_VIRT_START;
EXPORT_SYMBOL(machine_to_phys_mapping);
unsigned long  machine_to_phys_nr;
EXPORT_SYMBOL(machine_to_phys_nr);

struct start_info *xen_start_info;
EXPORT_SYMBOL_GPL(xen_start_info);

struct shared_info xen_dummy_shared_info;

__read_mostly int xen_have_vector_callback;
EXPORT_SYMBOL_GPL(xen_have_vector_callback);

/*
 * Point at some empty memory to start with. We map the real shared_info
 * page as soon as fixmap is up and running.
 */
struct shared_info *HYPERVISOR_shared_info = &xen_dummy_shared_info;

/*
 * Flag to determine whether vcpu info placement is available on all
 * VCPUs.  We assume it is to start with, and then set it to zero on
 * the first failure.  This is because it can succeed on some VCPUs
 * and not others, since it can involve hypervisor memory allocation,
 * or because the guest failed to guarantee all the appropriate
 * constraints on all VCPUs (ie buffer can't cross a page boundary).
 *
 * Note that any particular CPU may be using a placed vcpu structure,
 * but we can only optimise if the all are.
 *
 * 0: not available, 1: available
 */
int xen_have_vcpu_info_placement = 1;

static int xen_cpu_up_online(unsigned int cpu)
{
	xen_init_lock_cpu(cpu);
	return 0;
}

int xen_cpuhp_setup(int (*cpu_up_prepare_cb)(unsigned int),
		    int (*cpu_dead_cb)(unsigned int))
{
	int rc;

	rc = cpuhp_setup_state_nocalls(CPUHP_XEN_PREPARE,
				       "x86/xen/guest:prepare",
				       cpu_up_prepare_cb, cpu_dead_cb);
	if (rc >= 0) {
		rc = cpuhp_setup_state_nocalls(CPUHP_AP_ONLINE_DYN,
					       "x86/xen/guest:online",
					       xen_cpu_up_online, NULL);
		if (rc < 0)
			cpuhp_remove_state_nocalls(CPUHP_XEN_PREPARE);
	}

	return rc >= 0 ? 0 : rc;
}

static int xen_vcpu_setup_restore(int cpu)
{
	int rc = 0;

	/* Any per_cpu(xen_vcpu) is stale, so reset it */
	xen_vcpu_info_reset(cpu);

	/*
	 * For PVH and PVHVM, setup online VCPUs only. The rest will
	 * be handled by hotplug.
	 */
	if (xen_pv_domain() ||
	    (xen_hvm_domain() && cpu_online(cpu))) {
		rc = xen_vcpu_setup(cpu);
	}

	return rc;
}

/*
 * On restore, set the vcpu placement up again.
 * If it fails, then we're in a bad state, since
 * we can't back out from using it...
 */
void xen_vcpu_restore(void)
{
	int cpu, rc;

	for_each_possible_cpu(cpu) {
		bool other_cpu = (cpu != smp_processor_id());
		bool is_up;

		if (xen_vcpu_nr(cpu) == XEN_VCPU_ID_INVALID)
			continue;

		/* Only Xen 4.5 and higher support this. */
		is_up = HYPERVISOR_vcpu_op(VCPUOP_is_up,
					   xen_vcpu_nr(cpu), NULL) > 0;

		if (other_cpu && is_up &&
		    HYPERVISOR_vcpu_op(VCPUOP_down, xen_vcpu_nr(cpu), NULL))
			BUG();

		if (xen_pv_domain() || xen_feature(XENFEAT_hvm_safe_pvclock))
			xen_setup_runstate_info(cpu);

		rc = xen_vcpu_setup_restore(cpu);
		if (rc)
			pr_emerg_once("vcpu restore failed for cpu=%d err=%d. "
					"System will hang.\n", cpu, rc);
		/*
		 * In case xen_vcpu_setup_restore() fails, do not bring up the
		 * VCPU. This helps us avoid the resulting OOPS when the VCPU
		 * accesses pvclock_vcpu_time via xen_vcpu (which is NULL.)
		 * Note that this does not improve the situation much -- now the
		 * VM hangs instead of OOPSing -- with the VCPUs that did not
		 * fail, spinning in stop_machine(), waiting for the failed
		 * VCPUs to come up.
		 */
		if (other_cpu && is_up && (rc == 0) &&
		    HYPERVISOR_vcpu_op(VCPUOP_up, xen_vcpu_nr(cpu), NULL))
			BUG();
	}
}

void xen_vcpu_info_reset(int cpu)
{
	if (xen_vcpu_nr(cpu) < MAX_VIRT_CPUS) {
		per_cpu(xen_vcpu, cpu) =
			&HYPERVISOR_shared_info->vcpu_info[xen_vcpu_nr(cpu)];
	} else {
		/* Set to NULL so that if somebody accesses it we get an OOPS */
		per_cpu(xen_vcpu, cpu) = NULL;
	}
<<<<<<< HEAD
	return true;
#else
	return false;
#endif
}
static void __init xen_init_cpuid_mask(void)
{
	unsigned int ax, bx, cx, dx;
	unsigned int xsave_mask;

	cpuid_leaf1_edx_mask =
		~((1 << X86_FEATURE_MTRR) |  /* disable MTRR */
		  (1 << X86_FEATURE_ACC));   /* thermal monitoring */

	/*
	 * Xen PV would need some work to support PCID: CR3 handling as well
	 * as xen_flush_tlb_others() would need updating.
	 */
	cpuid_leaf1_ecx_mask &= ~(1 << (X86_FEATURE_PCID % 32));  /* disable PCID */

	if (!xen_initial_domain())
		cpuid_leaf1_edx_mask &=
			~((1 << X86_FEATURE_ACPI));  /* disable ACPI */

	cpuid_leaf1_ecx_mask &= ~(1 << (X86_FEATURE_X2APIC % 32));

	ax = 1;
	cx = 0;
	cpuid(1, &ax, &bx, &cx, &dx);

	xsave_mask =
		(1 << (X86_FEATURE_XSAVE % 32)) |
		(1 << (X86_FEATURE_OSXSAVE % 32));

	/* Xen will set CR4.OSXSAVE if supported and not disabled by force */
	if ((cx & xsave_mask) != xsave_mask)
		cpuid_leaf1_ecx_mask &= ~xsave_mask; /* disable XSAVE & OSXSAVE */
	if (xen_check_mwait())
		cpuid_leaf1_ecx_set_mask = (1 << (X86_FEATURE_MWAIT % 32));
}

static void __init xen_init_capabilities(void)
{
	if (xen_pv_domain())
		setup_force_cpu_cap(X86_FEATURE_XENPV);
}

static void xen_set_debugreg(int reg, unsigned long val)
{
	HYPERVISOR_set_debugreg(reg, val);
}

static unsigned long xen_get_debugreg(int reg)
{
	return HYPERVISOR_get_debugreg(reg);
}

static void xen_end_context_switch(struct task_struct *next)
{
	xen_mc_flush();
	paravirt_end_context_switch(next);
}

static unsigned long xen_store_tr(void)
{
	return 0;
=======
>>>>>>> temp
}

int xen_vcpu_setup(int cpu)
{
	struct vcpu_register_vcpu_info info;
	int err;
	struct vcpu_info *vcpup;

	BUG_ON(HYPERVISOR_shared_info == &xen_dummy_shared_info);

	/*
	 * This path is called on PVHVM at bootup (xen_hvm_smp_prepare_boot_cpu)
	 * and at restore (xen_vcpu_restore). Also called for hotplugged
	 * VCPUs (cpu_init -> xen_hvm_cpu_prepare_hvm).
	 * However, the hypercall can only be done once (see below) so if a VCPU
	 * is offlined and comes back online then let's not redo the hypercall.
	 *
	 * For PV it is called during restore (xen_vcpu_restore) and bootup
	 * (xen_setup_vcpu_info_placement). The hotplug mechanism does not
	 * use this function.
	 */
	if (xen_hvm_domain()) {
		if (per_cpu(xen_vcpu, cpu) == &per_cpu(xen_vcpu_info, cpu))
			return 0;
	}

	if (xen_have_vcpu_info_placement) {
		vcpup = &per_cpu(xen_vcpu_info, cpu);
		info.mfn = arbitrary_virt_to_mfn(vcpup);
		info.offset = offset_in_page(vcpup);

		/*
		 * Check to see if the hypervisor will put the vcpu_info
		 * structure where we want it, which allows direct access via
		 * a percpu-variable.
		 * N.B. This hypercall can _only_ be called once per CPU.
		 * Subsequent calls will error out with -EINVAL. This is due to
		 * the fact that hypervisor has no unregister variant and this
		 * hypercall does not allow to over-write info.mfn and
		 * info.offset.
		 */
		err = HYPERVISOR_vcpu_op(VCPUOP_register_vcpu_info,
					 xen_vcpu_nr(cpu), &info);

		if (err) {
			pr_warn_once("register_vcpu_info failed: cpu=%d err=%d\n",
				     cpu, err);
			xen_have_vcpu_info_placement = 0;
		} else {
			/*
			 * This cpu is using the registered vcpu info, even if
			 * later ones fail to.
			 */
			per_cpu(xen_vcpu, cpu) = vcpup;
		}
	}

	if (!xen_have_vcpu_info_placement)
		xen_vcpu_info_reset(cpu);

	return ((per_cpu(xen_vcpu, cpu) == NULL) ? -ENODEV : 0);
}

void xen_reboot(int reason)
{
	struct sched_shutdown r = { .reason = reason };
	int cpu;

	for_each_online_cpu(cpu)
		xen_pmu_finish(cpu);

	if (HYPERVISOR_sched_op(SCHEDOP_shutdown, &r))
		BUG();
}

void xen_emergency_restart(void)
{
	xen_reboot(SHUTDOWN_reboot);
}

static int
xen_panic_event(struct notifier_block *this, unsigned long event, void *ptr)
{
	if (!kexec_crash_loaded())
		xen_reboot(SHUTDOWN_crash);
	return NOTIFY_DONE;
}

static struct notifier_block xen_panic_block = {
	.notifier_call = xen_panic_event,
	.priority = INT_MIN
};

int xen_panic_handler_init(void)
{
	atomic_notifier_chain_register(&panic_notifier_list, &xen_panic_block);
	return 0;
}

void xen_pin_vcpu(int cpu)
{
	static bool disable_pinning;
	struct sched_pin_override pin_override;
	int ret;

	if (disable_pinning)
		return;

	pin_override.pcpu = cpu;
	ret = HYPERVISOR_sched_op(SCHEDOP_pin_override, &pin_override);

	/* Ignore errors when removing override. */
	if (cpu < 0)
		return;

	switch (ret) {
	case -ENOSYS:
		pr_warn("Unable to pin on physical cpu %d. In case of problems consider vcpu pinning.\n",
			cpu);
		disable_pinning = true;
		break;
	case -EPERM:
		WARN(1, "Trying to pin vcpu without having privilege to do so\n");
		disable_pinning = true;
		break;
	case -EINVAL:
	case -EBUSY:
		pr_warn("Physical cpu %d not available for pinning. Check Xen cpu configuration.\n",
			cpu);
		break;
	case 0:
		break;
	default:
		WARN(1, "rc %d while trying to pin vcpu\n", ret);
		disable_pinning = true;
	}
}

#ifdef CONFIG_HOTPLUG_CPU
void xen_arch_register_cpu(int num)
{
	arch_register_cpu(num);
}
EXPORT_SYMBOL(xen_arch_register_cpu);

void xen_arch_unregister_cpu(int num)
{
	arch_unregister_cpu(num);
}
EXPORT_SYMBOL(xen_arch_unregister_cpu);
#endif

#ifdef CONFIG_XEN_BALLOON_MEMORY_HOTPLUG
void __init arch_xen_balloon_init(struct resource *hostmem_resource)
{
	struct xen_memory_map memmap;
	int rc;
	unsigned int i, last_guest_ram;
	phys_addr_t max_addr = PFN_PHYS(max_pfn);
	struct e820_table *xen_e820_table;
	const struct e820_entry *entry;
	struct resource *res;

	if (!xen_initial_domain())
		return;

	xen_e820_table = kmalloc(sizeof(*xen_e820_table), GFP_KERNEL);
	if (!xen_e820_table)
		return;

	memmap.nr_entries = ARRAY_SIZE(xen_e820_table->entries);
	set_xen_guest_handle(memmap.buffer, xen_e820_table->entries);
	rc = HYPERVISOR_memory_op(XENMEM_machine_memory_map, &memmap);
	if (rc) {
		pr_warn("%s: Can't read host e820 (%d)\n", __func__, rc);
		goto out;
	}

	last_guest_ram = 0;
	for (i = 0; i < memmap.nr_entries; i++) {
		if (xen_e820_table->entries[i].addr >= max_addr)
			break;
		if (xen_e820_table->entries[i].type == E820_TYPE_RAM)
			last_guest_ram = i;
	}

	entry = &xen_e820_table->entries[last_guest_ram];
	if (max_addr >= entry->addr + entry->size)
		goto out; /* No unallocated host RAM. */

	hostmem_resource->start = max_addr;
	hostmem_resource->end = entry->addr + entry->size;

	/*
	 * Mark non-RAM regions between the end of dom0 RAM and end of host RAM
	 * as unavailable. The rest of that region can be used for hotplug-based
	 * ballooning.
	 */
	for (; i < memmap.nr_entries; i++) {
		entry = &xen_e820_table->entries[i];

		if (entry->type == E820_TYPE_RAM)
			continue;

		if (entry->addr >= hostmem_resource->end)
			break;

		res = kzalloc(sizeof(*res), GFP_KERNEL);
		if (!res)
			goto out;

		res->name = "Unavailable host RAM";
		res->start = entry->addr;
		res->end = (entry->addr + entry->size < hostmem_resource->end) ?
			    entry->addr + entry->size : hostmem_resource->end;
		rc = insert_resource(hostmem_resource, res);
		if (rc) {
			pr_warn("%s: Can't insert [%llx - %llx) (%d)\n",
				__func__, res->start, res->end, rc);
			kfree(res);
			goto  out;
		}
	}

 out:
	kfree(xen_e820_table);
}
<<<<<<< HEAD

static void xen_convert_trap_info(const struct desc_ptr *desc,
				  struct trap_info *traps)
{
	unsigned in, out, count;

	count = (desc->size+1) / sizeof(gate_desc);
	BUG_ON(count > 256);

	for (in = out = 0; in < count; in++) {
		gate_desc *entry = (gate_desc*)(desc->address) + in;

		if (cvt_gate_to_trap(in, entry, &traps[out]))
			out++;
	}
	traps[out].address = 0;
}

void xen_copy_trap_info(struct trap_info *traps)
{
	const struct desc_ptr *desc = this_cpu_ptr(&idt_desc);

	xen_convert_trap_info(desc, traps);
}

/* Load a new IDT into Xen.  In principle this can be per-CPU, so we
   hold a spinlock to protect the static traps[] array (static because
   it avoids allocation, and saves stack space). */
static void xen_load_idt(const struct desc_ptr *desc)
{
	static DEFINE_SPINLOCK(lock);
	static struct trap_info traps[257];

	trace_xen_cpu_load_idt(desc);

	spin_lock(&lock);

	memcpy(this_cpu_ptr(&idt_desc), desc, sizeof(idt_desc));

	xen_convert_trap_info(desc, traps);

	xen_mc_flush();
	if (HYPERVISOR_set_trap_table(traps))
		BUG();

	spin_unlock(&lock);
}

/* Write a GDT descriptor entry.  Ignore LDT descriptors, since
   they're handled differently. */
static void xen_write_gdt_entry(struct desc_struct *dt, int entry,
				const void *desc, int type)
{
	trace_xen_cpu_write_gdt_entry(dt, entry, desc, type);

	preempt_disable();

	switch (type) {
	case DESC_LDT:
	case DESC_TSS:
		/* ignore */
		break;

	default: {
		xmaddr_t maddr = arbitrary_virt_to_machine(&dt[entry]);

		xen_mc_flush();
		if (HYPERVISOR_update_descriptor(maddr.maddr, *(u64 *)desc))
			BUG();
	}

	}

	preempt_enable();
}

/*
 * Version of write_gdt_entry for use at early boot-time needed to
 * update an entry as simply as possible.
 */
static void __init xen_write_gdt_entry_boot(struct desc_struct *dt, int entry,
					    const void *desc, int type)
{
	trace_xen_cpu_write_gdt_entry(dt, entry, desc, type);

	switch (type) {
	case DESC_LDT:
	case DESC_TSS:
		/* ignore */
		break;

	default: {
		xmaddr_t maddr = virt_to_machine(&dt[entry]);

		if (HYPERVISOR_update_descriptor(maddr.maddr, *(u64 *)desc))
			dt[entry] = *(struct desc_struct *)desc;
	}

	}
}

static void xen_load_sp0(struct tss_struct *tss,
			 struct thread_struct *thread)
{
	struct multicall_space mcs;

	mcs = xen_mc_entry(0);
	MULTI_stack_switch(mcs.mc, __KERNEL_DS, thread->sp0);
	xen_mc_issue(PARAVIRT_LAZY_CPU);
	tss->x86_tss.sp0 = thread->sp0;
}

void xen_set_iopl_mask(unsigned mask)
{
	struct physdev_set_iopl set_iopl;

	/* Force the change at ring 0. */
	set_iopl.iopl = (mask == 0) ? 1 : (mask >> 12) & 3;
	HYPERVISOR_physdev_op(PHYSDEVOP_set_iopl, &set_iopl);
}

static void xen_io_delay(void)
{
}

static void xen_clts(void)
{
	struct multicall_space mcs;

	mcs = xen_mc_entry(0);

	MULTI_fpu_taskswitch(mcs.mc, 0);

	xen_mc_issue(PARAVIRT_LAZY_CPU);
}

static DEFINE_PER_CPU(unsigned long, xen_cr0_value);

static unsigned long xen_read_cr0(void)
{
	unsigned long cr0 = this_cpu_read(xen_cr0_value);

	if (unlikely(cr0 == 0)) {
		cr0 = native_read_cr0();
		this_cpu_write(xen_cr0_value, cr0);
	}

	return cr0;
}

static void xen_write_cr0(unsigned long cr0)
{
	struct multicall_space mcs;

	this_cpu_write(xen_cr0_value, cr0);

	/* Only pay attention to cr0.TS; everything else is
	   ignored. */
	mcs = xen_mc_entry(0);

	MULTI_fpu_taskswitch(mcs.mc, (cr0 & X86_CR0_TS) != 0);

	xen_mc_issue(PARAVIRT_LAZY_CPU);
}

static void xen_write_cr4(unsigned long cr4)
{
	cr4 &= ~(X86_CR4_PGE | X86_CR4_PSE | X86_CR4_PCE);

	native_write_cr4(cr4);
}
#ifdef CONFIG_X86_64
static inline unsigned long xen_read_cr8(void)
{
	return 0;
}
static inline void xen_write_cr8(unsigned long val)
{
	BUG_ON(val);
}
#endif

static u64 xen_read_msr_safe(unsigned int msr, int *err)
{
	u64 val;

	if (pmu_msr_read(msr, &val, err))
		return val;

	val = native_read_msr_safe(msr, err);
	switch (msr) {
	case MSR_IA32_APICBASE:
#ifdef CONFIG_X86_X2APIC
		if (!(cpuid_ecx(1) & (1 << (X86_FEATURE_X2APIC & 31))))
#endif
			val &= ~X2APIC_ENABLE;
		break;
	}
	return val;
}

static int xen_write_msr_safe(unsigned int msr, unsigned low, unsigned high)
{
	int ret;

	ret = 0;

	switch (msr) {
#ifdef CONFIG_X86_64
		unsigned which;
		u64 base;

	case MSR_FS_BASE:		which = SEGBASE_FS; goto set;
	case MSR_KERNEL_GS_BASE:	which = SEGBASE_GS_USER; goto set;
	case MSR_GS_BASE:		which = SEGBASE_GS_KERNEL; goto set;

	set:
		base = ((u64)high << 32) | low;
		if (HYPERVISOR_set_segment_base(which, base) != 0)
			ret = -EIO;
		break;
#endif

	case MSR_STAR:
	case MSR_CSTAR:
	case MSR_LSTAR:
	case MSR_SYSCALL_MASK:
	case MSR_IA32_SYSENTER_CS:
	case MSR_IA32_SYSENTER_ESP:
	case MSR_IA32_SYSENTER_EIP:
		/* Fast syscall setup is all done in hypercalls, so
		   these are all ignored.  Stub them out here to stop
		   Xen console noise. */
		break;

	default:
		if (!pmu_msr_write(msr, low, high, &ret))
			ret = native_write_msr_safe(msr, low, high);
	}

	return ret;
}

void xen_setup_shared_info(void)
{
	if (!xen_feature(XENFEAT_auto_translated_physmap)) {
		set_fixmap(FIX_PARAVIRT_BOOTMAP,
			   xen_start_info->shared_info);

		HYPERVISOR_shared_info =
			(struct shared_info *)fix_to_virt(FIX_PARAVIRT_BOOTMAP);
	} else
		HYPERVISOR_shared_info =
			(struct shared_info *)__va(xen_start_info->shared_info);

#ifndef CONFIG_SMP
	/* In UP this is as good a place as any to set up shared info */
	xen_setup_vcpu_info_placement();
#endif

	xen_setup_mfn_list_list();
}

/* This is called once we have the cpu_possible_mask */
void xen_setup_vcpu_info_placement(void)
{
	int cpu;

	for_each_possible_cpu(cpu)
		xen_vcpu_setup(cpu);

	/* xen_vcpu_setup managed to place the vcpu_info within the
	 * percpu area for all cpus, so make use of it. Note that for
	 * PVH we want to use native IRQ mechanism. */
	if (have_vcpu_info_placement && !xen_pvh_domain()) {
		pv_irq_ops.save_fl = __PV_IS_CALLEE_SAVE(xen_save_fl_direct);
		pv_irq_ops.restore_fl = __PV_IS_CALLEE_SAVE(xen_restore_fl_direct);
		pv_irq_ops.irq_disable = __PV_IS_CALLEE_SAVE(xen_irq_disable_direct);
		pv_irq_ops.irq_enable = __PV_IS_CALLEE_SAVE(xen_irq_enable_direct);
		pv_mmu_ops.read_cr2 = xen_read_cr2_direct;
	}
}

static unsigned xen_patch(u8 type, u16 clobbers, void *insnbuf,
			  unsigned long addr, unsigned len)
{
	char *start, *end, *reloc;
	unsigned ret;

	start = end = reloc = NULL;

#define SITE(op, x)							\
	case PARAVIRT_PATCH(op.x):					\
	if (have_vcpu_info_placement) {					\
		start = (char *)xen_##x##_direct;			\
		end = xen_##x##_direct_end;				\
		reloc = xen_##x##_direct_reloc;				\
	}								\
	goto patch_site

	switch (type) {
		SITE(pv_irq_ops, irq_enable);
		SITE(pv_irq_ops, irq_disable);
		SITE(pv_irq_ops, save_fl);
		SITE(pv_irq_ops, restore_fl);
#undef SITE

	patch_site:
		if (start == NULL || (end-start) > len)
			goto default_patch;

		ret = paravirt_patch_insns(insnbuf, len, start, end);

		/* Note: because reloc is assigned from something that
		   appears to be an array, gcc assumes it's non-null,
		   but doesn't know its relationship with start and
		   end. */
		if (reloc > start && reloc < end) {
			int reloc_off = reloc - start;
			long *relocp = (long *)(insnbuf + reloc_off);
			long delta = start - (char *)addr;

			*relocp += delta;
		}
		break;

	default_patch:
	default:
		ret = paravirt_patch_default(type, clobbers, insnbuf,
					     addr, len);
		break;
	}

	return ret;
}

static const struct pv_info xen_info __initconst = {
	.paravirt_enabled = 1,
	.shared_kernel_pmd = 0,

#ifdef CONFIG_X86_64
	.extra_user_64bit_cs = FLAT_USER_CS64,
#endif
	.features = 0,
	.name = "Xen",
};

static const struct pv_init_ops xen_init_ops __initconst = {
	.patch = xen_patch,
};

static const struct pv_cpu_ops xen_cpu_ops __initconst = {
	.cpuid = xen_cpuid,

	.set_debugreg = xen_set_debugreg,
	.get_debugreg = xen_get_debugreg,

	.clts = xen_clts,

	.read_cr0 = xen_read_cr0,
	.write_cr0 = xen_write_cr0,

	.read_cr4 = native_read_cr4,
	.read_cr4_safe = native_read_cr4_safe,
	.write_cr4 = xen_write_cr4,

#ifdef CONFIG_X86_64
	.read_cr8 = xen_read_cr8,
	.write_cr8 = xen_write_cr8,
#endif

	.wbinvd = native_wbinvd,

	.read_msr = xen_read_msr_safe,
	.write_msr = xen_write_msr_safe,

	.read_pmc = xen_read_pmc,

	.iret = xen_iret,
#ifdef CONFIG_X86_64
	.usergs_sysret32 = xen_sysret32,
	.usergs_sysret64 = xen_sysret64,
#else
	.irq_enable_sysexit = xen_sysexit,
#endif

	.load_tr_desc = paravirt_nop,
	.set_ldt = xen_set_ldt,
	.load_gdt = xen_load_gdt,
	.load_idt = xen_load_idt,
	.load_tls = xen_load_tls,
#ifdef CONFIG_X86_64
	.load_gs_index = xen_load_gs_index,
#endif

	.alloc_ldt = xen_alloc_ldt,
	.free_ldt = xen_free_ldt,

	.store_idt = native_store_idt,
	.store_tr = xen_store_tr,

	.write_ldt_entry = xen_write_ldt_entry,
	.write_gdt_entry = xen_write_gdt_entry,
	.write_idt_entry = xen_write_idt_entry,
	.load_sp0 = xen_load_sp0,

	.set_iopl_mask = xen_set_iopl_mask,
	.io_delay = xen_io_delay,

	/* Xen takes care of %gs when switching to usermode for us */
	.swapgs = paravirt_nop,

	.start_context_switch = paravirt_start_context_switch,
	.end_context_switch = xen_end_context_switch,
};

static const struct pv_apic_ops xen_apic_ops __initconst = {
#ifdef CONFIG_X86_LOCAL_APIC
	.startup_ipi_hook = paravirt_nop,
#endif
};

static void xen_reboot(int reason)
{
	struct sched_shutdown r = { .reason = reason };
	int cpu;

	for_each_online_cpu(cpu)
		xen_pmu_finish(cpu);

	if (HYPERVISOR_sched_op(SCHEDOP_shutdown, &r))
		BUG();
}

static void xen_restart(char *msg)
{
	xen_reboot(SHUTDOWN_reboot);
}

static void xen_emergency_restart(void)
{
	xen_reboot(SHUTDOWN_reboot);
}

static void xen_machine_halt(void)
{
	xen_reboot(SHUTDOWN_poweroff);
}

static void xen_machine_power_off(void)
{
	if (pm_power_off)
		pm_power_off();
	xen_reboot(SHUTDOWN_poweroff);
}

static void xen_crash_shutdown(struct pt_regs *regs)
{
	xen_reboot(SHUTDOWN_crash);
}

static int
xen_panic_event(struct notifier_block *this, unsigned long event, void *ptr)
{
	xen_reboot(SHUTDOWN_crash);
	return NOTIFY_DONE;
}

static struct notifier_block xen_panic_block = {
	.notifier_call= xen_panic_event,
	.priority = INT_MIN
};

int xen_panic_handler_init(void)
{
	atomic_notifier_chain_register(&panic_notifier_list, &xen_panic_block);
	return 0;
}

static const struct machine_ops xen_machine_ops __initconst = {
	.restart = xen_restart,
	.halt = xen_machine_halt,
	.power_off = xen_machine_power_off,
	.shutdown = xen_machine_halt,
	.crash_shutdown = xen_crash_shutdown,
	.emergency_restart = xen_emergency_restart,
};

static unsigned char xen_get_nmi_reason(void)
{
	unsigned char reason = 0;

	/* Construct a value which looks like it came from port 0x61. */
	if (test_bit(_XEN_NMIREASON_io_error,
		     &HYPERVISOR_shared_info->arch.nmi_reason))
		reason |= NMI_REASON_IOCHK;
	if (test_bit(_XEN_NMIREASON_pci_serr,
		     &HYPERVISOR_shared_info->arch.nmi_reason))
		reason |= NMI_REASON_SERR;

	return reason;
}

static void __init xen_boot_params_init_edd(void)
{
#if IS_ENABLED(CONFIG_EDD)
	struct xen_platform_op op;
	struct edd_info *edd_info;
	u32 *mbr_signature;
	unsigned nr;
	int ret;

	edd_info = boot_params.eddbuf;
	mbr_signature = boot_params.edd_mbr_sig_buffer;

	op.cmd = XENPF_firmware_info;

	op.u.firmware_info.type = XEN_FW_DISK_INFO;
	for (nr = 0; nr < EDDMAXNR; nr++) {
		struct edd_info *info = edd_info + nr;

		op.u.firmware_info.index = nr;
		info->params.length = sizeof(info->params);
		set_xen_guest_handle(op.u.firmware_info.u.disk_info.edd_params,
				     &info->params);
		ret = HYPERVISOR_dom0_op(&op);
		if (ret)
			break;

#define C(x) info->x = op.u.firmware_info.u.disk_info.x
		C(device);
		C(version);
		C(interface_support);
		C(legacy_max_cylinder);
		C(legacy_max_head);
		C(legacy_sectors_per_track);
#undef C
	}
	boot_params.eddbuf_entries = nr;

	op.u.firmware_info.type = XEN_FW_DISK_MBR_SIGNATURE;
	for (nr = 0; nr < EDD_MBR_SIG_MAX; nr++) {
		op.u.firmware_info.index = nr;
		ret = HYPERVISOR_dom0_op(&op);
		if (ret)
			break;
		mbr_signature[nr] = op.u.firmware_info.u.disk_mbr_signature.mbr_signature;
	}
	boot_params.edd_mbr_sig_buf_entries = nr;
#endif
}

/*
 * Set up the GDT and segment registers for -fstack-protector.  Until
 * we do this, we have to be careful not to call any stack-protected
 * function, which is most of the kernel.
 *
 * Note, that it is __ref because the only caller of this after init
 * is PVH which is not going to use xen_load_gdt_boot or other
 * __init functions.
 */
static void __ref xen_setup_gdt(int cpu)
{
	if (xen_feature(XENFEAT_auto_translated_physmap)) {
#ifdef CONFIG_X86_64
		unsigned long dummy;

		load_percpu_segment(cpu); /* We need to access per-cpu area */
		switch_to_new_gdt(cpu); /* GDT and GS set */

		/* We are switching of the Xen provided GDT to our HVM mode
		 * GDT. The new GDT has  __KERNEL_CS with CS.L = 1
		 * and we are jumping to reload it.
		 */
		asm volatile ("pushq %0\n"
			      "leaq 1f(%%rip),%0\n"
			      "pushq %0\n"
			      "lretq\n"
			      "1:\n"
			      : "=&r" (dummy) : "0" (__KERNEL_CS));

		/*
		 * While not needed, we also set the %es, %ds, and %fs
		 * to zero. We don't care about %ss as it is NULL.
		 * Strictly speaking this is not needed as Xen zeros those
		 * out (and also MSR_FS_BASE, MSR_GS_BASE, MSR_KERNEL_GS_BASE)
		 *
		 * Linux zeros them in cpu_init() and in secondary_startup_64
		 * (for BSP).
		 */
		loadsegment(es, 0);
		loadsegment(ds, 0);
		loadsegment(fs, 0);
#else
		/* PVH: TODO Implement. */
		BUG();
#endif
		return; /* PVH does not need any PV GDT ops. */
	}
	pv_cpu_ops.write_gdt_entry = xen_write_gdt_entry_boot;
	pv_cpu_ops.load_gdt = xen_load_gdt_boot;

	setup_stack_canary_segment(0);
	switch_to_new_gdt(0);

	pv_cpu_ops.write_gdt_entry = xen_write_gdt_entry;
	pv_cpu_ops.load_gdt = xen_load_gdt;
}

#ifdef CONFIG_XEN_PVH
/*
 * A PV guest starts with default flags that are not set for PVH, set them
 * here asap.
 */
static void xen_pvh_set_cr_flags(int cpu)
{

	/* Some of these are setup in 'secondary_startup_64'. The others:
	 * X86_CR0_TS, X86_CR0_PE, X86_CR0_ET are set by Xen for HVM guests
	 * (which PVH shared codepaths), while X86_CR0_PG is for PVH. */
	write_cr0(read_cr0() | X86_CR0_MP | X86_CR0_NE | X86_CR0_WP | X86_CR0_AM);

	if (!cpu)
		return;
	/*
	 * For BSP, PSE PGE are set in probe_page_size_mask(), for APs
	 * set them here. For all, OSFXSR OSXMMEXCPT are set in fpu__init_cpu().
	*/
	if (cpu_has_pse)
		cr4_set_bits_and_update_boot(X86_CR4_PSE);

	if (cpu_has_pge)
		cr4_set_bits_and_update_boot(X86_CR4_PGE);
}

/*
 * Note, that it is ref - because the only caller of this after init
 * is PVH which is not going to use xen_load_gdt_boot or other
 * __init functions.
 */
void __ref xen_pvh_secondary_vcpu_init(int cpu)
{
	xen_setup_gdt(cpu);
	xen_pvh_set_cr_flags(cpu);
}

static void __init xen_pvh_early_guest_init(void)
{
	if (!xen_feature(XENFEAT_auto_translated_physmap))
		return;

	if (!xen_feature(XENFEAT_hvm_callback_vector))
		return;

	xen_have_vector_callback = 1;

	xen_pvh_early_cpu_init(0, false);
	xen_pvh_set_cr_flags(0);

#ifdef CONFIG_X86_32
	BUG(); /* PVH: Implement proper support. */
#endif
}
#endif    /* CONFIG_XEN_PVH */

/* First C function to be called on Xen boot */
asmlinkage __visible void __init xen_start_kernel(void)
{
	struct physdev_set_iopl set_iopl;
	unsigned long initrd_start = 0;
	int rc;

	if (!xen_start_info)
		return;

	xen_domain_type = XEN_PV_DOMAIN;

	xen_setup_features();
#ifdef CONFIG_XEN_PVH
	xen_pvh_early_guest_init();
#endif
	xen_setup_machphys_mapping();

	/* Install Xen paravirt ops */
	pv_info = xen_info;
	if (xen_initial_domain())
		pv_info.features |= PV_SUPPORTED_RTC;
	pv_init_ops = xen_init_ops;
	pv_apic_ops = xen_apic_ops;
	if (!xen_pvh_domain()) {
		pv_cpu_ops = xen_cpu_ops;

		x86_platform.get_nmi_reason = xen_get_nmi_reason;
	}

	if (xen_feature(XENFEAT_auto_translated_physmap))
		x86_init.resources.memory_setup = xen_auto_xlated_memory_setup;
	else
		x86_init.resources.memory_setup = xen_memory_setup;
	x86_init.oem.arch_setup = xen_arch_setup;
	x86_init.oem.banner = xen_banner;

	xen_init_time_ops();

	/*
	 * Set up some pagetable state before starting to set any ptes.
	 */

	xen_init_mmu_ops();

	/* Prevent unwanted bits from being set in PTEs. */
	__supported_pte_mask &= ~_PAGE_GLOBAL;

	/*
	 * Prevent page tables from being allocated in highmem, even
	 * if CONFIG_HIGHPTE is enabled.
	 */
	__userpte_alloc_gfp &= ~__GFP_HIGHMEM;

	/* Work out if we support NX */
	x86_configure_nx();

	/* Get mfn list */
	xen_build_dynamic_phys_to_machine();

	/*
	 * Set up kernel GDT and segment registers, mainly so that
	 * -fstack-protector code can be executed.
	 */
	xen_setup_gdt(0);

	xen_init_irq_ops();
	xen_init_cpuid_mask();
	xen_init_capabilities();

#ifdef CONFIG_X86_LOCAL_APIC
	/*
	 * set up the basic apic ops.
	 */
	xen_init_apic();
#endif

	if (xen_feature(XENFEAT_mmu_pt_update_preserve_ad)) {
		pv_mmu_ops.ptep_modify_prot_start = xen_ptep_modify_prot_start;
		pv_mmu_ops.ptep_modify_prot_commit = xen_ptep_modify_prot_commit;
	}

	machine_ops = xen_machine_ops;

	/*
	 * The only reliable way to retain the initial address of the
	 * percpu gdt_page is to remember it here, so we can go and
	 * mark it RW later, when the initial percpu area is freed.
	 */
	xen_initial_gdt = &per_cpu(gdt_page, 0);

	xen_smp_init();

#ifdef CONFIG_ACPI_NUMA
	/*
	 * The pages we from Xen are not related to machine pages, so
	 * any NUMA information the kernel tries to get from ACPI will
	 * be meaningless.  Prevent it from trying.
	 */
	acpi_numa = -1;
#endif
	/* Don't do the full vcpu_info placement stuff until we have a
	   possible map and a non-dummy shared_info. */
	per_cpu(xen_vcpu, 0) = &HYPERVISOR_shared_info->vcpu_info[0];

	local_irq_disable();
	early_boot_irqs_disabled = true;

	xen_raw_console_write("mapping kernel into physical memory\n");
	xen_setup_kernel_pagetable((pgd_t *)xen_start_info->pt_base,
				   xen_start_info->nr_pages);
	xen_reserve_special_pages();

	/* keep using Xen gdt for now; no urgent need to change it */

#ifdef CONFIG_X86_32
	pv_info.kernel_rpl = 1;
	if (xen_feature(XENFEAT_supervisor_mode_kernel))
		pv_info.kernel_rpl = 0;
#else
	pv_info.kernel_rpl = 0;
#endif
	/* set the limit of our address space */
	xen_reserve_top();

	/* PVH: runs at default kernel iopl of 0 */
	if (!xen_pvh_domain()) {
		/*
		 * We used to do this in xen_arch_setup, but that is too late
		 * on AMD were early_cpu_init (run before ->arch_setup()) calls
		 * early_amd_init which pokes 0xcf8 port.
		 */
		set_iopl.iopl = 1;
		rc = HYPERVISOR_physdev_op(PHYSDEVOP_set_iopl, &set_iopl);
		if (rc != 0)
			xen_raw_printk("physdev_op failed %d\n", rc);
	}

#ifdef CONFIG_X86_32
	/* set up basic CPUID stuff */
	cpu_detect(&new_cpu_data);
	set_cpu_cap(&new_cpu_data, X86_FEATURE_FPU);
	new_cpu_data.wp_works_ok = 1;
	new_cpu_data.x86_capability[0] = cpuid_edx(1);
#endif

	if (xen_start_info->mod_start) {
	    if (xen_start_info->flags & SIF_MOD_START_PFN)
		initrd_start = PFN_PHYS(xen_start_info->mod_start);
	    else
		initrd_start = __pa(xen_start_info->mod_start);
	}

	/* Poke various useful things into boot_params */
	boot_params.hdr.type_of_loader = (9 << 4) | 0;
	boot_params.hdr.ramdisk_image = initrd_start;
	boot_params.hdr.ramdisk_size = xen_start_info->mod_len;
	boot_params.hdr.cmd_line_ptr = __pa(xen_start_info->cmd_line);

	if (!xen_initial_domain()) {
		add_preferred_console("xenboot", 0, NULL);
		add_preferred_console("tty", 0, NULL);
		add_preferred_console("hvc", 0, NULL);
		if (pci_xen)
			x86_init.pci.arch_init = pci_xen_init;
	} else {
		const struct dom0_vga_console_info *info =
			(void *)((char *)xen_start_info +
				 xen_start_info->console.dom0.info_off);
		struct xen_platform_op op = {
			.cmd = XENPF_firmware_info,
			.interface_version = XENPF_INTERFACE_VERSION,
			.u.firmware_info.type = XEN_FW_KBD_SHIFT_FLAGS,
		};

		xen_init_vga(info, xen_start_info->console.dom0.info_size);
		xen_start_info->console.domU.mfn = 0;
		xen_start_info->console.domU.evtchn = 0;

		if (HYPERVISOR_dom0_op(&op) == 0)
			boot_params.kbd_status = op.u.firmware_info.u.kbd_shift_flags;

		/* Make sure ACS will be enabled */
		pci_request_acs();

		xen_acpi_sleep_register();

		/* Avoid searching for BIOS MP tables */
		x86_init.mpparse.find_smp_config = x86_init_noop;
		x86_init.mpparse.get_smp_config = x86_init_uint_noop;

		xen_boot_params_init_edd();
	}
#ifdef CONFIG_PCI
	/* PCI BIOS service won't work from a PV guest. */
	pci_probe &= ~PCI_PROBE_BIOS;
#endif
	xen_raw_console_write("about to get started...\n");

	xen_setup_runstate_info(0);

	xen_efi_init();

	/* Start the world */
#ifdef CONFIG_X86_32
	i386_start_kernel();
#else
	cr4_init_shadow(); /* 32b kernel does this in i386_start_kernel() */
	x86_64_start_reservations((char *)__pa_symbol(&boot_params));
#endif
}

void __ref xen_hvm_init_shared_info(void)
{
	int cpu;
	struct xen_add_to_physmap xatp;
	static struct shared_info *shared_info_page = 0;

	if (!shared_info_page)
		shared_info_page = (struct shared_info *)
			extend_brk(PAGE_SIZE, PAGE_SIZE);
	xatp.domid = DOMID_SELF;
	xatp.idx = 0;
	xatp.space = XENMAPSPACE_shared_info;
	xatp.gpfn = __pa(shared_info_page) >> PAGE_SHIFT;
	if (HYPERVISOR_memory_op(XENMEM_add_to_physmap, &xatp))
		BUG();

	HYPERVISOR_shared_info = (struct shared_info *)shared_info_page;

	/* xen_vcpu is a pointer to the vcpu_info struct in the shared_info
	 * page, we use it in the event channel upcall and in some pvclock
	 * related functions. We don't need the vcpu_info placement
	 * optimizations because we don't use any pv_mmu or pv_irq op on
	 * HVM.
	 * When xen_hvm_init_shared_info is run at boot time only vcpu 0 is
	 * online but xen_hvm_init_shared_info is run at resume time too and
	 * in that case multiple vcpus might be online. */
	for_each_online_cpu(cpu) {
		/* Leave it to be NULL. */
		if (cpu >= MAX_VIRT_CPUS)
			continue;
		per_cpu(xen_vcpu, cpu) = &HYPERVISOR_shared_info->vcpu_info[cpu];
	}
}

#ifdef CONFIG_XEN_PVHVM
static void __init init_hvm_pv_info(void)
{
	int major, minor;
	uint32_t eax, ebx, ecx, edx, pages, msr, base;
	u64 pfn;

	base = xen_cpuid_base();
	cpuid(base + 1, &eax, &ebx, &ecx, &edx);

	major = eax >> 16;
	minor = eax & 0xffff;
	printk(KERN_INFO "Xen version %d.%d.\n", major, minor);

	cpuid(base + 2, &pages, &msr, &ecx, &edx);

	pfn = __pa(hypercall_page);
	wrmsr_safe(msr, (u32)pfn, (u32)(pfn >> 32));

	xen_setup_features();

	pv_info.name = "Xen HVM";

	xen_domain_type = XEN_HVM_DOMAIN;
}

static int xen_hvm_cpu_notify(struct notifier_block *self, unsigned long action,
			      void *hcpu)
{
	int cpu = (long)hcpu;
	switch (action) {
	case CPU_UP_PREPARE:
		xen_vcpu_setup(cpu);
		if (xen_have_vector_callback) {
			if (xen_feature(XENFEAT_hvm_safe_pvclock))
				xen_setup_timer(cpu);
		}
		break;
	default:
		break;
	}
	return NOTIFY_OK;
}

static struct notifier_block xen_hvm_cpu_notifier = {
	.notifier_call	= xen_hvm_cpu_notify,
};

#ifdef CONFIG_KEXEC_CORE
static void xen_hvm_shutdown(void)
{
	native_machine_shutdown();
	if (kexec_in_progress)
		xen_reboot(SHUTDOWN_soft_reset);
}

static void xen_hvm_crash_shutdown(struct pt_regs *regs)
{
	native_machine_crash_shutdown(regs);
	xen_reboot(SHUTDOWN_soft_reset);
}
#endif

static void __init xen_hvm_guest_init(void)
{
	if (xen_pv_domain())
		return;

	init_hvm_pv_info();

	xen_hvm_init_shared_info();

	xen_panic_handler_init();

	if (xen_feature(XENFEAT_hvm_callback_vector))
		xen_have_vector_callback = 1;
	xen_hvm_smp_init();
	register_cpu_notifier(&xen_hvm_cpu_notifier);
	xen_unplug_emulated_devices();
	x86_init.irqs.intr_init = xen_init_IRQ;
	xen_hvm_init_time_ops();
	xen_hvm_init_mmu_ops();
#ifdef CONFIG_KEXEC_CORE
	machine_ops.shutdown = xen_hvm_shutdown;
	machine_ops.crash_shutdown = xen_hvm_crash_shutdown;
#endif
}
#endif

static bool xen_nopv = false;
static __init int xen_parse_nopv(char *arg)
{
       xen_nopv = true;
       return 0;
}
early_param("xen_nopv", xen_parse_nopv);

static uint32_t __init xen_platform(void)
{
	if (xen_nopv)
		return 0;

	return xen_cpuid_base();
}

bool xen_hvm_need_lapic(void)
{
	if (xen_nopv)
		return false;
	if (xen_pv_domain())
		return false;
	if (!xen_hvm_domain())
		return false;
	if (xen_feature(XENFEAT_hvm_pirqs) && xen_have_vector_callback)
		return false;
	return true;
}
EXPORT_SYMBOL_GPL(xen_hvm_need_lapic);

const struct hypervisor_x86 x86_hyper_xen = {
	.name			= "Xen",
	.detect			= xen_platform,
#ifdef CONFIG_XEN_PVHVM
	.init_platform		= xen_hvm_guest_init,
#endif
	.x2apic_available	= xen_x2apic_para_available,
};
EXPORT_SYMBOL(x86_hyper_xen);

#ifdef CONFIG_HOTPLUG_CPU
void xen_arch_register_cpu(int num)
{
	arch_register_cpu(num);
}
EXPORT_SYMBOL(xen_arch_register_cpu);

void xen_arch_unregister_cpu(int num)
{
	arch_unregister_cpu(num);
}
EXPORT_SYMBOL(xen_arch_unregister_cpu);
#endif
=======
#endif /* CONFIG_XEN_BALLOON_MEMORY_HOTPLUG */
>>>>>>> temp
