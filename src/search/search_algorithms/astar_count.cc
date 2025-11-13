#include "astar_count.h"

#include "search_common.h"

#include "../evaluation_context.h"
#include "../open_list_factory.h"

#include "../plugins/plugin.h"
#include "../task_utils/successor_generator.h"
#include "../task_utils/task_properties.h"

using namespace std;

// TODO: If we have a relative bound and no absolut, we may get nodes in the open list which are
// to costly. So, we can stop early when the bound is exceeded. Q: Do we need tie breaking [f,g,h]
// In particular the h in the end? probably not but valuable to thing about it.

namespace astar_count {
AStarCount::AStarCount(const shared_ptr<Evaluator> &h_eval,
                       OperatorCost cost_type, int bound, double max_time,
                       const string &description, utils::Verbosity verbosity) :
    SearchAlgorithm(cost_type, bound, max_time, description, verbosity),
    num_goal_paths(0) {
    auto [open_list_factory, eval] = search_common::create_astar_count_open_list_factory_and_f_eval(h_eval, verbosity);
    open_list = open_list_factory->create_state_open_list();
    f_evaluator = eval;
}

void AStarCount::initialize() {
    log << "Conducting astar count search"
        << ", (real) bound = " << bound
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
        num_goal_paths += state_cost_to_path_count[s.get_id().get_value()][node->get_real_g()];

        for (const OperatorProxy &op: task_proxy.get_operators()) {
            utils::g_log << op.get_name() << ": " << state_cost_operator_to_path_count[s.get_id().get_value()][node->get_real_g()][op.get_id()] << endl;
            num_operator_on_goal_path[op.get_id()] +=
                state_cost_operator_to_path_count[s.get_id().get_value()][node->get_real_g()][op.get_id()];
        }

        if (!found_solution()) {
            check_goal_and_set_plan(s);
        }
        if (bound == numeric_limits<int>::max()) {
            bound = min(bound, node->get_real_g());
        }
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

        State succ_state = state_registry.get_successor_state(s, op);
        statistics.inc_generated();

        SearchNode succ_node = search_space.get_node(succ_state);

        // Previously encountered dead end. Don't re-evaluate.
        if (succ_node.is_dead_end())
            continue;

        int succ_g = node.get_g() + get_adjusted_cost(op);

        if (succ_node.is_new()) {
            EvaluationContext succ_eval_context(succ_state, succ_g, false, &statistics);
            statistics.inc_evaluated_states();

            if (open_list->is_dead_end(succ_eval_context)) {
                succ_node.mark_as_dead_end();
                statistics.inc_dead_ends();
                continue;
            }
            if (succ_eval_context.get_evaluator_value_or_infinity(f_evaluator.get()) >= bound)
                continue;
            succ_node.open_new_node(node, op, get_adjusted_cost(op));

            open_list->insert(succ_eval_context, succ_state.get_id());
            if (search_progress.check_progress(succ_eval_context))
                statistics.print_checkpoint_line(succ_node.get_g());
        } else if (succ_node.get_g() > node.get_g() + get_adjusted_cost(op)) {
            if (succ_node.is_closed()) {
                // We found a new cheapest path to a closed state.
                cerr << "Found a new cheapest path to an open closed state! "
                     << "Only consistent heuristics are supported by now!" << endl;
                utils::exit_with(utils::ExitCode::SEARCH_CRITICAL_ERROR);
            }
            assert(succ_node.is_open());
            succ_node.update_open_node_parent(node, op, get_adjusted_cost(op));
            EvaluationContext succ_eval_context(succ_state, succ_node.get_g(), false, &statistics);
            assert(succ_eval_context.get_evaluator_value_or_infinity(f_evaluator.get()) < bound);
            open_list->insert(succ_eval_context, succ_state.get_id());
        } else {
            /*
              We found an equally or more expensive path to an open or closed
              state.
            */

            // TODO: If we plan to count all the solutions beyond the optimal cost, we need to reexpand states with
            // higher costs than the lowest reachable ones. Should be possible by "reopening"
        }

        update_path_count_for_state(succ_state, s, succ_g, node.get_g(), op_id);
    }
}

optional<SearchNode> AStarCount::pop_min_node() {
    while (!open_list->empty()) {
        StateID id = open_list->remove_min();
        State s = state_registry.lookup_state(id);
        SearchNode node = search_space.get_node(s);

        if (node.is_closed()) {
            cout << s.get_id() << endl;
            continue;
        }
        return node;
    }
    return nullopt;
}

void AStarCount::update_path_count_for_state(const State &state, const State &parent,
                                             int state_g, int parent_g,
                                             const OperatorID &operator_id) {
    assert(state_g >= parent_g);

    auto state_id = state.get_id().get_value();
    auto parent_id = parent.get_id().get_value();

    // Update total path count
    int parent_count = state_cost_to_path_count[parent_id][parent_g];
    state_cost_to_path_count[state_id][state_g] += parent_count;

    // Update path counts per operator
    auto &state_op_counts = state_cost_operator_to_path_count[state_id][state_g];
    const auto &parent_op_counts = state_cost_operator_to_path_count[parent_id][parent_g];

    for (const auto &[op_id, count] : parent_op_counts) {
        state_op_counts[op_id] += count;
    }

    // Increment the count for the operator that led to this state
    state_op_counts[operator_id.get_index()] += parent_count;
}

void AStarCount::print_statistics() const {
    log << "Number of plans: " << num_goal_paths << endl;
    for (const OperatorProxy &op : task_proxy.get_operators()) {
        utils::g_log << op.get_name() << ": " << num_operator_on_goal_path.at(op.get_id()) << endl;
    }
    statistics.print_detailed_statistics();
    search_space.print_statistics();
}

void AStarCount::start_f_value_statistics(EvaluationContext &eval_context) {
    int f_value = eval_context.get_evaluator_value(f_evaluator.get());
    statistics.report_f_value_progress(f_value);
}

/* TODO: HACK! This is very inefficient for simply looking up an h value.
   Also, if h values are not saved it would recompute h for each and every state. */
void AStarCount::update_f_value_statistics(EvaluationContext &eval_context) {
    int f_value = eval_context.get_evaluator_value(f_evaluator.get());
    statistics.report_f_value_progress(f_value);
}

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
    }

    virtual shared_ptr<AStarCount>
    create_component(const plugins::Options &opts) const override {
        return plugins::make_shared_from_arg_tuples<astar_count::AStarCount>(
            opts.get<shared_ptr<Evaluator>>("h"),
            get_search_algorithm_arguments_from_options(opts)
            );
    }
};

static plugins::FeaturePlugin<AStarCountFeature> _plugin;
}
