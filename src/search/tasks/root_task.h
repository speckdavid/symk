#ifndef TASKS_ROOT_TASK_H
#define TASKS_ROOT_TASK_H

#include "../abstract_task.h"

namespace tasks {
extern std::shared_ptr<AbstractTask> g_root_task;
extern void read_root_task(std::istream &in);

// Call the RootTask-specific helper on a task if it is a RootTask. This
// avoids exposing the full definition of RootTask in the header.
void add_cost_variable(AbstractTask &task, size_t max_value);
}
#endif
