
// $Id$

/*
    Meddly: Multi-terminal and Edge-valued Decision Diagram LibrarY.
    Copyright (C) 2009, Iowa State University Research Foundation, Inc.

    This library is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published 
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/



#include<iostream>
#include<vector>
#include<algorithm>
#include<iterator>

using namespace std;


#define MDD 1
#define DEBUG 0
#define VAR_ORD 1
#define WES2 0
#define WES1 0

class Genome {
  public:
    static void Print(const vector<int>& v, FILE* s)
    {
      assert(s != NULL);
      if (v.empty()) return;
      vector<int>::const_iterator iter = v.begin();
      fprintf(s, "[%d", *iter++);
      for ( ; iter != v.end(); ) {
        fprintf(s, ", %d", *iter++);
      }
      fprintf(s, "]");
      fflush(s);
    }

    static void Print(const vector<double>& v, FILE* s)
    {
      assert(s != NULL);
      if (v.empty()) return;
      vector<double>::const_iterator iter = v.begin();
      fprintf(s, "[%lf", *iter++);
      for ( ; iter != v.end(); ) {
        fprintf(s, ", %lf", *iter++);
      }
      fprintf(s, "]");
      fflush(s);
    }

    Genome(int sz, int mini, int maxi, bool make_random)
      : score(0), analyzed(false), min(mini), max(maxi)
      {
        if (make_random) {
          assert((max - min + 1) == sz);
          order.resize(sz);
          Randomize();
        } else {
          order.resize(sz);
          fill(order.begin(), order.end(), min);
        }
      }
    Genome(int mini, int maxi, vector<int>& ordering)
      : score(0), analyzed(false), min(mini), max(maxi)
      {
        order = ordering;
      }
    Genome(int sz, int mini, int maxi, const int* ordering)
      : score(0), analyzed(false), min(mini), max(maxi)
      {
        order.resize(sz);
        copy(ordering, ordering + sz, order.begin());
      }
    Genome(const Genome& a)
      : order(a.order), score(0), analyzed(false), min(a.min), max(a.max) {}
    Genome& operator=(const Genome& a)
    {
      order = a.order;
      min = a.min;
      max = a.max;
      score = 0;
      analyzed = false;
      return *this;
    }
    double GetScore()
    {
      if (!analyzed) Analyze();
      return score;
    }
    void SetScore(double s)
    {
      if (!analyzed) analyzed = true;
      score = s;
    }
    void Crossover(const Genome& a, int at, Genome*& child) const
    {
      assert(at >= 0 && at < int(order.size()));
      assert(child == NULL);
#if MDD
      vector<int> child_order;
      vector<bool> missing_comp(a.order.size(), true);
      vector<int>::const_iterator citer;
      // mark comp already in child
      for (citer = order.begin(); citer != order.begin() + at; ++citer) {
        child_order.push_back(*citer);
        missing_comp[*citer] = false;
      }
      // add missing comp to child based on level ordering in a
      for (citer = a.order.begin(); citer != a.order.end(); ++citer) {
        if (missing_comp[*citer]) {
          missing_comp[*citer] = false;
          child_order.push_back(*citer);
        }
      }
      child = new Genome(min, max, child_order);
#else
      child = new Genome(*this);
      child->analyzed = false;
      copy(a.order.begin() + at, a.order.end(), child->order.begin() + at);
#endif
    }
    void Mutate(int at)
    {
#if MDD
      int temp = order[at];
      int to = random() % order.size();
      order[at] = order[to];
      order[to] = temp;
#else
      order[at] = (random() % (max - min)) + min;
#endif
      analyzed = false;
    }
    void Mutate()
    {
      Randomize();
      analyzed = false;
    }
    const vector<int>& GetOrder() const {
      return order;
    }
    void Print(FILE* s) const
    {
      Print(order, s);
    }

  protected:
    vector<int> order;
    double score;
    bool analyzed;
    int min;
    int max;

    void Analyze()
    {
#if ADD
      score = 0;
#if 0
      vector<int>::const_iterator iter = order.begin();
      for ( ; iter != order.end(); ++iter) {
        score += *iter;
      }
#else
      for (int i = 0 ; i < order.size(); ++i) {
        score += ((i % 2 == 0)? (order[i] + 1): -(order[i]) + 1);
      }
#endif
#else
      score = 1;
      for (int i = 0 ; i < order.size(); ++i) {
        if (i % 2 == 0) {
          score *= (order[i] + 1);
        } else {
          score /= (order[i] + 1);
        }
      }
#endif
      analyzed = true;
    }
    void Randomize()
    {
#if MDD
      int sz = order.size();
      vector<bool> taken(sz, false);
      int skip, comp;
      for (int i = 0; i < sz; ++i) {
        skip = random() % (sz - i);
        for (comp = 0; skip > 0; ++comp) { if (!taken[comp]) --skip; }
        while (taken[comp]) ++comp;
        taken[comp] = true;
        order[i] = comp;
      }
#else
      vector<int>::iterator iter = order.begin();
      for ( ; iter != order.end(); ++iter) {
        *iter = (random() % (max - min)) + min;
      }
#endif
    }

};


static vector<vector<int> > event;

void InitEvents()
{
  event.clear();
  event.resize(6);
  // F
  event[0].clear();
  event[0].push_back(12);
  event[0].push_back(13);
  event[0].push_back(14);
  event[0].push_back(15);
  event[0].push_back(0);
  event[0].push_back(1);
  event[0].push_back(2);
  event[0].push_back(3);
  // U
  event[1].clear();
  event[1].push_back(12);
  event[1].push_back(13);
  event[1].push_back(16);
  event[1].push_back(17);
  event[1].push_back(0);
  event[1].push_back(4);
  event[1].push_back(5);
  event[1].push_back(6);
  // R
  event[2].clear();
  event[2].push_back(13);
  event[2].push_back(14);
  event[2].push_back(17);
  event[2].push_back(18);
  event[2].push_back(1);
  event[2].push_back(6);
  event[2].push_back(7);
  event[2].push_back(8);
  // L
  event[3].clear();
  event[3].push_back(12);
  event[3].push_back(15);
  event[3].push_back(16);
  event[3].push_back(19);
  event[3].push_back(3);
  event[3].push_back(4);
  event[3].push_back(10);
  event[3].push_back(11);
  // B
  event[4].clear();
  event[4].push_back(16);
  event[4].push_back(17);
  event[4].push_back(18);
  event[4].push_back(19);
  event[4].push_back(5);
  event[4].push_back(7);
  event[4].push_back(9);
  event[4].push_back(11);
  // D
  event[5].clear();
  event[5].push_back(14);
  event[5].push_back(15);
  event[5].push_back(18);
  event[5].push_back(19);
  event[5].push_back(8);
  event[5].push_back(9);
  event[5].push_back(10);
  event[5].push_back(2);
}

class VarOrdGenome {
  public:
    // static function and variables
    static void Print(const vector<int>& v, FILE* s)
    {
      Genome::Print(v, s);
    }

    static void Print(const vector<double>& v, FILE* s)
    {
      Genome::Print(v, s);
    }

  public:
    VarOrdGenome(int sz, int mini, int maxi, bool make_random)
    : analyzed(false), score(0)
    {
      g = new Genome(sz, mini, maxi, make_random);
    }
    VarOrdGenome(int mini, int maxi, vector<int>& ordering)
    : analyzed(false), score(0)
    {
      g = new Genome(mini, maxi, ordering);
    }
    VarOrdGenome(int sz, int mini, int maxi, const int* ordering)
    : analyzed(false), score(0)
    {
      g = new Genome(sz, mini, maxi, ordering);
    }
    VarOrdGenome(const VarOrdGenome& a)
    : analyzed(false), score(0)
    {
      g = new Genome(*(a.g));
    }
    ~VarOrdGenome()
    {
      delete g;
    }

    VarOrdGenome& operator=(const VarOrdGenome& a)
    {
      if (g != NULL) delete g;
      g = new Genome(*(a.g));
      analyzed = false;
      score = 0;
      return *this;
    }
    double GetScore()
    {
      if (!analyzed) {
        // Map events (vectors of components) to the levels used
        // based on g's ordering
        const vector<int>& level_order = g->GetOrder();
        fprintf(stderr, "\n");
        VarOrdGenome::Print(level_order, stderr);
        fprintf(stderr, "\n");
        vector<int> comp_to_level(level_order.size());
        for (int i = 0; i < level_order.size(); ++i) {
          comp_to_level[level_order[i]] = i;
        }
        VarOrdGenome::Print(comp_to_level, stderr);
        fprintf(stderr, "\n");
        vector<int> top(event.size(), 0);
        vector<int> bott(event.size(), level_order.size() - 1);
        for (int i = 0; i < event.size(); ++i) {
          vector<int>& curr_event = event[i];
          for (int j = 0; j < curr_event.size(); ++j) {
            if (top[i] < comp_to_level[curr_event[j]]) {
              top[i] = comp_to_level[curr_event[j]];
            }
            if (bott[i] > comp_to_level[curr_event[j]]) {
              bott[i] = comp_to_level[curr_event[j]];
            }
          }
        }
        VarOrdGenome::Print(top, stderr);
        fprintf(stderr, "\n");
        VarOrdGenome::Print(bott, stderr);
        fprintf(stderr, "\n");
        
        // Compute score based on WES(1) metric:
        score = 0;
        for (int i = 0; i < event.size(); ++i) {
#if WES2
          score += (top[i] * top[i] * (top[i] - bott[i] + 1));
#else
#if WES1
          score += (top[i] * (top[i] - bott[i] + 1));
#else
          score += top[i] - bott[i] + 1;
#endif
#endif
        }
#if WES2
        score *= 4.0 / double(event.size() *
            level_order.size() * level_order.size() * level_order.size());
#else
#if WES1
        score *= 2.0 / double(event.size() *
            level_order.size() * level_order.size());
#else
        score /= double(event.size() * level_order.size());
#endif
#endif
        analyzed = true;
      }
      return score;
    }
    void Crossover(const VarOrdGenome& a, int at, VarOrdGenome*& child) const
    {
      assert(child == NULL);
      Genome* child_g = NULL;
      g->Crossover(*(a.g), at, child_g);
      child = new VarOrdGenome(child_g);
    }
    void Mutate(int at)
    {
      g->Mutate(at);
      analyzed = false;
    }
    void Mutate()
    {
      g->Mutate();
      analyzed = false;
    }
    void Print(FILE* s) const
    {
      g->Print(s);
    }

  private:
    VarOrdGenome(Genome* a)
    {
      g = a;
    }
    Genome *g;
    bool analyzed;
    double score;
};

template <class GENOME>
class Generation {
  public:
    Generation(int p_sz, int g_sz, int mini, int maxi)
      : pop_sz(p_sz), genome_sz(g_sz), min(mini), max(maxi)
      {
        bad_pop = 0;
        pop.clear();
        best_ind_score = INT_MAX;
        best_ind = NULL;
      }
    void Set(int index, const GENOME& g) { pop[index] = g; }
    const GENOME& Get(int index) const { return pop[index]; }
    void PopulateRandomly()
    {
      if (!pop.empty()) pop.clear();
      for (int i = 0; i < pop_sz; ++i) {
        pop.push_back(GENOME(genome_sz, min, max, true));
      }
    }
    void EvaluateFitness(vector<GENOME>& pop, vector<double>& score)
    {
      if (score.size() != pop.size()) score.resize(pop.size());
      double temp = 0;
      for (int i = pop.size() - 1 ; i >= 0; --i) {
        temp = pop[i].GetScore();
        score[i] = (temp < 0)? -temp: temp;
      }
    }
    int EvaluateFitness(double acceptable = 0)
    {
      if (score.size() != pop.size()) score.resize(pop.size());
      double temp = 0;
      double sum = 0;
      int best_i = 0;
      best_fitness = INT_MAX;
      int optimum = -1;
      for (int i = pop.size() - 1 ; i >= 0; --i) {
        temp = pop[i].GetScore();
        if (temp < 0) temp = -temp;
        score[i] = temp;
        sum += temp;
        if (temp < best_fitness) {
          best_fitness = temp;
          best_i = i;
          // if (temp == 0) optimum = i;
          if (temp <= acceptable) optimum = i;
        }
      }
      if (best_fitness < best_ind_score) {
        best_ind_score = best_fitness;
        if (best_ind != NULL) delete best_ind;
        best_ind = new GENOME(pop[best_i]);
      }
      avg_fitness = sum / pop.size();
      return optimum;
    }
    int FindInterval(const vector<int>& cdf, int val)
    {
      assert(cdf.size() > 0);
      assert(val <= cdf.back());
      if (val <= cdf[0]) return 0;
      // cdf is in ascending order
      // searching in (cdf[start], cds[stop]]
      int start = 0;
      int stop = cdf.size() - 1;
      int mid;
      while ((stop - start) > 1) {
        mid = (start + stop) / 2;
        if (val <= cdf[mid]) stop = mid; else start = mid;
      }
      return stop;
    }

    void Reproduce(double rep_prob, double mut_prob)
    {
      // simple algo:
      // determine how many offspring to produce (n)
      // fitness-appropriate selection for reproduction
      // perform crossover to produce n offspring
      // peform mutation (produces m offspring)
      // remove (n + m) of the worst individuals in the prev gen
      // add new offspring

      assert(pop_sz == pop.size());
      assert(pop_sz == score.size());

      vector<GENOME> offspring;
      vector<double> pop_score(score);

      // create a probability vector to help pick parents
      // -- cdf for fitness-appropriate selection

      double new_sum = 0;
      double sum = avg_fitness * pop_sz;
      vector<double>::iterator iter_score = score.begin();
      for ( ; iter_score != score.end(); ++iter_score) {
        assert(*iter_score > 0);
        // modify so that lower scores are better
        *iter_score = sum / *iter_score;
        new_sum += *iter_score;
      }

      // normalize and create cdf
      int cumulative_sum = 0;
      vector<int> cdf;
      double normalizer = 1.0 / new_sum;
      double mult = double(INT_MAX) * normalizer;
      iter_score = score.begin();
      for ( ; iter_score != score.end(); ++iter_score) {
        cumulative_sum += int(*iter_score * mult);
        cdf.push_back(cumulative_sum);
      }
      if (cumulative_sum < INT_MAX) {
        // rounding error; fix it
        cdf.back() = INT_MAX;
      }
#if DEBUG
      GENOME::Print(cdf, stderr);
      fprintf(stderr, "\n");
#endif

      // crossover
      int p1, p2;
      int int_rep_prob = int(rep_prob * INT_MAX);
      int int_mut_prob = int(mut_prob * INT_MAX); // default 0.6%
      if (int_rep_prob < 0) int_rep_prob = INT_MAX;
      if (int_mut_prob < 0) int_mut_prob = INT_MAX;
      for (int i = 0; i < pop_sz; i++) {
        if (random() > int_rep_prob) continue;
        p1 = p2 = 0;
        p1 = FindInterval(cdf, random());
        p2 = FindInterval(cdf, random());
        if (p1 == p2) continue;
#if DEBUG
        fprintf(stderr, "Crossing: %d %d\n", p1, p2);
#endif
        GENOME *child = NULL;
        pop[p1].Crossover(pop[p2], random() % genome_sz, child);
        for (int j = 0; j < genome_sz; ++j) {
          if (random() < int_mut_prob) child->Mutate(j);
        }
        offspring.push_back(*child);
        delete child;
      }

/*
      int mutations = Mutate(offspring, mut_prob);
#if DEBUG      
      fprintf(stderr, "Mutated %d\n", mutations);
      fflush(stderr);
#endif
*/

