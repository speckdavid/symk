#include "test_task.h"

#include "../option_parser.h"
#include "../plugin.h"

#include "../task_utils/task_properties.h"
#include "../tasks/root_task.h"
#include "../utils/system.h"

#include <iostream>
#include <memory>

using namespace std;
using utils::ExitCode;

namespace tasks {
TestTask::TestTask(
    const shared_ptr<AbstractTask> &parent)
    : DelegatingTask(parent) {
}

static shared_ptr<AbstractTask> _parse(OptionParser &parser) {
  Options opts = parser.parse();
  if (parser.dry_run()) {
    return nullptr;
  } else {
    return make_shared<TestTask>(g_root_task);
  }
}

int TestTask::get_num_goals() const {
    return parent->get_num_goals() - 1;
}

FactPair TestTask::get_goal_fact(int index) const {
    return parent->get_goal_fact(index);
}

static Plugin<AbstractTask> _plugin("test", _parse);
} // namespace tasks
