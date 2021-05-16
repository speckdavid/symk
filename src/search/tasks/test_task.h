#ifndef TASKS_TEST_TASK_H
#define TASKS_TEST_TASK_H

#include "delegating_task.h"

namespace options {
class Options;
}

namespace tasks {

class TestTask : public DelegatingTask {
public:
  TestTask(const std::shared_ptr<AbstractTask> &parent);
  virtual ~TestTask() override = default;

  virtual int get_num_goals() const override;
  virtual FactPair get_goal_fact(int index) const override;
};
} // namespace tasks

#endif
