#ifndef ROUND_ROBIN_SCHEDULER_H
#define ROUND_ROBIN_SCHEDULER_H

#include "scheduler_interface.h"
#include "os.h"

// Pass in the desired quantum when creating
Scheduler* create_rr_scheduler(int quantum);

#endif