#ifndef PLAN_MANAGER_H
#define PLAN_MANAGER_H

#include <string>
#include <vector>

class OperatorID;
class TaskProxy;

using Plan = std::vector<OperatorID>;

class PlanManager
{
  std::string plan_filename;
  int num_previously_generated_plans;
  bool is_part_of_anytime_portfolio;
  bool dump_plan;
  std::vector<Plan> found_plans;

public:
  PlanManager(std::string file_name = "sas_plan", bool dump_plan = true);

  int get_num_of_genertated_plans() const
  {
    return num_previously_generated_plans;
  }

  const std::vector<Plan> &get_found_plans() const
  {
    return found_plans;
  }

  void set_plan_filename(const std::string &plan_filename);
  void set_num_previously_generated_plans(int num_previously_generated_plans);
  void set_is_part_of_anytime_portfolio(bool is_part_of_anytime_portfolio);
  void set_dump_plan(bool dump) { dump_plan = dump; };

  /*
    Set generates_multiple_plan_files to true if the planner can find more than
    one plan and should number the plans as FILENAME.1, ..., FILENAME.n.
  */
  void save_plan(const Plan &plan, const TaskProxy &task_proxy,
                 bool generates_multiple_plan_files = false);
};

extern int calculate_plan_cost(const Plan &plan, const TaskProxy &task_proxy);

#endif
