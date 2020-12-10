// SPDX-License-Identifier: GPL-2.0
/*
 *    Time of day based timer functions.
 *
 *  S390 version
 *    Copyright IBM Corp. 1999, 2008
 *    Author(s): Hartmut Penner (hp@de.ibm.com),
 *               Martin Schwidefsky (schwidefsky@de.ibm.com),
 *               Denis Joseph Barrow (djbarrow@de.ibm.com,barrow_dj@yahoo.com)
 *
 *  Derived from "arch/i386/kernel/time.c"
 *    Copyright (C) 1991, 1992, 1995  Linus Torvalds
 */

#define KMSG_COMPONENT "time"
#define pr_fmt(fmt) KMSG_COMPONENT ": " fmt

#include <linux/kernel_stat.h>
#include <linux/errno.h>
#include <linux/export.h>
#include <linux/sched.h>
#include <linux/sched/clock.h>
#include <linux/kernel.h>
#include <linux/param.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/cpu.h>
#include <linux/stop_machine.h>
#include <linux/time.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/smp.h>
#include <linux/types.h>
#include <linux/profile.h>
#include <linux/timex.h>
#include <linux/notifier.h>
#include <linux/timekeeper_internal.h>
#include <linux/clockchips.h>
#include <linux/gfp.h>
#include <linux/kprobes.h>
<<<<<<< HEAD
#include <asm/uaccess.h>
=======
#include <linux/uaccess.h>
>>>>>>> temp
#include <asm/facility.h>
#include <asm/delay.h>
#include <asm/div64.h>
#include <asm/vdso.h>
#include <asm/irq.h>
#include <asm/irq_regs.h>
#include <asm/vtimer.h>
#include <asm/stp.h>
#include <asm/cio.h>
#include "entry.h"

unsigned char tod_clock_base[16] __aligned(8) = {
	/* Force to data section. */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};
EXPORT_SYMBOL_GPL(tod_clock_base);

u64 clock_comparator_max = -1ULL;
EXPORT_SYMBOL_GPL(clock_comparator_max);

static DEFINE_PER_CPU(struct clock_event_device, comparators);

ATOMIC_NOTIFIER_HEAD(s390_epoch_delta_notifier);
EXPORT_SYMBOL(s390_epoch_delta_notifier);

unsigned char ptff_function_mask[16];
<<<<<<< HEAD
unsigned long lpar_offset;
=======

static unsigned long long lpar_offset;
static unsigned long long initial_leap_seconds;
static unsigned long long tod_steering_end;
static long long tod_steering_delta;
>>>>>>> temp

/*
 * Get time offsets with PTFF
 */