#if 0
      if (offspring.size() < pop_sz) {
        for (int i = pop_sz - offspring.size(); i > 0; --i) {
          offspring.push_back(GENOME(genome_sz, min, max, true));
        }
      }
#endif

      vector<double> offspring_score;
      EvaluateFitness(offspring, offspring_score);

      // append offspring to pop vector
      score = pop_score;
      while (!offspring.empty()) {
        pop.push_back(offspring.back());
        score.push_back(offspring_score.back());
        offspring.pop_back();
        offspring_score.pop_back();
      }

      // need to select between offspring and parents
      // -- determine probability of selecting each
      // -- select pop size

      pop_score = score;
      sum = 0;
      iter_score = score.begin();
      for ( ; iter_score != score.end(); ++iter_score) {
        sum += *iter_score;
      }
      new_sum = 0;
      iter_score = score.begin();
      for ( ; iter_score != score.end(); ++iter_score) {
        // modify so that lower scores are better
        *iter_score = sum / *iter_score;
        new_sum += *iter_score;
      }

      // normalize and create pdf
      // using vector cdf as a pdf
      cdf.clear();
      normalizer = 1.0 / new_sum;
      mult = double(INT_MAX) * normalizer;
      iter_score = score.begin();
      for ( ; iter_score != score.end(); ++iter_score) {
        cdf.push_back(int(*iter_score * mult));
      }
