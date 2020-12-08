#undef TRACE_SYSTEM
#define TRACE_SYSTEM trace_event_sample

#if !defined(_TRACE_EVENT_SAMPLE_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_EVENT_SAMPLE_H

#include <linux/tracepoint.h>

TRACE_EVENT(foo_bar,

	TP_PROTO(int bar),

	TP_ARGS(bar),

	TP_STRUCT__entry(__field(int,bar)),

	TP_fast_assign(__entry->bar = bar;),

	TP_printk("foo_bar %d", __entry->bar));

#endif

#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH /home/autotest/Documents/sources/tracing
#include <trace/define_trace.h>
