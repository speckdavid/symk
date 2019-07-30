#include "command_line.h"
#include "option_parser.h"
#include "search_engine.h"

#include "options/registries.h"
#include "tasks/root_task.h"
#include "task_utils/task_properties.h"
#include "utils/system.h"
#include "utils/timer.h"

#include <iostream>
#include <chrono>
#include <sys/resource.h>

using namespace std;
using utils::ExitCode;

int main(int argc, const char **argv) {
    utils::register_event_handlers();

    if (argc < 2) {
        cout << usage(argv[0]) << endl;
        utils::exit_with(ExitCode::SEARCH_INPUT_ERROR);
    }

    bool unit_cost = false;
    if (static_cast<string> (argv[1]) != "--help") {
        cout << "reading input... [t=" << utils::g_timer << "]" << endl;
        tasks::read_root_task(cin);
        cout << "done reading input! [t=" << utils::g_timer << "]" << endl;
        TaskProxy task_proxy(*tasks::g_root_task);
        unit_cost = task_properties::is_unit_cost(task_proxy);
    }

    // Increase stack size for top-k planning
    // https://stackoverflow.com/questions/2275550/change-stack-size-for-a-c-application-in-linux-during-compilation-with-gnu-com
    const rlim_t kStackSize = 500 * 1024 * 1024; // min stack size = 100 MB
    struct rlimit rl;
    int result;
    result = getrlimit(RLIMIT_STACK, &rl);
    if (result == 0) {
        if (rl.rlim_cur < kStackSize) {
            rl.rlim_cur = kStackSize;
            result = setrlimit(RLIMIT_STACK, &rl);
            if (result != 0) {
                std::cerr << "setrlimit returned result = " << result << std::endl;
            } else {
                std::cout << "setrlimit (stack size) to " << kStackSize / 1024 / 1024 << "MB" << std::endl;
            }
        }
    }

    shared_ptr<SearchEngine> engine;

    // The command line is parsed twice: once in dry-run mode, to
    // check for simple input errors, and then in normal mode.
    try {
        options::Registry registry(*options::RawRegistry::instance());
        parse_cmd_line(argc, argv, registry, true, unit_cost);
        engine = parse_cmd_line(argc, argv, registry, false, unit_cost);
    }    catch (const ArgError &error) {
        error.print();
        usage(argv[0]);
        utils::exit_with(ExitCode::SEARCH_INPUT_ERROR);
    }    catch (const OptionParserError &error) {
        error.print();
        usage(argv[0]);
        utils::exit_with(ExitCode::SEARCH_INPUT_ERROR);
    }    catch (const ParseError &error) {
        error.print();
        utils::exit_with(ExitCode::SEARCH_INPUT_ERROR);
    }

    std::chrono::steady_clock::time_point wall_begin = std::chrono::steady_clock::now();
    utils::Timer search_timer;
    engine->search();
    search_timer.stop();
    std::chrono::steady_clock::time_point wall_end = std::chrono::steady_clock::now();
    utils::g_timer.stop();

    // engine->save_plan_if_necessary();
    engine->print_statistics();
    cout << "Search time: " << search_timer << endl;
    cout << "Search-Wall time: " << std::chrono::duration_cast<std::chrono::microseconds>(wall_end - wall_begin).count() / 1000000.0 << "s" << endl;
    cout << "Total time: " << utils::g_timer << endl;

    if (engine->found_solution()) {
        utils::exit_with(ExitCode::SUCCESS);
    } else {
        utils::exit_with(ExitCode::SEARCH_UNSOLVED_INCOMPLETE);
    }
}