<<<<<<< HEAD
void __init ptff_init(void)
{
	struct ptff_qto qto;

	if (!test_facility(28))
		return;
=======
void __init time_early_init(void)
{
	struct ptff_qto qto;
	struct ptff_qui qui;

	/* Initialize TOD steering parameters */
	tod_steering_end = *(unsigned long long *) &tod_clock_base[1];
	vdso_data->ts_end = tod_steering_end;

	if (!test_facility(28))
		return;

>>>>>>> temp
	ptff(&ptff_function_mask, sizeof(ptff_function_mask), PTFF_QAF);

	/* get LPAR offset */
	if (ptff_query(PTFF_QTO) && ptff(&qto, sizeof(qto), PTFF_QTO) == 0)
		lpar_offset = qto.tod_epoch_difference;
<<<<<<< HEAD
=======

	/* get initial leap seconds */
	if (ptff_query(PTFF_QUI) && ptff(&qui, sizeof(qui), PTFF_QUI) == 0)
		initial_leap_seconds = (unsigned long long)
			((long) qui.old_leap * 4096000000L);
>>>>>>> temp
}

/*
 * Scheduler clock - returns current time in nanosec units.
 */
unsigned long long notrace sched_clock(void)
{
	return tod_to_ns(get_tod_clock_monotonic());
}
NOKPROBE_SYMBOL(sched_clock);

/*
 * Monotonic_clock - returns # of nanoseconds passed since time_init()
 */
unsigned long long monotonic_clock(void)
{
	return sched_clock();
}
EXPORT_SYMBOL(monotonic_clock);

static void ext_to_timespec64(unsigned char *clk, struct timespec64 *xt)
{
	unsigned long long high, low, rem, sec, nsec;

	/* Split extendnd TOD clock to micro-seconds and sub-micro-seconds */
	high = (*(unsigned long long *) clk) >> 4;
	low = (*(unsigned long long *)&clk[7]) << 4;
	/* Calculate seconds and nano-seconds */
	sec = high;
	rem = do_div(sec, 1000000);
	nsec = (((low >> 32) + (rem << 32)) * 1000) >> 32;

	xt->tv_sec = sec;
	xt->tv_nsec = nsec;
}

void clock_comparator_work(void)
{
	struct clock_event_device *cd;

	S390_lowcore.clock_comparator = clock_comparator_max;
	cd = this_cpu_ptr(&comparators);
	cd->event_handler(cd);
}

static int s390_next_event(unsigned long delta,
			   struct clock_event_device *evt)
{
	S390_lowcore.clock_comparator = get_tod_clock() + delta;
	set_clock_comparator(S390_lowcore.clock_comparator);
	return 0;
}

/*
 * Set up lowcore and control register of the current cpu to
 * enable TOD clock and clock comparator interrupts.
 */
void init_cpu_timer(void)
{
	struct clock_event_device *cd;
	int cpu;

	S390_lowcore.clock_comparator = clock_comparator_max;
	set_clock_comparator(S390_lowcore.clock_comparator);

	cpu = smp_processor_id();
	cd = &per_cpu(comparators, cpu);
	cd->name		= "comparator";
	cd->features		= CLOCK_EVT_FEAT_ONESHOT;
	cd->mult		= 16777;
	cd->shift		= 12;
	cd->min_delta_ns	= 1;
	cd->min_delta_ticks	= 1;
	cd->max_delta_ns	= LONG_MAX;
	cd->max_delta_ticks	= ULONG_MAX;
	cd->rating		= 400;
	cd->cpumask		= cpumask_of(cpu);
	cd->set_next_event	= s390_next_event;

	clockevents_register_device(cd);

	/* Enable clock comparator timer interrupt. */
	__ctl_set_bit(0,11);

	/* Always allow the timing alert external interrupt. */
	__ctl_set_bit(0, 4);
}

static void clock_comparator_interrupt(struct ext_code ext_code,
				       unsigned int param32,
				       unsigned long param64)
{
	inc_irq_stat(IRQEXT_CLK);
	if (S390_lowcore.clock_comparator == clock_comparator_max)
		set_clock_comparator(S390_lowcore.clock_comparator);
}

static void stp_timing_alert(struct stp_irq_parm *);

static void timing_alert_interrupt(struct ext_code ext_code,
				   unsigned int param32, unsigned long param64)
{
	inc_irq_stat(IRQEXT_TLA);
	if (param32 & 0x00038000)
		stp_timing_alert((struct stp_irq_parm *) &param32);
}

static void stp_reset(void);

void read_persistent_clock64(struct timespec64 *ts)
{
	unsigned char clk[STORE_CLOCK_EXT_SIZE];
	__u64 delta;

	delta = initial_leap_seconds + TOD_UNIX_EPOCH;
	get_tod_clock_ext(clk);
	*(__u64 *) &clk[1] -= delta;
	if (*(__u64 *) &clk[1] > delta)
		clk[0]--;
	ext_to_timespec64(clk, ts);
}

void read_boot_clock64(struct timespec64 *ts)
{
	unsigned char clk[STORE_CLOCK_EXT_SIZE];
	__u64 delta;

	delta = initial_leap_seconds + TOD_UNIX_EPOCH;
	memcpy(clk, tod_clock_base, 16);
	*(__u64 *) &clk[1] -= delta;
	if (*(__u64 *) &clk[1] > delta)
		clk[0]--;
	ext_to_timespec64(clk, ts);
}

static u64 read_tod_clock(struct clocksource *cs)
{
	unsigned long long now, adj;

	preempt_disable(); /* protect from changes to steering parameters */
	now = get_tod_clock();
	adj = tod_steering_end - now;
	if (unlikely((s64) adj >= 0))
		/*
		 * manually steer by 1 cycle every 2^16 cycles. This
		 * corresponds to shifting the tod delta by 15. 1s is
		 * therefore steered in ~9h. The adjust will decrease
		 * over time, until it finally reaches 0.
		 */
		now += (tod_steering_delta < 0) ? (adj >> 15) : -(adj >> 15);
	preempt_enable();
	return now;
}

static struct clocksource clocksource_tod = {
	.name		= "tod",
	.rating		= 400,
	.read		= read_tod_clock,
	.mask		= -1ULL,
	.mult		= 1000,
	.shift		= 12,
	.flags		= CLOCK_SOURCE_IS_CONTINUOUS,
};

struct clocksource * __init clocksource_default_clock(void)
{
	return &clocksource_tod;
}

void update_vsyscall(struct timekeeper *tk)
{
	u64 nsecps;

	if (tk->tkr_mono.clock != &clocksource_tod)
		return;

	/* Make userspace gettimeofday spin until we're done. */
	++vdso_data->tb_update_count;
	smp_wmb();
	vdso_data->xtime_tod_stamp = tk->tkr_mono.cycle_last;
	vdso_data->xtime_clock_sec = tk->xtime_sec;
	vdso_data->xtime_clock_nsec = tk->tkr_mono.xtime_nsec;
	vdso_data->wtom_clock_sec =
		tk->xtime_sec + tk->wall_to_monotonic.tv_sec;
	vdso_data->wtom_clock_nsec = tk->tkr_mono.xtime_nsec +
		+ ((u64) tk->wall_to_monotonic.tv_nsec << tk->tkr_mono.shift);
	nsecps = (u64) NSEC_PER_SEC << tk->tkr_mono.shift;
	while (vdso_data->wtom_clock_nsec >= nsecps) {
		vdso_data->wtom_clock_nsec -= nsecps;
		vdso_data->wtom_clock_sec++;
	}

	vdso_data->xtime_coarse_sec = tk->xtime_sec;
	vdso_data->xtime_coarse_nsec =
		(long)(tk->tkr_mono.xtime_nsec >> tk->tkr_mono.shift);
	vdso_data->wtom_coarse_sec =
		vdso_data->xtime_coarse_sec + tk->wall_to_monotonic.tv_sec;
	vdso_data->wtom_coarse_nsec =
		vdso_data->xtime_coarse_nsec + tk->wall_to_monotonic.tv_nsec;
	while (vdso_data->wtom_coarse_nsec >= NSEC_PER_SEC) {
		vdso_data->wtom_coarse_nsec -= NSEC_PER_SEC;
		vdso_data->wtom_coarse_sec++;
	}

	vdso_data->tk_mult = tk->tkr_mono.mult;
	vdso_data->tk_shift = tk->tkr_mono.shift;
	smp_wmb();
	++vdso_data->tb_update_count;
}

extern struct timezone sys_tz;

void update_vsyscall_tz(void)
{
	vdso_data->tz_minuteswest = sys_tz.tz_minuteswest;
	vdso_data->tz_dsttime = sys_tz.tz_dsttime;
}

/*
 * Initialize the TOD clock and the CPU timer of
 * the boot cpu.
 */
void __init time_init(void)
{
	/* Reset time synchronization interfaces. */
	stp_reset();

	/* request the clock comparator external interrupt */
	if (register_external_irq(EXT_IRQ_CLK_COMP, clock_comparator_interrupt))
		panic("Couldn't request external interrupt 0x1004");

	/* request the timing alert external interrupt */
	if (register_external_irq(EXT_IRQ_TIMING_ALERT, timing_alert_interrupt))
		panic("Couldn't request external interrupt 0x1406");

	if (__clocksource_register(&clocksource_tod) != 0)
		panic("Could not register TOD clock source");

	/* Enable TOD clock interrupts on the boot cpu. */
	init_cpu_timer();

	/* Enable cpu timer interrupts on the boot cpu. */
	vtime_init();
}

static DEFINE_PER_CPU(atomic_t, clock_sync_word);
static DEFINE_MUTEX(clock_sync_mutex);
static unsigned long clock_sync_flags;

#define CLOCK_SYNC_HAS_STP	0
#define CLOCK_SYNC_STP		1

/*
 * The get_clock function for the physical clock. It will get the current
 * TOD clock, subtract the LPAR offset and write the result to *clock.
 * The function returns 0 if the clock is in sync with the external time
 * source. If the clock mode is local it will return -EOPNOTSUPP and
 * -EAGAIN if the clock is not in sync with the external reference.
 */
<<<<<<< HEAD
int get_phys_clock(unsigned long long *clock)
=======
int get_phys_clock(unsigned long *clock)
>>>>>>> temp
{
	atomic_t *sw_ptr;
	unsigned int sw0, sw1;

	sw_ptr = &get_cpu_var(clock_sync_word);
	sw0 = atomic_read(sw_ptr);
	*clock = get_tod_clock() - lpar_offset;
	sw1 = atomic_read(sw_ptr);
	put_cpu_var(clock_sync_word);
	if (sw0 == sw1 && (sw0 & 0x80000000U))
		/* Success: time is in sync. */
		return 0;
	if (!test_bit(CLOCK_SYNC_HAS_STP, &clock_sync_flags))
		return -EOPNOTSUPP;
	if (!test_bit(CLOCK_SYNC_STP, &clock_sync_flags))
		return -EACCES;
	return -EAGAIN;
}
EXPORT_SYMBOL(get_phys_clock);

/*
 * Make get_phys_clock() return -EAGAIN.
 */
static void disable_sync_clock(void *dummy)
{
	atomic_t *sw_ptr = this_cpu_ptr(&clock_sync_word);
	/*
	 * Clear the in-sync bit 2^31. All get_phys_clock calls will
	 * fail until the sync bit is turned back on. In addition
	 * increase the "sequence" counter to avoid the race of an
	 * stp event and the complete recovery against get_phys_clock.
	 */
	atomic_andnot(0x80000000, sw_ptr);
	atomic_inc(sw_ptr);
}

/*
 * Make get_phys_clock() return 0 again.
 * Needs to be called from a context disabled for preemption.
 */
static void enable_sync_clock(void)
{
	atomic_t *sw_ptr = this_cpu_ptr(&clock_sync_word);
	atomic_or(0x80000000, sw_ptr);
}

/*
 * Function to check if the clock is in sync.
 */
static inline int check_sync_clock(void)
{
	atomic_t *sw_ptr;
	int rc;

	sw_ptr = &get_cpu_var(clock_sync_word);
	rc = (atomic_read(sw_ptr) & 0x80000000U) != 0;
	put_cpu_var(clock_sync_word);
	return rc;
}

/*
 * Apply clock delta to the global data structures.
 * This is called once on the CPU that performed the clock sync.
 */
static void clock_sync_global(unsigned long long delta)
{
	unsigned long now, adj;
	struct ptff_qto qto;

	/* Fixup the monotonic sched clock. */
	*(unsigned long long *) &tod_clock_base[1] += delta;
	if (*(unsigned long long *) &tod_clock_base[1] < delta)
		/* Epoch overflow */
		tod_clock_base[0]++;
	/* Adjust TOD steering parameters. */
	vdso_data->tb_update_count++;
	now = get_tod_clock();
	adj = tod_steering_end - now;
	if (unlikely((s64) adj >= 0))
		/* Calculate how much of the old adjustment is left. */
		tod_steering_delta = (tod_steering_delta < 0) ?
			-(adj >> 15) : (adj >> 15);
	tod_steering_delta += delta;
	if ((abs(tod_steering_delta) >> 48) != 0)
		panic("TOD clock sync offset %lli is too large to drift\n",
		      tod_steering_delta);
	tod_steering_end = now + (abs(tod_steering_delta) << 15);
	vdso_data->ts_dir = (tod_steering_delta < 0) ? 0 : 1;
	vdso_data->ts_end = tod_steering_end;
	vdso_data->tb_update_count++;
	/* Update LPAR offset. */
	if (ptff_query(PTFF_QTO) && ptff(&qto, sizeof(qto), PTFF_QTO) == 0)
		lpar_offset = qto.tod_epoch_difference;
	/* Call the TOD clock change notifier. */
	atomic_notifier_call_chain(&s390_epoch_delta_notifier, 0, &delta);
}

/*
 * Apply clock delta to the per-CPU data structures of this CPU.
 * This is called for each online CPU after the call to clock_sync_global.
 */
static void clock_sync_local(unsigned long long delta)
{
	/* Add the delta to the clock comparator. */
	if (S390_lowcore.clock_comparator != clock_comparator_max) {
		S390_lowcore.clock_comparator += delta;
		set_clock_comparator(S390_lowcore.clock_comparator);
	}
	/* Adjust the last_update_clock time-stamp. */
	S390_lowcore.last_update_clock += delta;
}

/* Single threaded workqueue used for stp sync events */
static struct workqueue_struct *time_sync_wq;

static void __init time_init_wq(void)
{
	if (time_sync_wq)
		return;
	time_sync_wq = create_singlethread_workqueue("timesync");
}

struct clock_sync_data {
	atomic_t cpus;
	int in_sync;
<<<<<<< HEAD
	unsigned long long fixup_cc;
	int etr_port;
	struct etr_aib *etr_aib;
};

static void clock_sync_cpu(struct clock_sync_data *sync)
{
	atomic_dec(&sync->cpus);
	enable_sync_clock();
	/*
	 * This looks like a busy wait loop but it isn't. etr_sync_cpus
	 * is called on all other cpus while the TOD clocks is stopped.
	 * __udelay will stop the cpu on an enabled wait psw until the
	 * TOD is running again.
	 */
	while (sync->in_sync == 0) {
		__udelay(1);
		/*
		 * A different cpu changes *in_sync. Therefore use
		 * barrier() to force memory access.
		 */
		barrier();
	}
	if (sync->in_sync != 1)
		/* Didn't work. Clear per-cpu in sync bit again. */
		disable_sync_clock(NULL);
	/*
	 * This round of TOD syncing is done. Set the clock comparator
	 * to the next tick and let the processor continue.
	 */
	fixup_clock_comparator(sync->fixup_cc);
}

/*
 * Sync the TOD clock using the port referred to by aibp. This port
 * has to be enabled and the other port has to be disabled. The
 * last eacr update has to be more than 1.6 seconds in the past.
 */
static int etr_sync_clock(void *data)
{
	static int first;
	unsigned long long clock, old_clock, clock_delta, delay, delta;
	struct clock_sync_data *etr_sync;
	struct etr_aib *sync_port, *aib;
	struct ptff_qto qto;
	int port;
	int rc;

	etr_sync = data;

	if (xchg(&first, 1) == 1) {
		/* Slave */
		clock_sync_cpu(etr_sync);
		return 0;
	}

	/* Wait until all other cpus entered the sync function. */
	while (atomic_read(&etr_sync->cpus) != 0)
		cpu_relax();

	port = etr_sync->etr_port;
	aib = etr_sync->etr_aib;
	sync_port = (port == 0) ? &etr_port0 : &etr_port1;
	enable_sync_clock();

	/* Set clock to next OTE. */
	__ctl_set_bit(14, 21);
	__ctl_set_bit(0, 29);
	clock = ((unsigned long long) (aib->edf2.etv + 1)) << 32;
	old_clock = get_tod_clock();
	if (set_tod_clock(clock) == 0) {
		__udelay(1);	/* Wait for the clock to start. */
		__ctl_clear_bit(0, 29);
		__ctl_clear_bit(14, 21);
		etr_stetr(aib);
		/* Adjust Linux timing variables. */
		delay = (unsigned long long)
			(aib->edf2.etv - sync_port->edf2.etv) << 32;
		delta = adjust_time(old_clock, clock, delay);
		clock_delta = clock - old_clock;
		atomic_notifier_call_chain(&s390_epoch_delta_notifier, 0,
					   &clock_delta);
		etr_sync->fixup_cc = delta;
		fixup_clock_comparator(delta);
		/* Verify that the clock is properly set. */
		if (!etr_aib_follows(sync_port, aib, port)) {
			/* Didn't work. */
			disable_sync_clock(NULL);
			etr_sync->in_sync = -EAGAIN;
			rc = -EAGAIN;
		} else {
			if (ptff_query(PTFF_QTO) &&
			    ptff(&qto, sizeof(qto), PTFF_QTO) == 0)
				/* Update LPAR offset */
				lpar_offset = qto.tod_epoch_difference;
			etr_sync->in_sync = 1;
			rc = 0;
		}
	} else {
		/* Could not set the clock ?!? */
		__ctl_clear_bit(0, 29);
		__ctl_clear_bit(14, 21);
		disable_sync_clock(NULL);
		etr_sync->in_sync = -EAGAIN;
		rc = -EAGAIN;
	}
	xchg(&first, 0);
	return rc;
}

static int etr_sync_clock_stop(struct etr_aib *aib, int port)
{
	struct clock_sync_data etr_sync;
	struct etr_aib *sync_port;
	int follows;
	int rc;

	/* Check if the current aib is adjacent to the sync port aib. */
	sync_port = (port == 0) ? &etr_port0 : &etr_port1;
	follows = etr_aib_follows(sync_port, aib, port);
	memcpy(sync_port, aib, sizeof(*aib));
	if (!follows)
		return -EAGAIN;
	memset(&etr_sync, 0, sizeof(etr_sync));
	etr_sync.etr_aib = aib;
	etr_sync.etr_port = port;
	get_online_cpus();
	atomic_set(&etr_sync.cpus, num_online_cpus() - 1);
	rc = stop_machine(etr_sync_clock, &etr_sync, cpu_online_mask);
	put_online_cpus();
	return rc;
}

/*
 * Handle the immediate effects of the different events.
 * The port change event is used for online/offline changes.
 */
static struct etr_eacr etr_handle_events(struct etr_eacr eacr)
{
	if (test_and_clear_bit(ETR_EVENT_SYNC_CHECK, &etr_events))
		eacr.es = 0;
	if (test_and_clear_bit(ETR_EVENT_SWITCH_LOCAL, &etr_events))
		eacr.es = eacr.sl = 0;
	if (test_and_clear_bit(ETR_EVENT_PORT_ALERT, &etr_events))
		etr_port0_uptodate = etr_port1_uptodate = 0;

	if (test_and_clear_bit(ETR_EVENT_PORT0_CHANGE, &etr_events)) {
		if (eacr.e0)
			/*
			 * Port change of an enabled port. We have to
			 * assume that this can have caused an stepping
			 * port switch.
			 */
			etr_tolec = get_tod_clock();
		eacr.p0 = etr_port0_online;
		if (!eacr.p0)
			eacr.e0 = 0;
		etr_port0_uptodate = 0;
	}
	if (test_and_clear_bit(ETR_EVENT_PORT1_CHANGE, &etr_events)) {
		if (eacr.e1)
			/*
			 * Port change of an enabled port. We have to
			 * assume that this can have caused an stepping
			 * port switch.
			 */
			etr_tolec = get_tod_clock();
		eacr.p1 = etr_port1_online;
		if (!eacr.p1)
			eacr.e1 = 0;
		etr_port1_uptodate = 0;
	}
	clear_bit(ETR_EVENT_UPDATE, &etr_events);
	return eacr;
}

/*
 * Set up a timer that expires after the etr_tolec + 1.6 seconds if
 * one of the ports needs an update.
 */
static void etr_set_tolec_timeout(unsigned long long now)
{
	unsigned long micros;

	if ((!etr_eacr.p0 || etr_port0_uptodate) &&
	    (!etr_eacr.p1 || etr_port1_uptodate))
		return;
	micros = (now > etr_tolec) ? ((now - etr_tolec) >> 12) : 0;
	micros = (micros > 1600000) ? 0 : 1600000 - micros;
	mod_timer(&etr_timer, jiffies + (micros * HZ) / 1000000 + 1);
}

/*
 * Set up a time that expires after 1/2 second.
 */
static void etr_set_sync_timeout(void)
{
	mod_timer(&etr_timer, jiffies + HZ/2);
}

/*
 * Update the aib information for one or both ports.
 */
static struct etr_eacr etr_handle_update(struct etr_aib *aib,
					 struct etr_eacr eacr)
{
	/* With both ports disabled the aib information is useless. */
	if (!eacr.e0 && !eacr.e1)
		return eacr;

	/* Update port0 or port1 with aib stored in etr_work_fn. */
	if (aib->esw.q == 0) {
		/* Information for port 0 stored. */
		if (eacr.p0 && !etr_port0_uptodate) {
			etr_port0 = *aib;
			if (etr_port0_online)
				etr_port0_uptodate = 1;
		}
	} else {
		/* Information for port 1 stored. */
		if (eacr.p1 && !etr_port1_uptodate) {
			etr_port1 = *aib;
			if (etr_port0_online)
				etr_port1_uptodate = 1;
		}
	}

	/*
	 * Do not try to get the alternate port aib if the clock
	 * is not in sync yet.
	 */
	if (!eacr.es || !check_sync_clock())
		return eacr;

	/*
	 * If steai is available we can get the information about
	 * the other port immediately. If only stetr is available the
	 * data-port bit toggle has to be used.
	 */
	if (etr_steai_available) {
		if (eacr.p0 && !etr_port0_uptodate) {
			etr_steai_cv(&etr_port0, ETR_STEAI_PORT_0);
			etr_port0_uptodate = 1;
		}
		if (eacr.p1 && !etr_port1_uptodate) {
			etr_steai_cv(&etr_port1, ETR_STEAI_PORT_1);
			etr_port1_uptodate = 1;
		}
	} else {
		/*
		 * One port was updated above, if the other
		 * port is not uptodate toggle dp bit.
		 */
		if ((eacr.p0 && !etr_port0_uptodate) ||
		    (eacr.p1 && !etr_port1_uptodate))
			eacr.dp ^= 1;
		else
			eacr.dp = 0;
	}
	return eacr;
}

/*
 * Write new etr control register if it differs from the current one.
 * Return 1 if etr_tolec has been updated as well.
 */
static void etr_update_eacr(struct etr_eacr eacr)
{
	int dp_changed;

	if (memcmp(&etr_eacr, &eacr, sizeof(eacr)) == 0)
		/* No change, return. */
		return;
	/*
	 * The disable of an active port of the change of the data port
	 * bit can/will cause a change in the data port.
	 */
	dp_changed = etr_eacr.e0 > eacr.e0 || etr_eacr.e1 > eacr.e1 ||
		(etr_eacr.dp ^ eacr.dp) != 0;
	etr_eacr = eacr;
	etr_setr(&etr_eacr);
	if (dp_changed)
		etr_tolec = get_tod_clock();
}

/*
 * ETR work. In this function you'll find the main logic. In
 * particular this is the only function that calls etr_update_eacr(),
 * it "controls" the etr control register.
 */
static void etr_work_fn(struct work_struct *work)
{
	unsigned long long now;
	struct etr_eacr eacr;
	struct etr_aib aib;
	int sync_port;

	/* prevent multiple execution. */
	mutex_lock(&etr_work_mutex);

	/* Create working copy of etr_eacr. */
	eacr = etr_eacr;

	/* Check for the different events and their immediate effects. */
	eacr = etr_handle_events(eacr);

	/* Check if ETR is supposed to be active. */
	eacr.ea = eacr.p0 || eacr.p1;
	if (!eacr.ea) {
		/* Both ports offline. Reset everything. */
		eacr.dp = eacr.es = eacr.sl = 0;
		on_each_cpu(disable_sync_clock, NULL, 1);
		del_timer_sync(&etr_timer);
		etr_update_eacr(eacr);
		goto out_unlock;
	}

	/* Store aib to get the current ETR status word. */
	BUG_ON(etr_stetr(&aib) != 0);
	etr_port0.esw = etr_port1.esw = aib.esw;	/* Copy status word. */
	now = get_tod_clock();

	/*
	 * Update the port information if the last stepping port change
	 * or data port change is older than 1.6 seconds.
	 */
	if (now >= etr_tolec + (1600000 << 12))
		eacr = etr_handle_update(&aib, eacr);

	/*
	 * Select ports to enable. The preferred synchronization mode is PPS.
	 * If a port can be enabled depends on a number of things:
	 * 1) The port needs to be online and uptodate. A port is not
	 *    disabled just because it is not uptodate, but it is only
	 *    enabled if it is uptodate.
	 * 2) The port needs to have the same mode (pps / etr).
	 * 3) The port needs to be usable -> etr_port_valid() == 1
	 * 4) To enable the second port the clock needs to be in sync.
	 * 5) If both ports are useable and are ETR ports, the network id
	 *    has to be the same.
	 * The eacr.sl bit is used to indicate etr mode vs. pps mode.
	 */
	if (eacr.p0 && aib.esw.psc0 == etr_lpsc_pps_mode) {
		eacr.sl = 0;
		eacr.e0 = 1;
		if (!etr_mode_is_pps(etr_eacr))
			eacr.es = 0;
		if (!eacr.es || !eacr.p1 || aib.esw.psc1 != etr_lpsc_pps_mode)
			eacr.e1 = 0;
		// FIXME: uptodate checks ?
		else if (etr_port0_uptodate && etr_port1_uptodate)
			eacr.e1 = 1;
		sync_port = (etr_port0_uptodate &&
			     etr_port_valid(&etr_port0, 0)) ? 0 : -1;
	} else if (eacr.p1 && aib.esw.psc1 == etr_lpsc_pps_mode) {
		eacr.sl = 0;
		eacr.e0 = 0;
		eacr.e1 = 1;
		if (!etr_mode_is_pps(etr_eacr))
			eacr.es = 0;
		sync_port = (etr_port1_uptodate &&
			     etr_port_valid(&etr_port1, 1)) ? 1 : -1;
	} else if (eacr.p0 && aib.esw.psc0 == etr_lpsc_operational_step) {
		eacr.sl = 1;
		eacr.e0 = 1;
		if (!etr_mode_is_etr(etr_eacr))
			eacr.es = 0;
		if (!eacr.es || !eacr.p1 ||
		    aib.esw.psc1 != etr_lpsc_operational_alt)
			eacr.e1 = 0;
		else if (etr_port0_uptodate && etr_port1_uptodate &&
			 etr_compare_network(&etr_port0, &etr_port1))
			eacr.e1 = 1;
		sync_port = (etr_port0_uptodate &&
			     etr_port_valid(&etr_port0, 0)) ? 0 : -1;
	} else if (eacr.p1 && aib.esw.psc1 == etr_lpsc_operational_step) {
		eacr.sl = 1;
		eacr.e0 = 0;
		eacr.e1 = 1;
		if (!etr_mode_is_etr(etr_eacr))
			eacr.es = 0;
		sync_port = (etr_port1_uptodate &&
			     etr_port_valid(&etr_port1, 1)) ? 1 : -1;
	} else {
		/* Both ports not usable. */
		eacr.es = eacr.sl = 0;
		sync_port = -1;
	}

	/*
	 * If the clock is in sync just update the eacr and return.
	 * If there is no valid sync port wait for a port update.
	 */
	if ((eacr.es && check_sync_clock()) || sync_port < 0) {
		etr_update_eacr(eacr);
		etr_set_tolec_timeout(now);
		goto out_unlock;
	}

	/*
	 * Prepare control register for clock syncing
	 * (reset data port bit, set sync check control.
	 */
	eacr.dp = 0;
	eacr.es = 1;

	/*
	 * Update eacr and try to synchronize the clock. If the update
	 * of eacr caused a stepping port switch (or if we have to
	 * assume that a stepping port switch has occurred) or the
	 * clock syncing failed, reset the sync check control bit
	 * and set up a timer to try again after 0.5 seconds
	 */
	etr_update_eacr(eacr);
	if (now < etr_tolec + (1600000 << 12) ||
	    etr_sync_clock_stop(&aib, sync_port) != 0) {
		/* Sync failed. Try again in 1/2 second. */
		eacr.es = 0;
		etr_update_eacr(eacr);
		etr_set_sync_timeout();
	} else
		etr_set_tolec_timeout(now);
out_unlock:
	mutex_unlock(&etr_work_mutex);
}

/*
 * Sysfs interface functions
 */
static struct bus_type etr_subsys = {
	.name		= "etr",
	.dev_name	= "etr",
};

static struct device etr_port0_dev = {
	.id	= 0,
	.bus	= &etr_subsys,
};

static struct device etr_port1_dev = {
	.id	= 1,
	.bus	= &etr_subsys,
};

/*
 * ETR subsys attributes
 */
static ssize_t etr_stepping_port_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	return sprintf(buf, "%i\n", etr_port0.esw.p);
}

