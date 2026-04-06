#include "sched.h"

#define  FREEZER_TIMESLICE ((100 * HZ)/1000)

int sched_freezer_timeslice = FREEZER_TIMESLICE;

void init_freezer_rq(struct freezer_rq *freezer_rq)
{
	INIT_LIST_HEAD(&freezer_rq->freezer_list);
	freezer_rq->nr_running = 0;
}

static void requeue_task_freezer(struct rq *rq, struct task_struct *p)
{
	struct sched_freezer_entity *fz_se = &p->freezer;
	struct freezer_rq *fz_rq = &rq->freezer;

	list_move_tail(&fz_se->freezer_list, &fz_rq->freezer_list);
}

static void update_curr_freezer(struct rq *rq)
{
	struct task_struct *curr = rq->curr;

	if (curr->sched_class != &freezer_sched_class)
		return;

	update_curr_common(rq);
}

static void task_tick_freezer(struct rq *rq, struct task_struct *curr, int queued)
{
	struct sched_freezer_entity *fz_se = &curr->freezer;

	update_curr_freezer(rq);          // update runtime
	//no need for avg load time -> 1 cpu

	//watchdof is like rt specific

	//decrement time and check
	if (--fz_se->time_slice)
		return;

	//reset time slice
	fz_se->time_slice = sched_freezer_timeslice;

	//gotta update queue otherwise flags won't be set
	//and will just return to og task
	if (fz_se->freezer_list.prev != fz_se->freezer_list.next) {
		// rotate task to back of queue
		requeue_task_freezer(rq, curr);
		// set the flag
		resched_curr(rq);
		return;
	}
}


static void switched_to_freezer(struct rq *rq, struct task_struct *p)
{
	return;
}

static void switched_from_freezer(struct rq *rq, struct task_struct *p)
{
	return;
}

static void
prio_changed_freezer(struct rq *rq, struct task_struct *p, int oldprio)
{
	return;
}

static void
enqueue_task_freezer(struct rq *rq, struct task_struct *p, int flags)
{
	struct sched_freezer_entity *freezer_se = &(p->freezer);

	pr_info("enqueue\n");
	if (!(&freezer_se->freezer_list == freezer_se->freezer_list.next &&
      &freezer_se->freezer_list == freezer_se->freezer_list.prev))
		return;

	freezer_se->time_slice = sched_freezer_timeslice;
	list_add_tail(&freezer_se->freezer_list, &rq->freezer.freezer_list);
	++rq->freezer.nr_running;
}

static void
dequeue_task_freezer(struct rq *rq, struct task_struct *p, int flags)
{
	struct sched_freezer_entity *freezer_se = &(p->freezer);

	pr_info("dequeue\n");
	update_curr_freezer(rq);
	if (&freezer_se->freezer_list == freezer_se->freezer_list.next &&
    &freezer_se->freezer_list == freezer_se->freezer_list.prev)
		return;

	list_del_init(&freezer_se->freezer_list);
	--rq->freezer.nr_running;
}

#ifdef CONFIG_SMP
static int
select_task_rq_freezer(struct task_struct *p, int cpu, int flags)
{
	pr_info("select_task_rq_freezer\n");
	return cpu;
}

static int
balance_freezer(struct rq *rq, struct task_struct *prev, struct rq_flags *rf)
{
	pr_info("balance_freezer");
	return 0;
}

static struct task_struct *pick_task_freezer(struct rq *rq)
{
	struct freezer_rq *freezer = &rq->freezer;
	struct sched_freezer_entity *freezer_se;
	struct task_struct *next;

	pr_info("select_pick_task_freezer\n");

	if (list_empty(&freezer->freezer_list))
		return NULL;

	freezer_se = list_first_entry(&freezer->freezer_list,
				      struct sched_freezer_entity,
				      freezer_list);

	next = container_of(freezer_se, struct task_struct, freezer);

	return next;
}

#endif

static void set_next_task_freezer(struct rq *rq, struct task_struct *next, bool first)
{
	return;
}

struct task_struct *pick_next_task_freezer(struct rq *rq)
{
	struct task_struct *next = pick_task_freezer(rq);

	pr_info("pick_next_task\n");

	if (!next)
		return NULL;

	set_next_task_freezer(rq, next, true);
	return next;
}
/*
 * freezer tasks are unconditionally rescheduled:
 */
static void wakeup_preempt_freezer(struct rq *rq, struct task_struct *p, int flags)
{
	return;
}

static void put_prev_task_freezer(struct rq *rq, struct task_struct *prev)
{
	update_curr_freezer(rq);
}

/*
 * Simple, special scheduling class for the per-CPU freezer tasks:
 */
DEFINE_SCHED_CLASS(freezer) = {


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
	.switched_from		= switched_from_freezer,
#endif

	.task_tick		= task_tick_freezer,

	.prio_changed		= prio_changed_freezer,
	.switched_to		= switched_to_freezer,
	.update_curr		= update_curr_freezer,
};
