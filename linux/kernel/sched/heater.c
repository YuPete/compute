#include "sched.h"

LIST_HEAD(global_rq);
DEFINE_RAW_SPINLOCK(global_rq_lock);

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
	struct sched_heater_entity *heater_se = &(p->heater);
	struct heater_rq *heater = &rq->heater;

	pr_info("enqueue heater\n");
	if (heater_se->on_rq)
		return;

	if (!heater->run_q) {
		heater->run_q = p;
		heater_se->on_rq = true;
		add_nr_running(rq, 1);
		return;
	}

	raw_spin_lock(&global_rq_lock);
	list_add_tail(&heater_se->heater_list, &global_rq);
	heater_se->on_rq = true;
	raw_spin_unlock(&global_rq_lock);
	add_nr_running(rq, 1);
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
	
	if (heater->run_q == p) {
         heater->run_q = NULL;
    } else {
		//what if signal or something comes and kills task on global queue
        raw_spin_lock(&global_rq_lock);
        list_del_init(&heater_se->heater_list);
        raw_spin_unlock(&global_rq_lock);
    }

    heater_se->on_rq = false;
    sub_nr_running(rq, 1);
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
	int cur_cpu =  smp_processor_id();
	int cpu;

	if (heater->run_q)
		return heater->run_q;

	raw_spin_lock(&global_rq_lock);

	if (list_empty(&global_rq)) {
		raw_spin_unlock(&global_rq_lock);
		return NULL;
	}

	heater_se = list_first_entry(&global_rq,
				      struct sched_heater_entity,
				      heater_list);

	next = container_of(heater_se, struct task_struct, heater);

	//TODO: add check for if next cannot be run on this cpu; edit implemented below

	       if (!is_cpu_allowed(next,cur_cpu)) {
                raw_spin_unlock(&global_rq_lock);
                return NULL;
        }

	list_del_init(&heater_se->heater_list);
	heater->run_q = next;

	cpu = smp_processor_id();
	if (task_cpu(next) != cpu)
		set_task_cpu(next, cpu);

	raw_spin_unlock(&global_rq_lock);
	return next;
}

struct task_struct *pick_next_task_heater(struct rq *rq)
{
	struct task_struct *next = pick_task_heater(rq);

	pr_info("pick_next_task\n");

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
	return;
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