static DEVICE_ATTR(stepping_port, 0400, etr_stepping_port_show, NULL);

static ssize_t etr_stepping_mode_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	char *mode_str;

	if (etr_mode_is_pps(etr_eacr))
		mode_str = "pps";
	else if (etr_mode_is_etr(etr_eacr))
		mode_str = "etr";
	else
		mode_str = "local";
	return sprintf(buf, "%s\n", mode_str);
}

static DEVICE_ATTR(stepping_mode, 0400, etr_stepping_mode_show, NULL);

/*
 * ETR port attributes
 */
static inline struct etr_aib *etr_aib_from_dev(struct device *dev)
{
	if (dev == &etr_port0_dev)
		return etr_port0_online ? &etr_port0 : NULL;
	else
		return etr_port1_online ? &etr_port1 : NULL;
}

static ssize_t etr_online_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	unsigned int online;

	online = (dev == &etr_port0_dev) ? etr_port0_online : etr_port1_online;
	return sprintf(buf, "%i\n", online);
}

static ssize_t etr_online_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	unsigned int value;

	value = simple_strtoul(buf, NULL, 0);
	if (value != 0 && value != 1)
		return -EINVAL;
	if (!test_bit(CLOCK_SYNC_HAS_ETR, &clock_sync_flags))
		return -EOPNOTSUPP;
	mutex_lock(&clock_sync_mutex);
	if (dev == &etr_port0_dev) {
		if (etr_port0_online == value)
			goto out;	/* Nothing to do. */
		etr_port0_online = value;
		if (etr_port0_online && etr_port1_online)
			set_bit(CLOCK_SYNC_ETR, &clock_sync_flags);
		else
			clear_bit(CLOCK_SYNC_ETR, &clock_sync_flags);
		set_bit(ETR_EVENT_PORT0_CHANGE, &etr_events);
		queue_work(time_sync_wq, &etr_work);
	} else {
		if (etr_port1_online == value)
			goto out;	/* Nothing to do. */
		etr_port1_online = value;
		if (etr_port0_online && etr_port1_online)
			set_bit(CLOCK_SYNC_ETR, &clock_sync_flags);
		else
			clear_bit(CLOCK_SYNC_ETR, &clock_sync_flags);
		set_bit(ETR_EVENT_PORT1_CHANGE, &etr_events);
		queue_work(time_sync_wq, &etr_work);
	}
