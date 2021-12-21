#ifndef PLAN_MANAGER_H
#define PLAN_MANAGER_H

#include <string>
#include <vector>

class OperatorID;
class TaskProxy;

using Plan = std::vector<OperatorID>;

class PlanManager {
    std::string plan_filename;
    int num_previously_generated_plans;
    bool is_part_of_anytime_portfolio;
public:
    PlanManager();

    void set_plan_filename(const std::string &plan_filename);
    std::string get_plan_filename() const {return plan_filename;}
    void set_num_previously_generated_plans(int num_previously_generated_plans);
    void set_is_part_of_anytime_portfolio(bool is_part_of_anytime_portfolio);
    int get_num_previously_generated_plans() const {
        return num_previously_generated_plans;
    }

    void dump_plan(const Plan &plan, const TaskProxy &task_proxy) const;
    /*
      Set generates_multiple_plan_files to true if the planner can find more than
      one plan and should number the plans as FILENAME.1, ..., FILENAME.n.
    */
    void save_plan(const Plan &plan, const TaskProxy &task_proxy,
                   bool dump_plan = true,
                   bool generates_multiple_plan_files = false);
};

extern int calculate_plan_cost(const Plan &plan, const TaskProxy &task_proxy);

#endif
