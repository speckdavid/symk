#include "helper_functions.h"
#include "operator.h"
#include "variable.h"

#include "h2_mutexes.h"
#include <cassert>
#include <iostream>
#include <fstream>
using namespace std;

Operator::Operator(istream &in, const vector<Variable *> &variables) : spurious(false) {
    check_magic(in, "begin_operator");
    in >> ws;
    getline(in, name);
    int count; // number of prevail conditions
    in >> count;
    for (int i = 0; i < count; i++) {
        int varNo, val;
        in >> varNo >> val;
        prevail.push_back(Prevail(variables[varNo], val));
    }
    in >> count; // number of pre_post conditions
    for (int i = 0; i < count; i++) {
        int eff_conds;
        vector<EffCond> ecs;
        in >> eff_conds;
        for (int j = 0; j < eff_conds; j++) {
            int var, value;
            in >> var >> value;
            ecs.push_back(EffCond(variables[var], value));
        }
        int varNo, val, newVal;
        in >> varNo >> val >> newVal;
        if (eff_conds)
            pre_post.push_back(PrePost(variables[varNo], ecs, val, newVal));
        else
            pre_post.push_back(PrePost(variables[varNo], val, newVal));
    }
    in >> cost;
    check_magic(in, "end_operator");
    // TODO: Evtl. effektiver: conditions schon sortiert einlesen?
}

void Operator::dump() const {
    cout << name << ":" << endl;
    cout << "prevail:";
    for (const auto &prev : prevail)
        cout << "  " << prev.var->get_name() << " := " << prev.prev;
    cout << endl;
    cout << "pre-post:";
    for (const auto &eff : pre_post) {
        if (eff.is_conditional_effect) {
            cout << "  if (";
            for (const auto &cond : eff.effect_conds)
                cout << cond.var->get_name() << " := " << cond.cond;
            cout << ") then";
        }
        cout << " " << eff.var->get_name() << " : "
             << eff.pre << " -> " << eff.post;
    }
    cout << endl;
}

int Operator::get_encoding_size() const {
    int size = 1 + prevail.size();
    for (const auto &eff : pre_post) {
        size += 1 + eff.effect_conds.size();
        if (eff.pre != -1)
            size += 1;
    }
    return size;
}

void Operator::strip_unimportant_effects() {
    int new_index = 0;
    for (const auto &eff : pre_post) {
        if (eff.var->get_level() != -1)
            pre_post[new_index++] = eff;
    }
    pre_post.erase(pre_post.begin() + new_index, pre_post.end());
}

bool Operator::is_redundant() const {
    return spurious || pre_post.empty();
}

void strip_operators(vector<Operator> &operators) {
    int old_count = operators.size();
    int new_index = 0;
    for (Operator &op : operators) {
        op.strip_unimportant_effects();
        if (!op.is_redundant())
            operators[new_index++] = op;
    }
    operators.erase(operators.begin() + new_index, operators.end());
    cout << operators.size() << " of " << old_count << " operators necessary." << endl;
}

void Operator::generate_cpp_input(ofstream &outfile) const {
    //TODO: beim Einlesen in search feststellen, ob leerer Operator
    outfile << "begin_operator" << endl;
    outfile << name << endl;

    outfile << prevail.size() << endl;
    for (const auto &prev : prevail) {
        assert(prev.var->get_level() != -1);
        if (prev.var->get_level() != -1)
            outfile << prev.var->get_level() << " " << prev.prev << endl;
    }

    outfile << pre_post.size() << endl;
    for (const auto &eff : pre_post) {
        assert(eff.var->get_level() != -1);
        outfile << eff.effect_conds.size();
        for (const auto &cond : eff.effect_conds)
            outfile << " " << cond.var->get_level()
                    << " " << cond.cond;
        outfile << " " << eff.var->get_level()
                << " " << eff.pre
                << " " << eff.post << endl;
    }
    outfile << cost << endl;
    outfile << "end_operator" << endl;
}