#if DEBUG
      GENOME::Print(cdf, stderr);
      fprintf(stderr, "\n");
#endif

      vector<GENOME> selection(pop);
      pop.clear();
      do {
        for (int i = selection.size() - 1; i >= 0; --i) {
          // starting from end since that's where the offspring are
          if (random() < cdf[i]) {
            pop.push_back(selection[i]);
            cdf[i] = 0;
            if (pop.size() >= pop_sz) break;
          }
        }
      } while (pop.size() < pop_sz);
      selection.clear();
    }


    int Mutate(vector<GENOME>& gen, double mut_prob = 0.006)
    {
      const int p = int(mut_prob * double(INT_MAX)); // default 0.6%
      int count = 0;
      for (int i = 0; i < gen.size(); ++i) {
        for (int j = 0; j < genome_sz; ++j) {
          if (random() <= p) {
            // mutate
            count++;
            gen[i].Mutate(random() % genome_sz);
          }
        }
      }
      return count;
    }

    void Print(FILE* s, bool verbose = true)
    {
      EvaluateFitness();
      if (verbose) {
        int sz = pop.size();
        for (int i = 0; i < sz; ++i) {
          fprintf(s, "%d: score %1.3e, ", i, score[i]);
          pop[i].Print(s);
          fprintf(s, "\n");
        }
      }
      fprintf(s, "Average Fitness: %1.3e\n", avg_fitness);
      fprintf(s, "Best Fitness: %1.3e\n", best_fitness);
      if (best_ind != NULL) {
        fprintf(s, "Best so far: %1.3e\n", best_ind_score);
        best_ind->Print(s);
        fprintf(s, "\n");
      }
      fprintf(s, "Bad Populations: %d\n", bad_pop);
      fflush(s);
    }
  private:
    vector<GENOME> pop;
    vector<double> score;
    int pop_sz;
    int genome_sz;
    int min;
    int max;
    double avg_fitness;
    double best_fitness;
    int bad_pop;
    GENOME* best_ind;
    double best_ind_score;
};

