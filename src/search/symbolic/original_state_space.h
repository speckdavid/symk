#ifndef SYMBOLIC_ORIGINAL_STATE_SPACE_H
#define SYMBOLIC_ORIGINAL_STATE_SPACE_H

#include "../mutex_group.h"
#include "sym_state_space_manager.h"

namespace symbolic {
class OriginalStateSpace : public SymStateSpaceManager {

  void create_single_trs_sequential();
  void create_single_trs_parallel();
  void
  create_single_trs_parallel_recursive(std::vector<TransitionRelation> &trs,
                                       int i, int k);
  void init_mutex(const std::vector<MutexGroup> &mutex_groups);
  void init_mutex(const std::vector<MutexGroup> &mutex_groups, bool genMutexBDD,
                  bool genMutexBDDByFluent, bool fw);

public:
  OriginalStateSpace(SymVariables *v, const SymParamsMgr &params);

  // Individual TRs: Useful for shrink and plan construction
  std::map<int, std::vector<TransitionRelation>> indTRs;

  // notMutex relative for each fluent
  std::vector<std::vector<Bdd>> notMutexBDDsByFluentFw, notMutexBDDsByFluentBw;
  std::vector<std::vector<Bdd>> exactlyOneBDDsByFluent;

  virtual std::string tag() const override { return "original"; }

  // Methods that require of mutex initialized
  inline const Bdd &getNotMutexBDDFw(int var, int val) const {
    return notMutexBDDsByFluentFw[var][val];
  }

  // Methods that require of mutex initialized
  inline const Bdd &getNotMutexBDDBw(int var, int val) const {
    return notMutexBDDsByFluentBw[var][val];
  }

  // Methods that require of mutex initialized
  inline const Bdd &getExactlyOneBDD(int var, int val) const {
    return exactlyOneBDDsByFluent[var][val];
  }

  virtual const std::map<int, std::vector<TransitionRelation>> &
  getIndividualTRs() const {
    return indTRs;
  }
};
} // namespace symbolic
#endif