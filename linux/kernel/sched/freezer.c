#include "sched.h"
#include <linux/mmu_context.h>

#define  FREEZER_TIMESLICE ((100 * HZ)/1000)
#define  get_freezer_nr_running(cpu) (READ_ONCE(cpu_rq(cpu)->freezer.nr_running))

struct task_struct *pick_next_task_freezer(struct rq *rq);
int balance_freezer(struct rq *rq, struct task_struct *prev, struct rq_flags *rf);

int sched_freezer_timeslice = FREEZER_TIMESLICE;

static inline bool is_cpu_allowed(struct task_struct *p, int cpu)
{
	/* When not in the task's cpumask, no point in looking further. */
	if (!cpumask_test_cpu(cpu, p->cpus_ptr)) {
		//pr_info("%d\n", 2);
		return false;
	}
		

	/* migrate_disabled() must be allowed to finish. */
	if (is_migration_disabled(p)) {
		//pr_info("%d\n", 3);
		return false;
	}
	
	/* But are allowed during online. */
	
	return true;
}

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

	update_curr_freezer(rq);

	if (--fz_se->time_slice)
		return;

	fz_se->time_slice = sched_freezer_timeslice;

	if (fz_se->freezer_list.prev != fz_se->freezer_list.next) {
		requeue_task_freezer(rq, curr);
		resched_curr(rq);
		return;
	}
}


static void switched_to_freezer(struct rq *rq, struct task_struct *p)
{
}

static void switched_from_freezer(struct rq *rq, struct task_struct *p)
{
}

static void
prio_changed_freezer(struct rq *rq, struct task_struct *p, int oldprio)
{
}

static void
enqueue_task_freezer(struct rq *rq, struct task_struct *p, int flags)
{
	struct sched_freezer_entity *freezer_se = &(p->freezer);

	//if (strcmp(p->comm, "fibonacci") == 0)
	//	pr_info("enqueue\n");

	if (freezer_se->on_rq)
		return;

	freezer_se->time_slice = sched_freezer_timeslice;
	list_add_tail(&freezer_se->freezer_list, &rq->freezer.freezer_list);
	WRITE_ONCE(rq->freezer.nr_running, rq->freezer.nr_running+1);
	add_nr_running(rq, 1);
	freezer_se->on_rq = true;
}

static void
dequeue_task_freezer(struct rq *rq, struct task_struct *p, int flags)
{
	//if (strcmp(p->comm, "fibonacci") == 0)
		//pr_info("dequeue\n");

	struct sched_freezer_entity *freezer_se = &(p->freezer);

	//pr_info("dequeue\n");
	if (!freezer_se->on_rq)
		return;

	update_curr_freezer(rq);
	list_del_init(&freezer_se->freezer_list);
	WRITE_ONCE(rq->freezer.nr_running, rq->freezer.nr_running-1);
	sub_nr_running(rq, 1);
	freezer_se->on_rq = false;
}

#ifdef CONFIG_SMP
static int
select_task_rq_freezer(struct task_struct *p, int cpu, int flags)
{
	//pr_info("select_task_rq_freezer\n");
	int cpu_candidate;
	unsigned long cur_min, tmp;

	//besides wakeups and fork, return task_cpu
	if (!(flags & (WF_TTWU | WF_FORK)))
		goto out;
	cur_min = get_freezer_nr_running(cpu);

	for_each_cpu(cpu_candidate, p->cpus_ptr) {
		//pr_info("cycling through cpus\n");
		tmp = get_freezer_nr_running(cpu_candidate);
		//pr_info("cpu %d has %lu freezer tasks\n", cpu_candidate, tmp);
		if (tmp < cur_min) {
			cpu = cpu_candidate;
			cur_min = tmp;
		}
	}
	//pr_info("%d cpu chosen!\n", cpu);
out:
	return cpu;
}

static bool is_task_allowed(struct task_struct *candidate, int new_cpu, int old_cpu)
{
	if (kthread_is_per_cpu(candidate)) {
		//pr_info("%d\n", 1);
		return false;
	}
	if (!is_cpu_allowed(candidate, new_cpu))
		return false;
	if (task_on_cpu(cpu_rq(old_cpu), candidate)) {
		//pr_info("%d\n", 4);
		return false;
	}
	//pr_info("%d\n", 10);
	return true;

}

