#include "transition_relation.h"

#include "../task_proxy.h"
#include "debug_macros.h"
#include <algorithm>
#include <cassert>

#include "original_state_space.h"
#include "sym_state_space_manager.h"

#include "../utils/timer.h"

using namespace std;

namespace symbolic
{

TransitionRelation::TransitionRelation(SymVariables *sVars, OperatorID op_id,
                                       int cost_)
    : sV(sVars), cost(cost_), tBDD(sVars->oneBDD()),
      existsVars(sVars->oneBDD()), existsBwVars(sVars->oneBDD())
{
  ops_ids.insert(op_id);
}

void TransitionRelation::init() {
  TaskProxy task_proxy(*tasks::g_root_task);
  OperatorProxy op = task_proxy.get_operators()[ops_ids.begin()->get_index()];

  for (auto const &pre : op.get_preconditions())
  { // Put precondition of label
    FactPair fact = pre.get_pair();
    tBDD *= sV->preBDD(fact.var, fact.value);
  }


  std::string op_name = op.get_name();
  std::remove(op_name.begin(), op_name.end(), ' ');

  map<int, Bdd> effect_conditions;
  map<int, Bdd> effects;

  // Get effects and the remaining conditions.
  for (auto const &eff : op.get_effects())
  {
    FactPair eff_fact = eff.get_fact().get_pair();
    int var = eff_fact.var;
    if (std::find(effVars.begin(), effVars.end(), var) == effVars.end())
    {
      effVars.push_back(var);
    }

    Bdd condition = sV->oneBDD();
    Bdd ppBDD = sV->effBDD(var, eff_fact.value);
    if (effect_conditions.count(var))
    {
      condition = effect_conditions.at(var);
    }
    else
    {
      effect_conditions[var] = condition;
      effects[var] = sV->zeroBDD();
    }

    for (const auto &cPrev : eff.get_conditions())
    {
      FactPair cPrev_cond = cPrev.get_pair();
      condition *= sV->preBDD(cPrev_cond.var, cPrev_cond.value);
    }
    effect_conditions[var] *= !condition;
    effects[var] += (condition * ppBDD);
  }

  // Add effects to the tBDD
  int counter = 0;
  for (auto it = effects.rbegin(); it != effects.rend(); ++it)
  {
    int var = it->first;
    Bdd effectBDD = it->second;
    // If some possibility is not covered by the conditions of the
    // conditional effect, then in those cases the value of the value
    // is preserved with a biimplication
    if (!effect_conditions[var].IsZero())
    {
      effectBDD += (effect_conditions[var] * sV->biimp(var));
    }
    tBDD *= effectBDD;
    counter++;
  }
  if (tBDD.IsZero())
  {
    cerr << "Operator is empty: " << op.get_name() << endl;
    // exit(0);
  }

  sort(effVars.begin(), effVars.end());
  for (int var : effVars)
  {
    for (int bdd_var : sV->vars_index_pre(var))
    {
      swapVarsS.push_back(sV->bddVar(bdd_var));
    }
    for (int bdd_var : sV->vars_index_eff(var))
    {
      swapVarsSp.push_back(sV->bddVar(bdd_var));
    }
  }
  assert(swapVarsS.size() == swapVarsSp.size());
  // existsVars/existsBwVars is just the conjunction of swapVarsS and swapVarsSp
  for (size_t i = 0; i < swapVarsS.size(); ++i)
  {
    existsVars *= swapVarsS[i];
    existsBwVars *= swapVarsSp[i];
  }
  // DEBUG_MSG(cout << "Computing tr took " << tr_timer; tBDD.print(1, 1););
  /*std::cout << op.get_name() << ":" << std::endl;
  for (auto pre : op.get_preconditions())
  {
      std::cout << " " << pre.get_pair() << std::flush;
  }
  std::cout << std::endl;
  for (auto eff : op.get_effects())
  {
      std::cout << " " << eff.get_fact().get_pair() << std::flush;
  }
  std::cout << std::endl;

  std::string op_name = op.get_name();
  std::remove(op_name.begin(), op_name.end(), ' ');
  tBDD.toDot(op_name + ".dot");*/
}

Bdd TransitionRelation::image(const Bdd &from) const
{
  Bdd from_copy = from;
  return from_copy.RelationProductNext(tBDD, existsVars, swapVarsSp, swapVarsS);
}

Bdd TransitionRelation::image(const Bdd &from, int maxNodes) const
{
  Bdd from_copy = from;
  return from_copy.RelationProductNext(tBDD, existsVars, swapVarsSp, swapVarsS,
                                       maxNodes);
}

Bdd TransitionRelation::preimage(const Bdd &from) const
{
  Bdd from_copy = from;
  return from_copy.RelationProductPrev(tBDD, existsBwVars, swapVarsS, swapVarsSp);
}

Bdd TransitionRelation::preimage(const Bdd &from, int maxNodes) const
{
  Bdd from_copy = from;
  return from_copy.RelationProductPrev(tBDD, existsBwVars, swapVarsS, swapVarsSp,
                                       maxNodes);
}

void TransitionRelation::merge(const TransitionRelation &t2, int maxNodes)
{
  assert(cost == t2.cost);
  if (cost != t2.cost)
  {
    cout << "Error: merging transitions with different cost: " << cost << " "
         << t2.cost << endl;
    utils::exit_with(utils::ExitCode::SEARCH_CRITICAL_ERROR);
  }

  //  cout << "set_union" << endl;
  // Attempt to generate the new tBDD
  vector<int> newEffVars;
  set_union(effVars.begin(), effVars.end(), t2.effVars.begin(),
            t2.effVars.end(), back_inserter(newEffVars));

  Bdd newTBDD = tBDD;
  Bdd newTBDD2 = t2.tBDD;

  //    cout << "Eff vars" << endl;
  vector<int>::const_iterator var1 = effVars.begin();
  vector<int>::const_iterator var2 = t2.effVars.begin();
  for (vector<int>::const_iterator var = newEffVars.begin();
       var != newEffVars.end(); ++var)
  {
    if (var1 == effVars.end() || *var1 != *var)
    {
      newTBDD *= sV->biimp(*var);
    }
    else
    {
      ++var1;
    }

    if (var2 == t2.effVars.end() || *var2 != *var)
    {
      newTBDD2 *= sV->biimp(*var);
    }
    else
    {
      ++var2;
    }
  }
  newTBDD = newTBDD.Or(newTBDD2, maxNodes);

  if (newTBDD.nodeCount() > maxNodes)
  {
    DEBUG_MSG(cout << "TR size exceeded: " << newTBDD.nodeCount() << ">"
                   << maxNodes << endl;);
    throw BDDError(); // We could not sucessfully merge
  }

  tBDD = newTBDD;

  effVars.swap(newEffVars);
  existsVars *= t2.existsVars;
  existsBwVars *= t2.existsBwVars;

  for (size_t i = 0; i < t2.swapVarsS.size(); i++)
  {
    if (find(swapVarsS.begin(), swapVarsS.end(), t2.swapVarsS[i]) ==
        swapVarsS.end())
    {
      swapVarsS.push_back(t2.swapVarsS[i]);
      swapVarsSp.push_back(t2.swapVarsSp[i]);
    }
  }

  ops_ids.insert(t2.ops_ids.begin(), t2.ops_ids.end());
}

// For each op, include relevant mutexes
void TransitionRelation::edeletion(
    const std::vector<std::vector<Bdd>> &notMutexBDDsByFluentFw,
    const std::vector<std::vector<Bdd>> &notMutexBDDsByFluentBw,
    const std::vector<std::vector<Bdd>> &exactlyOneBDDsByFluent)
{
  assert(ops_ids.size() == 1);
  assert(notMutexBDDsByFluentFw.size() ==
         tasks::g_root_task->get_num_variables());
  assert(notMutexBDDsByFluentBw.size() ==
         tasks::g_root_task->get_num_variables());
  assert(exactlyOneBDDsByFluent.size() ==
         tasks::g_root_task->get_num_variables());
  TaskProxy task_proxy(*tasks::g_root_task);
  // For each op, include relevant mutexes
  for (const OperatorID& op_id : ops_ids)
  {
    OperatorProxy op = task_proxy.get_operators()[op_id.get_index()];
    for (const auto &eff : op.get_effects())
    {
      FactPair pp = eff.get_fact().get_pair();
      // TODO: somehow fix this here
      FactPair pre(-1, -1);
      for (const auto &cond : op.get_preconditions())
      {
        if (cond.get_pair().var == pp.var)
        {
          pre = cond.get_pair();
          break;
        }
      }

      // edeletion bw
      if (pre.var == -1)
      {
        // We have a post effect over this variable.
        // That means that every previous value is possible
        // for each value of the variable
        for (int val = 0;
             val < tasks::g_root_task->get_variable_domain_size(pp.var);
             val++)
        {
          tBDD *= notMutexBDDsByFluentBw[pp.var][val];
        }
      }
      else
      {
        // In regression, we are making true pp.pre
        // So we must negate everything of these.
        tBDD *= notMutexBDDsByFluentBw[pp.var][pre.value];
      }
      // TODO(speckd): Here we need to swap in the correct direction!
      // edeletion fw
      tBDD *= notMutexBDDsByFluentFw[pp.var][pp.value].SwapVariables(
          swapVarsS, swapVarsSp);

      // edeletion invariants
       tBDD *= exactlyOneBDDsByFluent[pp.var][pp.value];
    }
  }
}

ostream &operator<<(std::ostream &os, const TransitionRelation &tr)
{
  os << "TR(";
  for (auto &op : tr.ops_ids)
  {
    os << tasks::g_root_task->get_operator_name(op.get_index(), false) << ", ";
  }
  return os << "): " << tr.tBDD.nodeCount() << endl;
}

} // namespace symbolic
