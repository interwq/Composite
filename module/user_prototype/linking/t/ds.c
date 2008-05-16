#include <cos_component.h>
#include <cos_scheduler.h>

extern int sched_create_child_brand(void);
extern void sched_child_yield_thd(void);

#define MAX_UPCALLS 2
#define MAX_RESOURCES 2
#define CPU_FREQ 2400000000
#define CPU_FREQ_PER_TICK ((unsigned int)(CPU_FREQ/100))

static volatile int ticks = 0, upcalls_alloc = 0, resources_alloc = 0;
static struct sched_thd *idle, *upcall;

static struct sched_thd *upcalls[MAX_UPCALLS];
static struct resource {
	unsigned long cycles_used, cycle_limit, period_elapsed, period;
} resources[MAX_RESOURCES];

extern int sched_create_net_upcall(unsigned short int port, int depth);

int sched_ds_create_net_upcall(unsigned short int port)
{
	int thd_id;
	struct sched_thd *t;
	static int urgency = 1;
	struct resource *r;

	thd_id = sched_create_net_upcall(port, 2);
	t = sched_alloc_thd(thd_id);
	assert(t);
	sched_alloc_event(t);
	sched_add_mapping(thd_id, t);
	sched_set_thd_urgency(t, urgency++);
	
	upcalls[upcalls_alloc++] = t;
	r = &resources[0/*resources_alloc++*/];
	resources_alloc = 1;
//	r->cycle_limit = (resources_alloc == 1) ? CPU_FREQ_PER_TICK*/*5*/6 : CPU_FREQ_PER_TICK*2;
	r->cycle_limit = CPU_FREQ_PER_TICK*10;
	r->period = 20;
	sched_get_accounting(t)->private = r;

	return thd_id;
}

static void evt_callback(struct sched_thd *t, u8_t flags, u32_t cpu_usage)
{
	struct resource *r;

	r = (struct resource*)sched_get_accounting(t)->private;
	r->cycles_used += cpu_usage;

	return;
}

extern void sched_suspend_thd(int thd_id);
extern void sched_resume_thd(int thd_id);

static void timer_tick(int amnt)
{
	int i;
	struct sched_thd *t;
	struct resource *r;

	cos_sched_lock_take();
	ticks++;
	cos_sched_process_events(evt_callback, 0);

	/* suspend any threads that have overshot their allocations */
	for (i = 0 ; i < upcalls_alloc ; i++) {
		t = upcalls[i];
		assert(t);

		r = sched_get_accounting(t)->private;
		if (!(t->flags & THD_SUSPENDED) && 
		    r->cycles_used > r->cycle_limit) {
			sched_suspend_thd(t->id);
			t->flags = THD_SUSPENDED;
		}
	}

	/* reset budgets and elapsed periods if we have hit a period */
	for (i = 0 ; i < resources_alloc ; i++) {
		r = &resources[i];

		r->period_elapsed++;
		if (r->period_elapsed >= r->period) {
			r->period_elapsed = 0;
			r->cycles_used = 0;
		}
	}

	/* 
	 * resume any threads that were suspended and had their
	 * budgets reset
	 */
	for (i = 0 ; i < upcalls_alloc ; i++) {
		t = upcalls[i];
		assert(t);

		r = sched_get_accounting(t)->private;
		if ((t->flags & THD_SUSPENDED) && 
		    r->cycles_used <= r->cycle_limit) {
			sched_resume_thd(t->id);
			t->flags = 0;
		}
	}
	
	cos_sched_lock_release();

	return;
}

static void sched_init(void)
{
	static int first = 1;
	int upcall_id;

	if (!first) return;
	first = 0;

	sched_ds_init();

	idle = sched_alloc_thd(cos_get_thd_id());

	print("ds sched_init %d%d%d", 0,0,0);
	upcall_id = sched_create_child_brand();
	upcall = sched_alloc_thd(upcall_id);

	print("created ds upcall %d. %d%d", upcall_id, 0,0);
	sched_child_yield_thd();
}

void cos_upcall_fn(upcall_type_t t, void *arg1, void *arg2, void *arg3)
{
	switch (t) {
	case COS_UPCALL_BRAND_EXEC:
	{
		static int first = 1;
		if (first) {
			cos_argreg_init();
			first = 0;
		}
		timer_tick((int)arg1);
		break;
	}
	case COS_UPCALL_BOOTSTRAP:
		sched_init();
		break;
	case COS_UPCALL_CREATE:
		assert(0);
		cos_argreg_init();
//		((crt_thd_fn_t)arg1)(arg2);
		break;
	case COS_UPCALL_BRAND_COMPLETE:
		assert(0);
		break;
	default:
		assert(0);
		return;
	}

	return;
}

