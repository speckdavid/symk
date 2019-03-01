#include "bdd_sylvan.h"
#include "bdd_tmpl.h"
#include <iostream>
#include "../../options/option_parser.h"

#ifdef DDLIBSYLVAN

namespace symbolic
{

utils::Timer Bdd::timer;
double Bdd::time_limit = std::numeric_limits<double>::infinity();
int Bdd::workers = 1;
bool Bdd::parallel_search = false;

void Bdd::add_options_to_parser(options::OptionParser &parser)
{
    parser.add_option<int>("bdd_workers", "number of workers", "1");
    parser.add_option<bool>("parallel_search", "parallel search", "false");
}

void Bdd::parse_options(const options::Options &opts) {
    workers = opts.get<int>("bdd_workers");
    parallel_search = opts.get<bool>("parallel_search");
}

bool Bdd::is_parallel_search() {
    return parallel_search;
}

Bdd Bdd::BddZero() { return Bdd(sylvan::Bdd::bddZero()); }

Bdd Bdd::BddOne() { return Bdd(sylvan::Bdd::bddOne()); }

Bdd Bdd::BddVar(int i) { return Bdd(sylvan::Bdd::bddVar(i)); }

void Bdd::initalize_manager(unsigned int /*numVars*/, unsigned int /*numVarsZ*/,
                            unsigned int /*numSlots*/,
                            unsigned int /*cacheSize*/,
                            unsigned long /*maxMemory*/)
{
    std::cout << "DD-lib: Sylvan with " << workers  << " workers" << std::endl;
    size_t deque_size =
        1500000; // default value for the size of task deques for the workers
    size_t program_stack_size = 0; // default value for the program stack of each pthread

    // Initialize the Lace framework for <n_workers> workers.
    lace_init(workers, deque_size);

    // Spawn and start all worker pthreads; suspends current thread until done.
    lace_startup(program_stack_size, NULL, NULL);

    //sylvan::sylvan_set_sizes(1LL << 24, 1LL << 26, 1LL << 24, 1LL << 25); //~3.6 GB
    sylvan::sylvan_set_sizes(1LL<<20, 1LL<<26, 1LL<<18, 1LL<<24); //~3.6 GB
    // sylvan::sylvan_set_sizes(1LL<<20, 1LL<<28, 1LL<<18, 1LL<<26); //~3.6 GB
    sylvan::sylvan_init_package();

    // Initialize the BDD module with granularity 1 (cache every operation)
    // A higher granularity (e.g. 6) often results in better performance in
    // practice
    sylvan::sylvan_init_bdd();
}

void Bdd::unset_time_limit() {
    time_limit = std::numeric_limits<double>::infinity();
}

void Bdd::reset_start_time() {
    timer.reset();
}

void Bdd::set_time_limit(unsigned long tl) {
    time_limit = tl / 1000.0;
}

Bdd::Bdd() {}

Bdd::Bdd(const Bdd &bdd) { this->bdd = bdd.get_lib_bdd(); }

Bdd::Bdd(sylvan::Bdd bdd) { this->bdd = bdd; }

sylvan::Bdd Bdd::get_lib_bdd() const { return bdd; }

Bdd Bdd::operator=(const Bdd &right) { return Bdd(bdd = right.get_lib_bdd()); }

bool Bdd::operator==(const Bdd &other) const
{
    return bdd == other.get_lib_bdd();
}

bool Bdd::operator!=(const Bdd &other) const
{
    return bdd != other.get_lib_bdd();
}

bool Bdd::operator<=(const Bdd &other) const
{
    return bdd <= other.get_lib_bdd();
}

bool Bdd::operator>=(const Bdd &other) const
{
    return bdd >= other.get_lib_bdd();
}

bool Bdd::operator<(const Bdd &other) const
{
    return bdd < other.get_lib_bdd();
}

bool Bdd::operator>(const Bdd &other) const
{
    return bdd > other.get_lib_bdd();
}

Bdd Bdd::operator!() const { 
    check_time_limit();
    return Bdd(!bdd);
}

Bdd Bdd::operator~() const { 
    check_time_limit();
    return Bdd(~bdd);
}

Bdd Bdd::operator*(const Bdd &other) const
{
    check_time_limit();
    return Bdd(bdd * other.get_lib_bdd());
}

Bdd Bdd::operator*=(const Bdd &other)
{
    check_time_limit();
    bdd *= other.get_lib_bdd();
    return *this;
}

Bdd Bdd::operator&(const Bdd &other) const
{
    check_time_limit();
    return Bdd(bdd & other.get_lib_bdd());
}

Bdd Bdd::operator&=(const Bdd &other)
{
    check_time_limit();
    bdd &= other.get_lib_bdd();
    return *this;
}

Bdd Bdd::operator+(const Bdd &other) const
{
    check_time_limit();
    return Bdd(bdd + other.get_lib_bdd());
}

Bdd Bdd::operator+=(const Bdd &other)
{
    check_time_limit();
    bdd += other.get_lib_bdd();
    return *this;
}

Bdd Bdd::operator|(const Bdd &other) const
{
    check_time_limit();
    return Bdd(bdd | other.get_lib_bdd());
}

Bdd Bdd::operator|=(const Bdd &other)
{
    check_time_limit();
    bdd |= other.get_lib_bdd();
    return *this;
}

Bdd Bdd::operator^(const Bdd &other) const
{
    check_time_limit();
    return Bdd(bdd ^ other.get_lib_bdd());
}

Bdd Bdd::operator^=(const Bdd &other)
{
    check_time_limit();
    bdd ^= other.get_lib_bdd();
    return *this;
}

Bdd Bdd::operator-(const Bdd &other) const
{
    check_time_limit();
    return Bdd(bdd - other.get_lib_bdd());
}

Bdd Bdd::operator-=(const Bdd &other)
{
    check_time_limit();
    bdd -= other.get_lib_bdd();
    return *this;
}

Bdd Bdd::Xnor(const Bdd &other) const
{
    check_time_limit();
    return Bdd(bdd.Xnor(other.get_lib_bdd()));
}

Bdd Bdd::Or(const Bdd &other, int maxNodes) const
{
    if (maxNodes > 0 && (nodeCount() > maxNodes || other.nodeCount() > maxNodes)) {
        exceptionError();
    }
    check_time_limit();
    Bdd result = Bdd(bdd.Or(other.get_lib_bdd()));
    if (maxNodes > 0 && result.nodeCount() > maxNodes)
    {
        exceptionError();
    }
    return result;
}

Bdd Bdd::And(const Bdd &other, int maxNodes) const
{
    if (maxNodes > 0 && (nodeCount() > maxNodes || other.nodeCount() > maxNodes)) {
        exceptionError();
    }
    check_time_limit();
    Bdd result = Bdd(bdd.And(other.get_lib_bdd()));
    if (maxNodes > 0 && result.nodeCount() > maxNodes)
    {
        exceptionError();
    }
    return result;
}

Bdd Bdd::SwapVariables(const std::vector<Bdd>& x, const std::vector<Bdd>& y) const
{
    check_time_limit();
    std::vector<uint32_t> x_level;
    std::vector<uint32_t> y_level;
    for (auto b : x)
    {
        x_level.push_back(b.get_lib_bdd().TopVar());
    }
    for (auto b : y)
    {
        y_level.push_back(b.get_lib_bdd().TopVar());
    }
    return Bdd(bdd.Permute(x_level, y_level));
}

int Bdd::nodeCount() const { return bdd.NodeCount(); }

bool Bdd::IsZero() const { return bdd.isZero(); }

bool Bdd::IsOne() const { return bdd.isOne(); }

Bdd Bdd::AndAbstract(const Bdd &g, const Bdd &cube, int limit) const
{
    check_time_limit();
    Bdd result = Bdd(bdd.AndAbstract(g.get_lib_bdd(), cube.get_lib_bdd()));
    if (limit > 0 && result.nodeCount() > limit)
    {
        exceptionError();
    }
    return result;
}

Bdd Bdd::RelationProductNext(const Bdd &relation, const Bdd &cube,
                             const std::vector<Bdd> &pre_vars,
                             const std::vector<Bdd> &succ_vars,
                             int max_nodes) const
{
    check_time_limit();
    Bdd tmp = AndAbstract(relation, cube, max_nodes);
    return tmp.SwapVariables(pre_vars, succ_vars);
}

Bdd Bdd::RelationProductPrev(const Bdd &relation, const Bdd &cube,
                             const std::vector<Bdd> &pre_vars,
                             const std::vector<Bdd> &succ_vars,
                             int max_nodes) const
{
    check_time_limit();
    Bdd tmp = SwapVariables(pre_vars, succ_vars);
    return tmp.AndAbstract(relation, cube, max_nodes);;
}

void Bdd::check_time_limit() const {
    if (timer() > time_limit) {
        exceptionError();
    }
}

void Bdd::toDot(const std::string &file_name) const
{
    auto file = fopen(file_name.c_str(), "w");
    bdd.PrintDot(file);
    fclose(file);
}

} // namespace symbolic

#endif