out:
	mutex_unlock(&clock_sync_mutex);
	return count;
}

static DEVICE_ATTR(online, 0600, etr_online_show, etr_online_store);

static ssize_t etr_stepping_control_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	return sprintf(buf, "%i\n", (dev == &etr_port0_dev) ?
		       etr_eacr.e0 : etr_eacr.e1);
}

static DEVICE_ATTR(stepping_control, 0400, etr_stepping_control_show, NULL);

static ssize_t etr_mode_code_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	if (!etr_port0_online && !etr_port1_online)
		/* Status word is not uptodate if both ports are offline. */
		return -ENODATA;
	return sprintf(buf, "%i\n", (dev == &etr_port0_dev) ?
		       etr_port0.esw.psc0 : etr_port0.esw.psc1);
}

static DEVICE_ATTR(state_code, 0400, etr_mode_code_show, NULL);

static ssize_t etr_untuned_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct etr_aib *aib = etr_aib_from_dev(dev);

	if (!aib || !aib->slsw.v1)
		return -ENODATA;
	return sprintf(buf, "%i\n", aib->edf1.u);
}

static DEVICE_ATTR(untuned, 0400, etr_untuned_show, NULL);

static ssize_t etr_network_id_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct etr_aib *aib = etr_aib_from_dev(dev);

	if (!aib || !aib->slsw.v1)
		return -ENODATA;
	return sprintf(buf, "%i\n", aib->edf1.net_id);
}