// Removes ambiguity in the preconditions,
// detects whether the operator is spurious
void Operator::remove_ambiguity(const H2Mutexes &h2) {
    if (is_redundant())
        return;
    // cout << "Check ambiguity: " << name << endl;

    vector<int> preconditions(h2.num_variables(), -1);
    vector<bool> original(h2.num_variables(), false);

    vector<bool> effect_var(h2.num_variables(), false);
    vector<pair<int, int>> effects;

    vector<pair<int, int>> known_values;

    for (const Prevail &prev : prevail) {
        int var = prev.var->get_level();
        if (var != -1) {
            preconditions[var] = prev.prev;
            known_values.push_back(make_pair(var, prev.prev));
            original[var] = true;
        }
    }
    for (const PrePost &effect : pre_post) {
        int var = effect.var->get_level();
        if (var != -1) {
            preconditions[var] = effect.pre;
            known_values.push_back(make_pair(var, effect.pre));
            original[var] = (preconditions[var] != -1);
            effect_var[var] = true;
            effects.push_back(make_pair(var, effect.post));
        }
    }
    for (const pair<int, int> &augmented_precondition : augmented_preconditions) {
        preconditions[augmented_precondition.first] = augmented_precondition.second;
        known_values.push_back(make_pair(augmented_precondition.first, augmented_precondition.second));
        original[augmented_precondition.first] = true;
    }

    // check that no precondition is unreachable or mutex with some other precondition
    for (size_t i = 0; i < preconditions.size(); i++) {
        if (preconditions[i] != -1) {
            if (h2.is_unreachable(i, preconditions[i])) {
                spurious = true;
                return;
            }
            for (size_t j = i + 1; j < preconditions.size(); j++) {
                if (h2.are_mutex(i, preconditions[i], j, preconditions[j])) {
                    spurious = true;
                    return;
                }
            }
        }
    }

    std::list<std::pair<unsigned, std::list<unsigned>>> candidates;
    for (int i = 0; i < h2.num_variables(); i++) {
        // consider unknown preconditions only
        if (preconditions[i] != -1)
            continue;

        pair<unsigned, list<unsigned>> candidate_var = make_pair(i, list<unsigned>());
        // add every reachable fluent
        for (int j = 0; j < h2.num_values(i); j++)
            candidate_var.second.push_back(j);

        candidates.push_back(candidate_var);
    }

    // actual disambiguation process
    while (!known_values.empty()) {
        vector<pair<int, int>> aux_values;
        // for each unknown variable
        for (list<pair<unsigned, list<unsigned>>>::iterator it = candidates.begin(); it != candidates.end();) {
            unsigned var = it->first;
            list<unsigned> candidate_var = it->second;
            // cout << var << " -> " << candidate_var.size() << endl;

            // we eliminate candidates mutex with other things
            for (list<unsigned>::iterator it2 = candidate_var.begin(); it2 != candidate_var.end();) {
                bool mutex = h2.is_unreachable(var, *it2);
                for (size_t k = 0; !mutex && k < known_values.size(); k++)
                    mutex = h2.are_mutex(known_values[k].first, known_values[k].second, var, *it2);

                if (!effect_var[var]) {
                    for (size_t k = 0; !mutex && k < effects.size(); k++)
                        mutex = h2.are_mutex(effects[k].first, effects[k].second, var, *it2);
                }



                if (mutex) {
                    it2 = candidate_var.erase(it2);
                } else {
                    ++it2;
                }
            }

            // we check the remaining candidates
            if (candidate_var.empty()) { // if no fluent is possible for a given variable, the operator is spurious
                spurious = true;
                return;
            } else if (candidate_var.size() == 1) { // add the single possible fluent to preconditions and aux_values and remove the variables from candidate
                pair<int, int> new_fluent = make_pair(var, candidate_var.front());
                aux_values.push_back(new_fluent);
                preconditions[new_fluent.first] = new_fluent.second;
                it = candidates.erase(it);
            } else {
                ++it;
            }
        }

        known_values.swap(aux_values);
    }


    // new preconditions are added
    for (size_t i = 0; i < preconditions.size(); i++)
        if (preconditions[i] != -1 && !original[i])
            augmented_preconditions.push_back(make_pair(i, preconditions[i]));

    // potential preconditions are set
    // important for backwards h^2
    // note: they may overlap with augmented preconditions
    potential_preconditions.clear();
    for (size_t i = 0; i < pre_post.size(); i++) {
        // for each undefined precondition
        if (pre_post[i].pre != -1)
            continue;

        int var = pre_post[i].var->get_level();
        if (preconditions[var] != -1) {
            potential_preconditions.push_back(make_pair(var, preconditions[var]));
            continue;
        }

        // for each fluent
        for (int j = 0; j < h2.num_values(var); j++) {
            bool conflict = false;
            for (size_t k = 0; !conflict && k < preconditions.size(); k++)
                if (preconditions[k] != -1)
                    conflict = h2.are_mutex(var, j, k, preconditions[k]);

            if (!conflict)
                potential_preconditions.push_back(make_pair(var, j));
        }
    }
}

