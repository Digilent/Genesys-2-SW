#ifndef PMON_H
#define PMON_H

#include "xstatus.h"


#define PERF_MON_METRIC_SEL_0		(0x44)
#define PERF_MON_METRIC_SEL_1		(0x48)
#define PERF_MON_METRIC_SEL_2		(0x4C)
#define PERF_MON_CONTROL_REG		(0x300)
#define PERF_MON_AGENT_SELECT_REG	(0x40)


#define METRIC_COUNTER_REG_0	(0x100)
#define METRIC_COUNTER_REG_1	(0x110)
#define METRIC_COUNTER_REG_2	(0x120)
#define METRIC_COUNTER_REG_3	(0x130)
#define METRIC_COUNTER_REG_4	(0x140)
#define METRIC_COUNTER_REG_5	(0x150)
#define METRIC_COUNTER_REG_6	(0x160)
#define METRIC_COUNTER_REG_7	(0x170)
#define METRIC_COUNTER_REG_8	(0x180)
#define METRIC_COUNTER_REG_9	(0x190)

void perfmon_main();
void axi_perf_mon_config();

#endif /* PMON_H_ */
