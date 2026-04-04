int sched_freezer_timeslice = FREEZER_TIMESLICE;

static void task_tick_freezer(struct rq *rq, struct task_struct *curr, int queued)
{
	WARN_ONCE(1, "msg");
}

static void switched_to_freezer(struct rq *rq, struct task_struct *p)
{
	WARN_ONCE(1, "msg");
}

static void
prio_changed_freezer(struct rq *rq, struct task_struct *p, int oldprio)
{
	WARN_ONCE(1, "msg");
}
static void update_curr_freezer(struct rq *rq)
{
	WARN_ONCE(1, "msg");
}

static void
enqueue_task_freezer(struct rq *rq, struct task_struct *p, int flags)
{
	WARN_ONCE(1, "msg");
}

static void
dequeue_task_freezer(struct rq *rq, struct task_struct *p, int flags)
{
	WARN_ONCE(1, "msg");
}

struct task_struct *pick_next_task_freezer(struct rq *rq)
{
	return NULL;
}
/*
 * freezer tasks are unconditionally rescheduled:
 */
static void wakeup_preempt_freezer(struct rq *rq, struct task_struct *p, int flags)
{
	WARN_ONCE(1, "msg");
}

static void put_prev_task_freezer(struct rq *rq, struct task_struct *prev)
{
	WARN_ONCE(1, "msg");
}

static void set_next_task_freezer(struct rq *rq, struct task_struct *next, bool first)
{
	WARN_ONCE(1, "msg");
}

#ifdef CONFIG_SMP
static int
select_task_rq_freezer(struct task_struct *p, int cpu, int flags)
{
	return 0; /* freezer tasks as never migrated */
}

static int
balance_freezer(struct rq *rq, struct task_struct *prev, struct rq_flags *rf)
{
	return 0;
}

static struct task_struct *pick_task_freezer(struct rq *rq)
{
	return NULL;
}
#endif
/*
 * Simple, special scheduling class for the per-CPU freezer tasks:
 */
DEFINE_SCHED_CLASS(freezer) = {

	/* no enqueue/yield_task for freezer tasks */

	/* dequeue is not valid, we print a debug message there: */
	.enqueue_task		= enqueue_task_freezer,
	.dequeue_task		= dequeue_task_freezer,

	.wakeup_preempt		= wakeup_preempt_freezer,

	.pick_next_task		= pick_next_task_freezer,
	.put_prev_task		= put_prev_task_freezer,
	.set_next_task          = set_next_task_freezer,

#ifdef CONFIG_SMP
	.balance		= balance_freezer,
	.pick_task		= pick_task_freezer,
	.select_task_rq		= select_task_rq_freezer,
	.set_cpus_allowed	= set_cpus_allowed_common,
#endif

	.task_tick		= task_tick_freezer,

	.prio_changed		= prio_changed_freezer,
	.switched_to		= switched_to_freezer,
	.update_curr		= update_curr_freezer,
};