static DEVICE_ATTR(network, 0400, etr_network_id_show, NULL);

static ssize_t etr_id_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct etr_aib *aib = etr_aib_from_dev(dev);

	if (!aib || !aib->slsw.v1)
		return -ENODATA;
	return sprintf(buf, "%i\n", aib->edf1.etr_id);
}

static DEVICE_ATTR(id, 0400, etr_id_show, NULL);

static ssize_t etr_port_number_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct etr_aib *aib = etr_aib_from_dev(dev);

	if (!aib || !aib->slsw.v1)
		return -ENODATA;
	return sprintf(buf, "%i\n", aib->edf1.etr_pn);
}

static DEVICE_ATTR(port, 0400, etr_port_number_show, NULL);

static ssize_t etr_coupled_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct etr_aib *aib = etr_aib_from_dev(dev);

	if (!aib || !aib->slsw.v3)
		return -ENODATA;
	return sprintf(buf, "%i\n", aib->edf3.c);
}

static DEVICE_ATTR(coupled, 0400, etr_coupled_show, NULL);

static ssize_t etr_local_time_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct etr_aib *aib = etr_aib_from_dev(dev);

	if (!aib || !aib->slsw.v3)
		return -ENODATA;
	return sprintf(buf, "%i\n", aib->edf3.blto);
}