void Operator::remove_unreachable_facts(const vector<Variable *> &variables) {
    vector<Prevail> newprev;
    for (Prevail &prev : prevail) {
        if (prev.var->is_necessary()) {
            prev.remove_unreachable_facts();
            newprev.push_back(prev);
        }
    }
    newprev.swap(prevail);
    for (PrePost &effect : pre_post) {
        effect.remove_unreachable_facts();
    }
    for (const pair<int, int> &augmented_precondition : augmented_preconditions) {
        int var = augmented_precondition.first;
        int val = augmented_precondition.second;
        if (variables[var]->is_necessary()) {
            augmented_preconditions_var.push_back(pair<Variable *, int> (variables[var], variables[var]->get_new_id(val)));
        }
    }
    for (const pair<int, int> &potential_precondition : potential_preconditions) {
        int var = potential_precondition.first;
        int val = potential_precondition.second;

        if (variables[var]->is_necessary()) {
            potential_preconditions_var.push_back(pair<Variable *, int> (variables[var], variables[var]->get_new_id(val)));
        }
    }
}



void Operator::include_augmented_preconditions() {
    //cout << "Including augmented precondition in" << name << endl;

    for (size_t i = 0; i < augmented_preconditions_var.size(); i++) {
        Variable *var = augmented_preconditions_var[i].first;
        int val = augmented_preconditions_var[i].second;
        //cout << name << " AFTER: " << var->get_fact_name(val) << endl;


        bool included_in_eff = false;
        for (size_t j = 0; j < pre_post.size(); j++) {
            if (pre_post[j].var->get_level() == var->get_level()) {
                if (pre_post[j].pre != -1) {
                    cerr <<
                        "Assertion error: augmented precondition was already encoded in the operator"
                         << endl;
                    cerr << name << endl;
                    exit(-1);
                } else {
                    if (pre_post[j].post != val) {
                        pre_post[j].pre = val;
                        included_in_eff = true;
                    } else {
                        //Remove prepost
                        pre_post.erase(pre_post.begin() + j);
                    }
                    break;
                }
            }
        }
        if (!included_in_eff) {
            prevail.push_back(Prevail(var, val));
        }
    }
    vector<pair<int, int>> ().swap(augmented_preconditions);
}


int Operator::count_potential_noeff_preconditions() const {
    std::set<Variable *> found, found_eff;
    for (size_t i = 0; i < potential_preconditions_var.size(); i++) {
        Variable *var = potential_preconditions_var[i].first;
        int val = potential_preconditions_var[i].second;
        bool isaug = false;
        for (size_t j = 0; j < augmented_preconditions_var.size(); j++) {
            if (augmented_preconditions_var[j].first->get_level() == var->get_level()) {
                isaug = true;
                break;
            }
        }
        if (isaug)
            continue;
        for (size_t j = 0; j < pre_post.size(); j++) {
            if (pre_post[j].var->get_level() == var->get_level()) {
                if (pre_post[j].post == val) {
                    found_eff.insert(var);
                } else {
                    found.insert(var);
                }
                break;
            }
        }
    }
    for (std::set<Variable *>::iterator it = found_eff.begin();
         it != found_eff.end(); ++it)
        found.erase(*it);


    return found.size();
}


int Operator::count_potential_preconditions() const {
    std::set<Variable *> found;
    for (size_t i = 0; i < potential_preconditions_var.size(); i++) {
        Variable *var = potential_preconditions_var[i].first;
        bool isaug = false;
        for (size_t j = 0; j < augmented_preconditions_var.size(); j++) {
            if (augmented_preconditions_var[j].first->get_level() == var->get_level()) {
                isaug = true;
                break;
            }
        }
        if (isaug)
            continue;

        found.insert(var);
    }

    return found.size();
}
