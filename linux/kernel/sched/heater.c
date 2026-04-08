#include "sched.h"
#include <linux/mmu_context.h>

LIST_HEAD(global_rq);
DEFINE_RAW_SPINLOCK(global_rq_lock);


/*
 * Per-CPU kthreads are allowed to run on !active && online CPUs, see
 * __set_cpus_allowed_ptr() and select_fallback_rq().
 */
static inline bool is_cpu_allowed(struct task_struct *p, int cpu)
{
	/* When not in the task's cpumask, no point in looking further. */
	if (!cpumask_test_cpu(cpu, p->cpus_ptr))
		return false;

	/* migrate_disabled() must be allowed to finish. */
	if (is_migration_disabled(p))
		return cpu_online(cpu);

	/* Non kernel threads are not allowed during either online or offline. */
	if (!(p->flags & PF_KTHREAD))
		return cpu_active(cpu) && task_cpu_possible(cpu, p);

	/* KTHREAD_IS_PER_CPU is always allowed. */
	if (kthread_is_per_cpu(p))
		return cpu_online(cpu);

	/* Regular kernel threads don't get to stay during offline. */
	if (cpu_dying(cpu))
		return false;

	/* But are allowed during online. */
	return cpu_online(cpu);
}



void init_heater_rq(struct heater_rq *heater_rq)
{
	heater_rq->run_q = NULL;
}

static void update_curr_heater(struct rq *rq)
{
	struct task_struct *curr = rq->curr;

	if (curr->sched_class != &heater_sched_class)
		return;

	update_curr_common(rq);
}

static void task_tick_heater(struct rq *rq, struct task_struct *curr, int queued)
{
	update_curr_heater(rq);          // update runtime

}


static void switched_to_heater(struct rq *rq, struct task_struct *p)
{
	return;
}

static void switched_from_heater(struct rq *rq, struct task_struct *p)
{
}

static void
prio_changed_heater(struct rq *rq, struct task_struct *p, int oldprio)
{
}

static void
enqueue_task_heater(struct rq *rq, struct task_struct *p, int flags)
{
	struct sched_heater_entity *heater_se = &(p->heater);
	struct heater_rq *heater = &rq->heater;

	pr_info("enqueue heater\n");
	if (heater_se->on_rq)
		return;

	//case 1: per-cpu-runq is NULL
	if (!heater->run_q) {
		heater->run_q = p;
		heater_se->on_rq = true;
		add_nr_running(rq, 1);
		return;
	}

	//case 2: overflow case
	raw_spin_lock(&global_rq_lock);
	list_add_tail(&heater_se->heater_list, &global_rq);
	heater_se->on_rq = true;
	raw_spin_unlock(&global_rq_lock);
}

static void
dequeue_task_heater(struct rq *rq, struct task_struct *p, int flags)
{
	struct sched_heater_entity *heater_se = &(p->heater);
	struct heater_rq *heater = &rq->heater;

	pr_info("dequeue heater\n");
	if (!heater_se->on_rq)
		return;

	update_curr_heater(rq);

	//case 1: task is on per_cpu_rq
	if (heater->run_q == p) {
		heater->run_q = NULL;
		sub_nr_running(rq, 1);
	} else {
	//case 2: task is on global_rq
		raw_spin_lock(&global_rq_lock);
		list_del_init(&heater_se->heater_list);
		raw_spin_unlock(&global_rq_lock);
	}

	heater_se->on_rq = false;
}

#ifdef CONFIG_SMP
static int
select_task_rq_heater(struct task_struct *p, int cpu, int flags)
{
	return cpu;
}

static int
balance_heater(struct rq *rq, struct task_struct *prev, struct rq_flags *rf)
{

	return 0;
}

#endif

static void set_next_task_heater(struct rq *rq, struct task_struct *next, bool first)
{
	next->se.exec_start = rq_clock_task(rq);
}

static struct task_struct *pick_task_heater(struct rq *rq)
{
	struct heater_rq *heater = &rq->heater;
	struct sched_heater_entity *heater_se;
	struct task_struct *next;
	int curr_cpu = smp_processor_id();

	//case 1: smth is one the per-cpu-runq
	if (heater->run_q)
		return heater->run_q;

	raw_spin_lock(&global_rq_lock);

	//case 2: the global runqueue is empty
	if (list_empty(&global_rq)) {
		raw_spin_unlock(&global_rq_lock);
		return NULL;
	}

	heater_se = list_first_entry(&global_rq,
				      struct sched_heater_entity,
				      heater_list);

	next = container_of(heater_se, struct task_struct, heater);

	//case 3: the head of the global_rq can't be run on this cpu
	if (!is_cpu_allowed(next,curr_cpu)) {
		raw_spin_unlock(&global_rq_lock);
		return NULL;
	}

	//case 4: we run the head on this cpu
	list_del_init(&heater_se->heater_list);
	heater->run_q = next;
	add_nr_running(rq, 1);

	if (task_cpu(next) != curr_cpu)
		set_task_cpu(next, curr_cpu);

	raw_spin_unlock(&global_rq_lock);
	return next;
}

struct task_struct *pick_next_task_heater(struct rq *rq)
{
	struct task_struct *next = pick_task_heater(rq);

	pr_info("pick_next_task_heater\n");

	if (!next)
		return NULL;

	set_next_task_heater(rq, next, true);
	return next;
}
/*
 * heater tasks are unconditionally rescheduled:
 */
static void wakeup_preempt_heater(struct rq *rq, struct task_struct *p, int flags)
{
}

static void put_prev_task_heater(struct rq *rq, struct task_struct *prev)
{
	update_curr_heater(rq);
}

/*
 * Simple, special scheduling class for the per-CPU heater tasks:
 */
DEFINE_SCHED_CLASS(heater) = {


	.enqueue_task		= enqueue_task_heater,
	.dequeue_task		= dequeue_task_heater,

	.wakeup_preempt		= wakeup_preempt_heater,
	.pick_task		= pick_task_heater,

	.pick_next_task		= pick_next_task_heater,
	.put_prev_task		= put_prev_task_heater,
	.set_next_task          = set_next_task_heater,

#ifdef CONFIG_SMP
	.balance		= balance_heater,
	.select_task_rq		= select_task_rq_heater,
	.set_cpus_allowed	= set_cpus_allowed_common,
	.switched_from		= switched_from_heater,
#endif

	.task_tick		= task_tick_heater,

	.prio_changed		= prio_changed_heater,
	.switched_to		= switched_to_heater,
	.update_curr		= update_curr_heater,
};