//we should be holding the current rq's spin lock
int balance_freezer(struct rq *rq, struct task_struct *prev, struct rq_flags *rf)
{
	int cur_cpu = cpu_of(rq);
	int candidate_cpu; // for loop
	int cur_max_cpu = -1; // current most busy cpu
	int cur_max_ftasks = 1;
	struct sched_freezer_entity *freezer_se;
	struct task_struct *next = NULL;
	struct freezer_rq *candidate_freezer_rq;

	//1. find cpu to steal freezer tasks from
	//pr_info("balance_freezer");
	if (rq->freezer.nr_running != 0)
		return 0;

	for_each_online_cpu(candidate_cpu) {
		if (candidate_cpu == cur_cpu)
			continue;
		if (get_freezer_nr_running(candidate_cpu) > cur_max_ftasks) {
			cur_max_cpu = candidate_cpu;
			cur_max_ftasks = get_freezer_nr_running(candidate_cpu);
		}
	}

	if (cur_max_cpu == -1)
		return 0;

	//2. move said task that we found to cur_cpu
	rq_unpin_lock(rq, rf);
	double_lock_balance(rq, cpu_rq(cur_max_cpu));

	if (get_freezer_nr_running(cur_cpu) != 0 || get_freezer_nr_running(cur_max_cpu) < 2)
		goto fail;

	candidate_freezer_rq =  &cpu_rq(cur_max_cpu)->freezer;

	list_for_each_entry(freezer_se, &candidate_freezer_rq->freezer_list, freezer_list) {
		next = container_of(freezer_se, struct task_struct, freezer);
		if (is_task_allowed(next, cur_max_cpu, cur_cpu)) // custom check
			goto success;
		next = NULL;
	}

fail:
	double_unlock_balance(rq, cpu_rq(cur_max_cpu));
	rq_repin_lock(rq, rf);
	return 0;

success:
	//3.move cpu logic, todo
	//pr_info("success");
	dequeue_task_freezer(cpu_rq(cur_max_cpu), next, 0);
	set_task_cpu(next, cur_cpu);
	enqueue_task_freezer(rq, next, 0);
	double_unlock_balance(rq, cpu_rq(cur_max_cpu));
	rq_repin_lock(rq, rf);
	return 1;

}

#endif

static void set_next_task_freezer(struct rq *rq, struct task_struct *next, bool first)
{
	next->se.exec_start = rq_clock_task(rq);
}

static struct task_struct *pick_task_freezer(struct rq *rq)
{
	struct freezer_rq *freezer = &rq->freezer;
	struct sched_freezer_entity *freezer_se;
	struct task_struct *next;

	//pr_info("select_pick_task_freezer\n");

	if (list_empty(&freezer->freezer_list))
		return NULL;

	freezer_se = list_first_entry(&freezer->freezer_list,
				      struct sched_freezer_entity,
				      freezer_list);

	next = container_of(freezer_se, struct task_struct, freezer);

	return next;
}

struct task_struct *pick_next_task_freezer(struct rq *rq)
{
	struct task_struct *next = pick_task_freezer(rq);

	//pr_info("pick_next_task\n");

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
}

static void put_prev_task_freezer(struct rq *rq, struct task_struct *prev)
{
	update_curr_freezer(rq);
}

static void yield_task_freezer(struct rq *rq)
{
	requeue_task_freezer(rq, rq->curr);
}


/*
 * Simple, special scheduling class for the per-CPU freezer tasks:
 */
DEFINE_SCHED_CLASS(freezer) = {


	.enqueue_task		= enqueue_task_freezer,
	.dequeue_task		= dequeue_task_freezer,
	.yield_task		= yield_task_freezer,

	.wakeup_preempt		= wakeup_preempt_freezer,
	.pick_task		= pick_task_freezer,

	.pick_next_task		= pick_next_task_freezer,
	.put_prev_task		= put_prev_task_freezer,
	.set_next_task          = set_next_task_freezer,

#ifdef CONFIG_SMP
	.balance		= balance_freezer,
	.select_task_rq		= select_task_rq_freezer,
	.set_cpus_allowed	= set_cpus_allowed_common,
	.switched_from		= switched_from_freezer,
#endif

	.task_tick		= task_tick_freezer,

	.prio_changed		= prio_changed_freezer,
	.switched_to		= switched_to_freezer,
	.update_curr		= update_curr_freezer,
};
