#include <defs.h>
#include <list.h>
#include <proc.h>
#include <assert.h>
#include <default_sched.h>
#define BIG_STRIDE 0x7fffffff // max signed int
static void
RR_init(struct run_queue *rq) {
     // list_init(&(rq->run_list));
	rq->lab6_run_pool = NULL;
    rq->proc_num = 0;
}
int stride_cmp(void *a, void *b){
	uint32_t s1 = le2proc(a, lab6_run_pool)->lab6_stride;
	uint32_t s2 = le2proc(b, lab6_run_pool)->lab6_stride;
	int32_t res = s1-s2;
	if (res>0) return 1;
	else if (res ==0)return 0;
	else return -1;
}
// add to runnable process pool
static void
RR_enqueue(struct run_queue *rq, struct proc_struct *proc) {
	//cprintf("enqueue: total %d\n", rq->proc_num+1);
//    assert(list_empty(&(proc->run_link)));
//    list_add_before(&(rq->run_list), &(proc->run_link));
    if (proc->time_slice == 0 || proc->time_slice > rq->max_time_slice) {
        proc->time_slice = rq->max_time_slice;
    }
//    proc->rq = rq;
	rq->lab6_run_pool = skew_heap_insert(rq->lab6_run_pool, &(proc->lab6_run_pool), stride_cmp);
	//proc->rq = rq;
	rq->proc_num ++;
}

static void
RR_dequeue(struct run_queue *rq, struct proc_struct *proc) {
	//cprintf("dequeue: total%d\n", rq->proc_num-1);
//    assert(!list_empty(&(proc->run_link)) && proc->rq == rq);
//    list_del_init(&(proc->run_link));
	rq->lab6_run_pool = skew_heap_remove(rq->lab6_run_pool, &(proc->lab6_run_pool), stride_cmp);
    rq->proc_num --;
}

// schedule manager picks a process to run
static struct proc_struct *
RR_pick_next(struct run_queue *rq) {
	// cprintf("pick next\n");
//    list_entry_t *le = list_next(&(rq->run_list));
//    if (le != &(rq->run_list)) {
//        return le2proc(le, run_link);
//    }
	if (rq->lab6_run_pool == NULL) return NULL;
	struct proc_struct* proc = le2proc(rq->lab6_run_pool, lab6_run_pool);
	if (proc->lab6_priority == 0)
		proc->lab6_stride += BIG_STRIDE;
	else proc->lab6_stride += BIG_STRIDE/proc->lab6_priority;
	// cprintf("picking proc %d stride is now %x\n", proc->pid, proc->lab6_stride);
	return proc;
    // return NULL;
}

// called when tick intr arrives. 
// this function can choose to flag that the process needs stop, 
// the schedule manager should take over and pick next process.
static void
RR_proc_tick(struct run_queue *rq, struct proc_struct *proc) {
    if (proc->time_slice > 0) {
        proc->time_slice --;
    }
    if (proc->time_slice == 0) {
        proc->need_resched = 1;
    }
}

struct sched_class default_sched_class = {
    //.name = "RR_scheduler",
	.name = "stride_scheduler",
    .init = RR_init,
    .enqueue = RR_enqueue,
    .dequeue = RR_dequeue,
    .pick_next = RR_pick_next,
    .proc_tick = RR_proc_tick,
};

