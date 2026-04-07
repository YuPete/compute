#include "sched.h"

struct heater_rq global_rq;
DEFINE_RAW_SPINLOCK(global_rq_lock);

#define  get_heater_nr_running_from_cpu(cpu) (cpu_rq(cpu)->heater.nr_running)

void init_heater_rq(struct heater_rq *heater_rq)
{
	INIT_LIST_HEAD(&heater_rq->heater_list);
	heater_rq->nr_running = 0;	
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
	update_curr_freezer(rq);          // update runtime

}


static void switched_to_heater(struct rq *rq, struct task_struct *p)
{
	return;
}

static void switched_from_heater(struct rq *rq, struct task_struct *p)
{
	return;
}

static void
prio_changed_heater(struct rq *rq, struct task_struct *p, int oldprio)
{
	return;
}

static void
enqueue_task_heater(struct rq *rq, struct task_struct *p, int flags)
{

}

static void
dequeue_task_heater(struct rq *rq, struct task_struct *p, int flags)
{

}

#ifdef CONFIG_SMP
static int
select_task_rq_heater(struct task_struct *p, int cpu, int flags)
{
	return cpu
}

static int
balance_heater(struct rq *rq, struct task_struct *prev, struct rq_flags *rf)
{

	return cpu;
}

#endif

static void set_next_task_heater(struct rq *rq, struct task_struct *next, bool first)
{

}

static struct task_struct *pick_task_heater(struct rq *rq)
{

	return NULL;
}

struct task_struct *pick_next_task_heater(struct rq *rq)
{
	return NULL;
}
/*
 * heater tasks are unconditionally rescheduled:
 */
static void wakeup_preempt_heater(struct rq *rq, struct task_struct *p, int flags)
{
	return;
}

static void put_prev_task_heater(struct rq *rq, struct task_struct *prev)
{

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