int main(int argc, char* argv[])
{
  int pop_sz = 10;
  int genome_sz = 5;
  int min = -20;
  int max = 20;
  int iterations = 10;
  double mut_prob = 0.0006;
  double rep_prob = 0.6;
  double acceptable_err = 0;

  for (int i = 0; i < argc; ++i) {
    char *cmd = argv[i];
    if (strncmp(cmd, "-pn", 3) == 0) {
      pop_sz = strtol(&cmd[3], NULL, 10);
    } else if (strncmp(cmd, "-gn", 3) == 0) {
      genome_sz = strtol(&cmd[3], NULL, 10);
    } else if (strncmp(cmd, "-h", 2) == 0) {
      max = strtol(&cmd[2], NULL, 10);
    } else if (strncmp(cmd, "-l", 2) == 0) {
      min = strtol(&cmd[2], NULL, 10);
    } else if (strncmp(cmd, "-i", 2) == 0) {
      iterations = strtol(&cmd[2], NULL, 10);
    } else if (strncmp(cmd, "-r", 2) == 0) {
      rep_prob = strtod(&cmd[2], NULL);
    } else if (strncmp(cmd, "-m", 2) == 0) {
      mut_prob = strtod(&cmd[2], NULL);
    } else if (strncmp(cmd, "-a", 2) == 0) {
      acceptable_err = strtod(&cmd[2], NULL);
    }
  }

#if VAR_ORD
  InitEvents();

  vector<int> init_ordering(genome_sz);
  for (int i = 0; i < genome_sz; ++i) init_ordering[i] = genome_sz - 1 - i;
  VarOrdGenome test(min, max, init_ordering);
  test.Print(stderr);
  fprintf(stderr, "\nScore: %1.3e\n", test.GetScore());
  exit(1);

  Generation<VarOrdGenome> gen0(pop_sz, genome_sz, min, max);
#else
  Generation<Genome> gen0(pop_sz, genome_sz, min, max);
#endif
  gen0.PopulateRandomly();
  int index = gen0.EvaluateFitness(acceptable_err);
  if (index != -1) {
    // found optimum!
    fprintf(stderr, "Found solution: ");
    (gen0.Get(index)).Print(stderr);
    fprintf(stderr, "\n");
    gen0.Print(stderr);
    return 0;
  }
  gen0.Print(stderr);
  for (int i = 1; i < iterations; ++i) {
    gen0.Reproduce(rep_prob, mut_prob);
    index = gen0.EvaluateFitness(acceptable_err);
    if (index != -1) {
      // found optimum!
      fprintf(stderr, "Found solution in %d iterations:\n", i);
      (gen0.Get(index)).Print(stderr);
      fprintf(stderr, "\n");
      gen0.Print(stderr);
      return 0;
    } else {
      // gen0.Print(stderr, false);
    }
  }
  fprintf(stderr, "After %d iterations:\n", iterations);
  gen0.Print(stderr);
  return 0;
}


