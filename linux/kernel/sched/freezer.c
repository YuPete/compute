static void requeue_task_freezer(struct rq *rq, struct task_struct *p)
{
    struct sched_freezer_entity *fz_se = &p->freezer;
    struct freezer_rq *fz_rq = &rq->freezer;

    list_move_tail(&fz_se->run_list, &fz_rq->queue);
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

	//gotta update queue otherwise flags wont be set 
	//and will just return to og task
    if (fz_se->run_list.prev != fz_se->run_list.next) {
        // rotate task to back of queue
        requeue_task_freezer(rq, curr);
        // set the flag
        resched_curr(rq);
        return;
    }
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
       struct sched_freezer_entity *freezer_se = &(p->freezer);

       list_del_init(&freezer_se->freezer_list);
       update_curr_freezer(rq);
       --rq.freezer.nr_running;
}

struct task_struct *pick_next_task_freezer(struct rq *rq)
{
	struct freezer_rq *freezer = &rq->freezer;
	struct sched_freezer_entity *freezer_se;
	struct task_struct *next;

	freezer_se = list_first_entry(&freezer->freezer_list,
				      struct sched_freezer_entity,
				      freezer_list);

	next = container_of(freezer_se, struct task_struct, freezer);

	set_next_task_freezer(rq, next, true);
	return next;
}
/*
 * freezer tasks are unconditionally rescheduled:
 */
static void wakeup_preempt_freezer(struct rq *rq, struct task_struct *p, int flags)
{
	resched_curr(rq);
}

static void put_prev_task_freezer(struct rq *rq, struct task_struct *prev)
{
}

static void set_next_task_freezer(struct rq *rq, struct task_struct *next, bool first)
{
	update_freezer_core(rq);
	schedstat_inc(rq->sched_gofreezer);
}

#ifdef CONFIG_SMP
static int
select_task_rq_freezer(struct task_struct *p, int cpu, int flags)
{
	return task_cpu(p); /* freezer tasks as never migrated */
}

static int
balance_freezer(struct rq *rq, struct task_struct *prev, struct rq_flags *rf)
{
	return WARN_ON_ONCE(1);
}

static struct task_struct *pick_task_freezer(struct rq *rq)
{
	return rq->freezer;
}

static int
select_task_rq_freezer(struct task_struct *p, int cpu, int flags)
{
	return task_cpu(p); /* freezer tasks as never migrated */
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