static DEVICE_ATTR(local_time, 0400, etr_local_time_show, NULL);

static ssize_t etr_utc_offset_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct etr_aib *aib = etr_aib_from_dev(dev);

	if (!aib || !aib->slsw.v3)
		return -ENODATA;
	return sprintf(buf, "%i\n", aib->edf3.buo);
}

static DEVICE_ATTR(utc_offset, 0400, etr_utc_offset_show, NULL);

static struct device_attribute *etr_port_attributes[] = {
	&dev_attr_online,
	&dev_attr_stepping_control,
	&dev_attr_state_code,
	&dev_attr_untuned,
	&dev_attr_network,
	&dev_attr_id,
	&dev_attr_port,
	&dev_attr_coupled,
	&dev_attr_local_time,
	&dev_attr_utc_offset,
	NULL
=======
	unsigned long long clock_delta;
>>>>>>> temp
};

/*
 * Server Time Protocol (STP) code.
 */
static bool stp_online;
static struct stp_sstpi stp_info;
static void *stp_page;

static void stp_work_fn(struct work_struct *work);
static DEFINE_MUTEX(stp_work_mutex);
static DECLARE_WORK(stp_work, stp_work_fn);
static struct timer_list stp_timer;

static int __init early_parse_stp(char *p)
{
	return kstrtobool(p, &stp_online);
}
early_param("stp", early_parse_stp);

