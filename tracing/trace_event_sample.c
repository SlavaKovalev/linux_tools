#include <linux/module.h>
#include <linux/kthread.h>

#define CREATE_TRACE_POINTS
#include "trace_event_sample.h"

/*static const char *random_strings[] = {
	"Mother Goose",
	"Snoopy",
	"Gandalf",
	"Frodo",
	"One ring to rule them all"
};*/

static struct task_struct *simple_tsk;

static void simple_thread_func(int cnt)
{
	int array[6];
	int len = cnt % 5;
	int i;

	set_current_state(TASK_INTERRUPTIBLE);
	schedule_timeout(HZ);

	for (i = 0; i < len; i++)
		array[i] = i + 1;
	array[i] = 0;

	/* Silly tracepoints */
	trace_foo_bar(cnt);
}

static int simple_thread(void *arg)
{
	int cnt = 0;

	while (!kthread_should_stop())
		simple_thread_func(cnt++);

	return 0;
}

static int __init trace_event_init(void)
{
	simple_tsk = kthread_run(simple_thread, NULL, "event_sample");
	if (IS_ERR(simple_tsk))
		return -1;

	return 0;
}

static void __exit trace_event_exit(void)
{
	kthread_stop(simple_tsk);
}

module_init(trace_event_init);
module_exit(trace_event_exit);

MODULE_AUTHOR("Svyatoslav Kovalev");
MODULE_DESCRIPTION("trace_events_sample");
MODULE_LICENSE("GPL");
