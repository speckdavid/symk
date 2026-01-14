#include "astar_count.h"

#include "search_common.h"

#include "../evaluation_context.h"
#include "../open_list_factory.h"
#include "../plan_manager.h"

#include "../plugins/plugin.h"
#include "../task_utils/successor_generator.h"
#include "../task_utils/task_properties.h"
#include "../tasks/root_task.h"

#include <cmath>
#include <iomanip>

using namespace std;

// TODO: If we have a relative bound and no absolut, we may get nodes in the
// open list which are to costly. So, we can stop early when the bound is
// exceeded. Q: Do we need tie breaking [f,g,h] In particular the h in the end?
// probably not but valuable to thing about it.

namespace astar_count {
AStarCount::AStarCount(
    const shared_ptr<Evaluator> &h_eval, OperatorCost cost_type, int bound,
    double max_time, const string &description, utils::Verbosity verbosity,
    double quality_multiplier, bool prefix_optimal,
    unordered_set<int> projected_operators)
    : SearchAlgorithm(cost_type, bound, max_time, description, verbosity),
      quality_multiplier(quality_multiplier),
      prefix_optimal(prefix_optimal),
      projected_operators(move(projected_operators)),
      num_goal_paths(0),
      max_plan_cost(-1) {
    auto [open_list_factory, eval] =
        search_common::create_astar_count_open_list_factory_and_f_eval(
            h_eval, verbosity);
    open_list = open_list_factory->create_state_open_list();
    f_evaluator = eval;

    log << "Prefix optimality: " << (prefix_optimal ? "True" : "False") << endl;

    for (int op_id : this->projected_operators) {
        assert(
            op_id >= 0 &&
            op_id < static_cast<int>(task_proxy.get_operators().size()));
        log << "Projecting operator with ID " << op_id << ": "
            << task_proxy.get_operators()[op_id].get_name() << endl;
    }
}

void AStarCount::initialize() {
    log << "Conducting astar count search" << ", (real) bound = " << bound
        << endl;
    assert(open_list);

    State initial_state = state_registry.get_initial_state();
    state_cost_to_path_count[initial_state.get_id().get_value()][0] = 1;

    EvaluationContext eval_context(initial_state, 0, true, &statistics);

    statistics.inc_evaluated_states();

    if (open_list->is_dead_end(eval_context)) {
        log << "Initial state is a dead end." << endl;
    } else {
        if (search_progress.check_progress(eval_context))
            statistics.print_checkpoint_line(0);
        start_f_value_statistics(eval_context);
        SearchNode node = search_space.get_node(initial_state);
        node.open_initial();

        open_list->insert(eval_context, initial_state.get_id());
    }

    print_initial_evaluator_values(eval_context);
}

SearchStatus AStarCount::step() {
    optional<SearchNode> node = pop_min_node();

    if (!node.has_value()) {
        assert(open_list->empty());
        return found_solution() ? SOLVED : FAILED;
    }

    // More expansive than the dynamic bound
    if (node->get_real_g() >= bound) {
        return IN_PROGRESS;
    }

    const State &s = node->get_state();
    EvaluationContext eval_context(s, node->get_g(), false, &statistics);

    node->close();
    assert(!node->is_dead_end());
    update_f_value_statistics(eval_context);
    statistics.inc_expanded();

    if (task_properties::is_goal_state(task_proxy, s)) {
        // task_properties::dump_fdr(s);
        num_goal_paths += state_cost_to_path_count[s.get_id().get_value()]
                                                  [node->get_real_g()];

        if (!found_solution()) {
            check_goal_and_set_plan(s);
            update_bound(node->get_real_g());
            log << "Plan cost bound: " << bound << endl;
        }
        max_plan_cost = max(max_plan_cost, node->get_real_g());
    }

    assert(node.has_value());
    expand(*node);

    return IN_PROGRESS;
}

void AStarCount::expand(const SearchNode &node) {
    const State &s = node.get_state();
    vector<OperatorID> applicable_ops;
    successor_generator.generate_applicable_ops(s, applicable_ops);

    for (OperatorID op_id : applicable_ops) {
        OperatorProxy op = task_proxy.get_operators()[op_id];
        if ((node.get_real_g() + op.get_cost()) >= bound)
            continue;

        State succ_state =
            state_registry.get_successor_state(s, op, !prefix_optimal);
        statistics.inc_generated();

        SearchNode succ_node = search_space.get_node(succ_state);

        // Previously encountered dead end. Don't re-evaluate.
        if (succ_node.is_dead_end())
            continue;

        int succ_g = node.get_g() + get_adjusted_cost(op);

        if (succ_node.is_new()) {
            EvaluationContext succ_eval_context(
                succ_state, succ_g, false, &statistics);
            statistics.inc_evaluated_states();

            if (open_list->is_dead_end(succ_eval_context)) {
                succ_node.mark_as_dead_end();
                statistics.inc_dead_ends();
                continue;
            }
            if (succ_eval_context.get_evaluator_value_or_infinity(
                    f_evaluator.get()) >= bound)
                continue;
            succ_node.open_new_node(node, op, get_adjusted_cost(op));

            open_list->insert(succ_eval_context, succ_state.get_id());
            if (search_progress.check_progress(succ_eval_context))
                statistics.print_checkpoint_line(succ_node.get_g());
        } else if (succ_node.get_g() > node.get_g() + get_adjusted_cost(op)) {
            if (succ_node.is_closed()) {
                // We found a new cheapest path to a closed state.
                cerr << "Found a new cheapest path to an open closed state! "
                     << "Only consistent heuristics are supported by now!"
                     << endl;
                utils::exit_with(utils::ExitCode::SEARCH_CRITICAL_ERROR);
            }
            assert(succ_node.is_open());
            succ_node.update_open_node_parent(node, op, get_adjusted_cost(op));
            EvaluationContext succ_eval_context(
                succ_state, succ_node.get_g(), false, &statistics);
            assert(
                succ_eval_context.get_evaluator_value_or_infinity(
                    f_evaluator.get()) < bound);
            open_list->insert(succ_eval_context, succ_state.get_id());
        } else {
            /*
              We found an equally or more expensive path to an open or closed
              state.
            */
        }

        update_path_count_for_state(succ_state, s, succ_g, node.get_g(), op);
    }
}

optional<SearchNode> AStarCount::pop_min_node() {
    while (!open_list->empty()) {
        StateID id = open_list->remove_min();
        State s = state_registry.lookup_state(id);
        SearchNode node = search_space.get_node(s);
        if (node.is_closed()) {
            // cout << s.get_id() << endl;
            continue;
        }
        return node;
    }
    return nullopt;
}

void AStarCount::update_path_count_for_state(
    const State &state, const State &parent, int state_g, int parent_g,
    const OperatorProxy &op) {
    assert(state_g >= parent_g);
    if (projected_operators.find(op.get_id()) != projected_operators.end()) {
        return;
    }

    auto state_id = state.get_id().get_value();
    auto parent_id = parent.get_id().get_value();

    // Update total path count
    Count parent_count = state_cost_to_path_count[parent_id][parent_g];
    state_cost_to_path_count[state_id][state_g] += parent_count;
}

void AStarCount::update_bound(int optimal_plan_cost) {
    if (quality_multiplier == numeric_limits<double>::infinity())
        return;

    int rel_bound = int(optimal_plan_cost * quality_multiplier) + 1;
    bound = min(bound, rel_bound);
}

void AStarCount::print_statistics() const {
    log << "Number of plans (exact): " << num_goal_paths << endl;
    log << "Number of plans (approx): " << scientific << setprecision(2)
        << num_goal_paths.get_d() << endl;
    if (found_solution()) {
        log << "Minimum plan cost: "
            << calculate_plan_cost(get_plan(), task_proxy) << endl;
        log << "Maximum plan cost: " << max_plan_cost << endl;
        log << "Cost bound: " << bound << endl;
    }
#ifndef HAS_GMP
    if (num_goal_paths.is_infinity()) {
        log << "Note: the number of plans is too large to be represented "
               "with the current configuration. "
               "Consider compiling with GMP support for arbitrary precision integers."
            << endl;
    }
    log << "Bytes for plan counts: " << sizeof(Count) << endl;
#else
    log << "Bytes for plan counts: "
        << (mpz_sizeinbase(num_goal_paths.get_mpz_t(), 2) + 7) / 8 << endl;
#endif
    statistics.print_detailed_statistics();
    search_space.print_statistics();
}

void AStarCount::start_f_value_statistics(EvaluationContext &eval_context) {
    int f_value = eval_context.get_evaluator_value(f_evaluator.get());
    statistics.report_f_value_progress(f_value);
}

/* TODO: HACK! This is very inefficient for simply looking up an h value.
   Also, if h values are not saved it would recompute h for each and every
   state. */
void AStarCount::update_f_value_statistics(EvaluationContext &eval_context) {
    int f_value = eval_context.get_evaluator_value(f_evaluator.get());
    statistics.report_f_value_progress(f_value);
}

enum class PlanType {
    All,
    PrefixOptimal
};

static plugins::TypedEnumPlugin<PlanType> _plan_type_enum_plugin(
    {{"all", "all paths are considered plans"},
     {"prefix_optimal",
      "only prefixes of cost-optimal paths are considered as plans"}});

class AStarCountFeature
    : public plugins::TypedFeature<SearchAlgorithm, AStarCount> {
public:
    AStarCountFeature() : TypedFeature("astar_count") {
        document_title("A*-Count search (eager)");
        document_synopsis(
            "A*-Count is a special case of A* search which count the number of solutions. "
            "We break ties using the evaluator.");

        add_option<shared_ptr<Evaluator>>("h", "heuristic");
        add_search_algorithm_options_to_feature(*this, "astar_count");

        add_option<double>(
            "quality", "Relative quality multiplier", "",
            plugins::Bounds("1.0", "infinity"));

        add_list_option<int>(
            "projected_operators",
            "Set of operator IDs to be projected (ignored) when counting plans. ",
            "[]");
        add_option<PlanType>(
            "plan_type",
            "Plan counting mode (optional). If omitted, inferred from `quality` (prefix-optimal when quality == 1.0).",
            plugins::ArgumentInfo::NO_DEFAULT);
    }

    virtual shared_ptr<AStarCount> create_component(
        const plugins::Options &opts) const override {
        vector<int> projected_operators =
            opts.get_list<int>("projected_operators");
        double quality = opts.get<double>("quality");

        // Determine plan counting mode. If the option is not provided we
        // infer it from `quality` (prefix-optimal when quality == 1.0)
        PlanType plan_type;
        if (opts.contains("plan_type")) {
            plan_type = opts.get<PlanType>("plan_type");
        } else {
            plan_type =
                quality == 1.0 ? PlanType::PrefixOptimal : PlanType::All;
        }
        bool prefix_optimal = (plan_type == PlanType::PrefixOptimal);

        auto search_arguments =
            get_search_algorithm_arguments_from_options(opts);
        int bound = get<1>(search_arguments);
        // Add cost variable for all plans
        if (!prefix_optimal) {
            tasks::add_cost_variable(*tasks::g_root_task, max(bound, 2));
        }
        return plugins::make_shared_from_arg_tuples<astar_count::AStarCount>(
            opts.get<shared_ptr<Evaluator>>("h"), search_arguments, quality,
            prefix_optimal,
            unordered_set<int>(
                projected_operators.begin(), projected_operators.end()));
    }
};

static plugins::FeaturePlugin<AStarCountFeature> _plugin;
}