/*
 * Reset STP attachment.
 */
static void __init stp_reset(void)
{
	int rc;

	stp_page = (void *) get_zeroed_page(GFP_ATOMIC);
	rc = chsc_sstpc(stp_page, STP_OP_CTRL, 0x0000, NULL);
	if (rc == 0)
		set_bit(CLOCK_SYNC_HAS_STP, &clock_sync_flags);
	else if (stp_online) {
		pr_warn("The real or virtual hardware system does not provide an STP interface\n");
		free_page((unsigned long) stp_page);
		stp_page = NULL;
		stp_online = false;
	}
}

static void stp_timeout(struct timer_list *unused)
{
	queue_work(time_sync_wq, &stp_work);
}

static int __init stp_init(void)
{
	if (!test_bit(CLOCK_SYNC_HAS_STP, &clock_sync_flags))
		return 0;
	timer_setup(&stp_timer, stp_timeout, 0);
	time_init_wq();
	if (!stp_online)
		return 0;
	queue_work(time_sync_wq, &stp_work);
	return 0;
}

arch_initcall(stp_init);

/*
 * STP timing alert. There are three causes:
 * 1) timing status change
 * 2) link availability change
 * 3) time control parameter change
 * In all three cases we are only interested in the clock source state.
 * If a STP clock source is now available use it.
 */
static void stp_timing_alert(struct stp_irq_parm *intparm)
{
	if (intparm->tsc || intparm->lac || intparm->tcpc)
		queue_work(time_sync_wq, &stp_work);
}

/*
 * STP sync check machine check. This is called when the timing state
 * changes from the synchronized state to the unsynchronized state.
 * After a STP sync check the clock is not in sync. The machine check
 * is broadcasted to all cpus at the same time.
 */
int stp_sync_check(void)
{
	disable_sync_clock(NULL);
	return 1;
}

/*
 * STP island condition machine check. This is called when an attached
 * server  attempts to communicate over an STP link and the servers
 * have matching CTN ids and have a valid stratum-1 configuration
 * but the configurations do not match.
 */
int stp_island_check(void)
{
	disable_sync_clock(NULL);
	return 1;
}

void stp_queue_work(void)
{
	queue_work(time_sync_wq, &stp_work);
}

static int stp_sync_clock(void *data)
{
	struct clock_sync_data *sync = data;
	unsigned long long clock_delta;
	static int first;
<<<<<<< HEAD
	unsigned long long old_clock, delta, new_clock, clock_delta;
	struct clock_sync_data *stp_sync;
	struct ptff_qto qto;
=======
>>>>>>> temp
	int rc;

	enable_sync_clock();
<<<<<<< HEAD

	rc = 0;
	if (stp_info.todoff[0] || stp_info.todoff[1] ||
	    stp_info.todoff[2] || stp_info.todoff[3] ||
	    stp_info.tmd != 2) {
		old_clock = get_tod_clock();
		rc = chsc_sstpc(stp_page, STP_OP_SYNC, 0, &clock_delta);
		if (rc == 0) {
			new_clock = old_clock + clock_delta;
			delta = adjust_time(old_clock, new_clock, 0);
			if (ptff_query(PTFF_QTO) &&
			    ptff(&qto, sizeof(qto), PTFF_QTO) == 0)
				/* Update LPAR offset */
				lpar_offset = qto.tod_epoch_difference;
			atomic_notifier_call_chain(&s390_epoch_delta_notifier,
						   0, &clock_delta);
			fixup_clock_comparator(delta);
			rc = chsc_sstpi(stp_page, &stp_info,
					sizeof(struct stp_sstpi));
			if (rc == 0 && stp_info.tmd != 2)
				rc = -EAGAIN;
=======
	if (xchg(&first, 1) == 0) {
		/* Wait until all other cpus entered the sync function. */
		while (atomic_read(&sync->cpus) != 0)
			cpu_relax();
		rc = 0;
		if (stp_info.todoff[0] || stp_info.todoff[1] ||
		    stp_info.todoff[2] || stp_info.todoff[3] ||
		    stp_info.tmd != 2) {
			rc = chsc_sstpc(stp_page, STP_OP_SYNC, 0,
					&clock_delta);
			if (rc == 0) {
				sync->clock_delta = clock_delta;
				clock_sync_global(clock_delta);
				rc = chsc_sstpi(stp_page, &stp_info,
						sizeof(struct stp_sstpi));
				if (rc == 0 && stp_info.tmd != 2)
					rc = -EAGAIN;
			}
>>>>>>> temp
		}
		sync->in_sync = rc ? -EAGAIN : 1;
		xchg(&first, 0);
	} else {
		/* Slave */
		atomic_dec(&sync->cpus);
		/* Wait for in_sync to be set. */
		while (READ_ONCE(sync->in_sync) == 0)
			__udelay(1);
	}
	if (sync->in_sync != 1)
		/* Didn't work. Clear per-cpu in sync bit again. */
		disable_sync_clock(NULL);
	/* Apply clock delta to per-CPU fields of this CPU. */
	clock_sync_local(sync->clock_delta);

	return 0;
}

/*
 * STP work. Check for the STP state and take over the clock
 * synchronization if the STP clock source is usable.
 */
static void stp_work_fn(struct work_struct *work)
{
	struct clock_sync_data stp_sync;
	int rc;

	/* prevent multiple execution. */
	mutex_lock(&stp_work_mutex);

	if (!stp_online) {
		chsc_sstpc(stp_page, STP_OP_CTRL, 0x0000, NULL);
		del_timer_sync(&stp_timer);
		goto out_unlock;
	}

	rc = chsc_sstpc(stp_page, STP_OP_CTRL, 0xb0e0, NULL);
	if (rc)
		goto out_unlock;

	rc = chsc_sstpi(stp_page, &stp_info, sizeof(struct stp_sstpi));
	if (rc || stp_info.c == 0)
		goto out_unlock;

	/* Skip synchronization if the clock is already in sync. */
	if (check_sync_clock())
		goto out_unlock;

	memset(&stp_sync, 0, sizeof(stp_sync));
	cpus_read_lock();
	atomic_set(&stp_sync.cpus, num_online_cpus() - 1);
	stop_machine_cpuslocked(stp_sync_clock, &stp_sync, cpu_online_mask);
	cpus_read_unlock();

	if (!check_sync_clock())
		/*
		 * There is a usable clock but the synchonization failed.
		 * Retry after a second.
		 */
		mod_timer(&stp_timer, jiffies + HZ);

out_unlock:
	mutex_unlock(&stp_work_mutex);
}

/*
 * STP subsys sysfs interface functions
 */
static struct bus_type stp_subsys = {
	.name		= "stp",
	.dev_name	= "stp",
};

static ssize_t stp_ctn_id_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	if (!stp_online)
		return -ENODATA;
	return sprintf(buf, "%016llx\n",
		       *(unsigned long long *) stp_info.ctnid);
}

static DEVICE_ATTR(ctn_id, 0400, stp_ctn_id_show, NULL);

static ssize_t stp_ctn_type_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	if (!stp_online)
		return -ENODATA;
	return sprintf(buf, "%i\n", stp_info.ctn);
}

static DEVICE_ATTR(ctn_type, 0400, stp_ctn_type_show, NULL);

static ssize_t stp_dst_offset_show(struct device *dev,
				   struct device_attribute *attr,
				   char *buf)
{
	if (!stp_online || !(stp_info.vbits & 0x2000))
		return -ENODATA;
	return sprintf(buf, "%i\n", (int)(s16) stp_info.dsto);
}

static DEVICE_ATTR(dst_offset, 0400, stp_dst_offset_show, NULL);

static ssize_t stp_leap_seconds_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	if (!stp_online || !(stp_info.vbits & 0x8000))
		return -ENODATA;
	return sprintf(buf, "%i\n", (int)(s16) stp_info.leaps);
}

static DEVICE_ATTR(leap_seconds, 0400, stp_leap_seconds_show, NULL);

static ssize_t stp_stratum_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	if (!stp_online)
		return -ENODATA;
	return sprintf(buf, "%i\n", (int)(s16) stp_info.stratum);
}

static DEVICE_ATTR(stratum, 0400, stp_stratum_show, NULL);

static ssize_t stp_time_offset_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	if (!stp_online || !(stp_info.vbits & 0x0800))
		return -ENODATA;
	return sprintf(buf, "%i\n", (int) stp_info.tto);
}

static DEVICE_ATTR(time_offset, 0400, stp_time_offset_show, NULL);

static ssize_t stp_time_zone_offset_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	if (!stp_online || !(stp_info.vbits & 0x4000))
		return -ENODATA;
	return sprintf(buf, "%i\n", (int)(s16) stp_info.tzo);
}

static DEVICE_ATTR(time_zone_offset, 0400,
			 stp_time_zone_offset_show, NULL);

static ssize_t stp_timing_mode_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	if (!stp_online)
		return -ENODATA;
	return sprintf(buf, "%i\n", stp_info.tmd);
}

static DEVICE_ATTR(timing_mode, 0400, stp_timing_mode_show, NULL);

static ssize_t stp_timing_state_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	if (!stp_online)
		return -ENODATA;
	return sprintf(buf, "%i\n", stp_info.tst);
}

static DEVICE_ATTR(timing_state, 0400, stp_timing_state_show, NULL);

static ssize_t stp_online_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	return sprintf(buf, "%i\n", stp_online);
}

static ssize_t stp_online_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	unsigned int value;

	value = simple_strtoul(buf, NULL, 0);
	if (value != 0 && value != 1)
		return -EINVAL;
	if (!test_bit(CLOCK_SYNC_HAS_STP, &clock_sync_flags))
		return -EOPNOTSUPP;
	mutex_lock(&clock_sync_mutex);
	stp_online = value;
	if (stp_online)
		set_bit(CLOCK_SYNC_STP, &clock_sync_flags);
	else
		clear_bit(CLOCK_SYNC_STP, &clock_sync_flags);
	queue_work(time_sync_wq, &stp_work);
	mutex_unlock(&clock_sync_mutex);
	return count;
}

/*
 * Can't use DEVICE_ATTR because the attribute should be named
 * stp/online but dev_attr_online already exists in this file ..
 */
static struct device_attribute dev_attr_stp_online = {
	.attr = { .name = "online", .mode = 0600 },
	.show	= stp_online_show,
	.store	= stp_online_store,
};

static struct device_attribute *stp_attributes[] = {
	&dev_attr_ctn_id,
	&dev_attr_ctn_type,
	&dev_attr_dst_offset,
	&dev_attr_leap_seconds,
	&dev_attr_stp_online,
	&dev_attr_stratum,
	&dev_attr_time_offset,
	&dev_attr_time_zone_offset,
	&dev_attr_timing_mode,
	&dev_attr_timing_state,
	NULL
};

static int __init stp_init_sysfs(void)
{
	struct device_attribute **attr;
	int rc;

	rc = subsys_system_register(&stp_subsys, NULL);
	if (rc)
		goto out;
	for (attr = stp_attributes; *attr; attr++) {
		rc = device_create_file(stp_subsys.dev_root, *attr);
		if (rc)
			goto out_unreg;
	}
	return 0;
out_unreg:
	for (; attr >= stp_attributes; attr--)
		device_remove_file(stp_subsys.dev_root, *attr);
	bus_unregister(&stp_subsys);
out:
	return rc;
}

device_initcall(stp_init_sysfs);
