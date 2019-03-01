
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



/*
  Generation and evaluation of next-state function for a 3x3 Rubik's Cube
  -- Junaid Babar

	The model has 6 square faces with 9 squares (of the same color) in each face
	for a total of 54 squares.
  
	These 54 squares can be grouped together as some of them move together.

	Type 1: Single square (6 such squares)  ,  6 * 1 =  6
	Type 2: Two squares (12 such L-corners) , 12 * 2 = 24
	Type 3: Three squares (8 such corners)  ,  8 * 3 = 24
	
	The model thus has 26 components and 26 component locations.
	Each component location is represented as a MDD level. The size of the MDD
	level depends on the number of components that can fit into that location.
	For example, Type 2 component locations are of size 12.
  
	Level 0        : Terminals
	Level 01 - 12  : Type 2 component locations (size 12)
	Level 13 - 20  : Type 3 component locations (size 8)

  The 6 Type 1 component locations (size 6) are not represented since they
  never move.	Previously at levels 21 - 26.

	Up:     In order (going right starting from front face left-upper corner)
          of components (Type:id),
          (3:0, 2:0, 3:1, 2:1, 3:2, 2:2, 3:3, 2:3)
          Note: (1:0) also belongs to this row but since it stays in the
          same location when this row is moved left or right, it is ignored.
	Down:   (3:4, 2:8, 3:5, 2:9, 3:6, 2:10, 3:7, 2:11) (1:5 ignored)
	Left:   (3:0, 2:4, 3:4, 2:11, 3:7, 2:7, 3:3, 2:3) (1:4 ignored)
	Right:  (3:1, 2:5, 3:5, 2:9, 3:6, 2:6, 3:2, 2:1) (1:2 ignored)
	Front:  (3:0, 2:0, 3:1, 2:5, 3:5, 2:8, 3:4, 2:4) (1:1 ignored)
	Back:   (3:3, 2:2, 3:2, 2:6, 3:6, 2:10, 3:7, 2:7) (1:3 ignored)

	Initially components are placed in components locations that match their
	Ids.
*/

#define FROM_TO 0

#include <iostream>
#include <unistd.h>             // fork, pipe, close
#include <signal.h>             // signal
#include <sys/types.h>          // waitpid -- why?
#include <sys/wait.h>           // waitpid
#include <sys/time.h>           // struct timeval -- helps portability
#include <sys/resource.h>       // getrusage

#include <domain.h>
#include <forest.h>
#include <dd_edge.h>
#include <ophandle.h>


// Faces on the Rubik's cube
typedef enum {F, B, L, R, U, D} face;
// Types of possible moves
typedef enum {CW, CCW, FLIP} direction;

///////////////////////////////////
// Global variables

// Determines verbosity of output
int debug_level = -1;

// Store level handles
level *variables = NULL;
int *sizes = NULL;

// Number of variables of each type
const int type1 = 6;
const int type2 = 12;
const int type3 = 8;

// Number of levels
const int num_levels = type2 + type3;

// Domain handle
domain *d;

// Forest storing the next state function
forest_hndl relation;

// Forest storing the set of states
forest_hndl states;

// Edge representing the initial set of states
dd_edge *initial;

// Edge representing the next state function
dd_edge *nsf;

// Mapping components to the levels they are stored at and vice-versa
int component_map[] =
  {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
int level_map[] =
  {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};

// Which moves should we include in the next-state function
// rotate faces clockwise
bool enable_F, enable_U, enable_R, enable_L, enable_B, enable_D;
// rotate faces counter-clockwise
bool enable_f, enable_u, enable_r, enable_l, enable_b, enable_d;
// flip
bool flip, union_flip;

///////////////////////////////////
// Utilities for accessing variable-ordering

int get_component_type (int comp_level);
int get_component_size (int comp_type);
int getLevelSize (int comp_level);
int get_component (int comp_type, int index);
int get_component_level (int comp);
void set_component_level (int comp, int level);
int get_level_component (int level);
void set_level_component (int level, int comp);
int get_component_level (int comp_type, int index);


///////////////////////////////////
// Utilities for initializing level info

void InitMemory();
void CheckVars();
void SetLevelSizes();
void ShowLevelInfo();


///////////////////////////////////
// Utilities for maniplulating the variable ordering

void SwapComponents(int comp1, int comp2);
void SwapComponents(int n1_type, int n1_index, int n2_type, int n2_index);
void SwapLevels(int l1, int l2);
void RandomizeLevels(int n);


///////////////////////////////////
// Utilities for builing the next-state function

void InitForest();
void BuildNSF();
void CleanUpForest();

// Build the nsf and find its score(s)
void AnalyzeNSF(double *score, double *wt_score, int sleep_internal);

dd_edge* DoMoveHelper(int a3, int b3, int c3, int d3,
    int a1, int b1, int c1, int d1);
dd_edge* DoFlipHelper(int a3, int b3, int c3, int d3,
    int a1, int b1, int c1, int d1);
dd_edge* DoMove(face f, direction d);

dd_edge* Front(direction dir);
dd_edge* Back(direction dir);
dd_edge* Left(direction dir);
dd_edge* Right(direction dir);
dd_edge* Up(direction dir);
dd_edge* Down(direction dir);


///////////////////////////////////
// Reading and writing between processes

int fd[2];
const size_t buf_sz = 100;
char buf[buf_sz];
void swab_buffer ();
void terminate (int param);
void write_score (double score, double wt_score);
void read_score (double *score, double *wt_score);

/////////////////////////////////


void usage() {
  fprintf(stderr, "Usage: rubik_cube [-m <MB>|-d<int>|-n<int>|-N<int>|-s<int>|-f|-F] -l<int>\n");
  fprintf(stderr, "-m<MB>  : sets the total memory to be used (in MB)\n");
  fprintf(stderr, "-d<int> : debug level (0: silent, 1:verbose, 2:very verbose)\n");
  fprintf(stderr, "-n<int> : number of random variable orderings to generate\n");
  fprintf(stderr, "-N<int> : # swaps to generate a random variable orderings\n");
  fprintf(stderr, "-s<int> : sleep interval in seconds\n");
  fprintf(stderr, "-f      : flip selected sides (same as two cw turns)\n");
  fprintf(stderr, "-F      : 'f', 'u', etc. represent both ccw and flip\n");
  fprintf(stderr, "-l<str> : CW: F, U, R, L, B, D; CCW: small letters\n");
  fprintf(stderr, "Example:\n");
  fprintf(stderr, "$ ./rubiks_cube_ordering -m500 -d0 -s2 -F -lFURLBDfurlbd\n");
  fprintf(stderr, "\n");
}

int main(int argc, char *argv[])
{
  int iterations = 0;
  int sleep_interval = 1;
  int num_swaps = 5;

  enable_F = enable_U = enable_R = enable_L = enable_B = enable_D = false;
  enable_f = enable_u = enable_r = enable_l = enable_b = enable_d = false;
  flip = union_flip = false;

  bool no_moves = true;

  if (argc > 7) { usage(); exit(1); }
  for (int i=1; i<argc; i++) {
    char *cmd = argv[i];
    if (strncmp(cmd, "-f", 3) == 0) flip = true;
    else if (strncmp(cmd, "-F", 3) == 0) union_flip = true;
    else if (strncmp(cmd, "-d0", 4) == 0) debug_level = 0;
    else if (strncmp(cmd, "-d1", 4) == 0) debug_level = 1;
    else if (strncmp(cmd, "-d2", 4) == 0) debug_level = 2;
    else if (strncmp(cmd, "-l", 2) == 0) {
      for (unsigned j = 2; j < strlen(cmd); j++) {
        switch (cmd[j]) {
          case 'F': enable_F = true; no_moves = false; break;
          case 'U': enable_U = true; no_moves = false; break;
          case 'R': enable_R = true; no_moves = false; break;
          case 'L': enable_L = true; no_moves = false; break;
          case 'B': enable_B = true; no_moves = false; break;
          case 'D': enable_D = true; no_moves = false; break;
          case 'f': enable_f = true; no_moves = false; break;
          case 'u': enable_u = true; no_moves = false; break;
          case 'r': enable_r = true; no_moves = false; break;
          case 'l': enable_l = true; no_moves = false; break;
          case 'b': enable_b = true; no_moves = false; break;
          case 'd': enable_d = true; no_moves = false; break;
        }
      }
    } else if (strncmp(cmd, "-m", 2) == 0) {
      int mem_total = strtol(&cmd[2], NULL, 10);
      if (mem_total < 1 || mem_total > 100*1024) { // 10 GB!
        usage();
        exit(1);
      }
      // set up memory available
      InitMemoryManager(mem_total*1024*1024);
    } else if (strncmp(cmd, "-n", 2) == 0) {
      iterations = strtol(&cmd[2], NULL, 10);
    } else if (strncmp(cmd, "-N", 2) == 0) {
      num_swaps = strtol(&cmd[2], NULL, 10);
    } else if (strncmp(cmd, "-s", 2) == 0) {
      sleep_interval = strtol(&cmd[2], NULL, 10);
    } else {
      usage();
      exit(1);
    }
  }
  if (no_moves) { usage(); exit(1); }

  initialize();

  bool random_search = (iterations > 0);
  bool GA = false;

  // set up arrays based on number of levels
  InitMemory();

  if (random_search) {
    if (GA) {
#if 0
      class Genome {
        public:
          Genome(int sz)
          {
            order.resize(sz);
            fill_n(order.begin(), sz, 0);
          }
          Genome(int sz, const int* ordering)
          {
            order.resize(sz);
            copy_n(ordering, sz, order.begin());
          }
          Genome(const Genome& a) : order(a.order) {}
          Genome& operator=(const Genome& a)
          {
            order = a.order;
            return *this;
          }
          double GetScore() const { return score; }
          double GetWtScore() const { return wt_score; }
          void SetScore(double score) { this->score = score; }
          void SetWtScore(double wt_score) { this->wt_score = wt_score; }
          void Print(FILE *s) const
          {
            fprintf(s, "[%lf", order[0]);
            for (int i = 1; i < order.size(); i++) {
              fprintf(s, " ,%lf", order[i]);
            }
            fprintf(s, "]\n");
            fflush(s);
          }
          void ReproduceAt(const Genome& a, int at, Genome& *b1, Genome& *b2)
          {
            assert(a >= 0);
            assert(a < order.size());
            assert(b1 == NULL && b2 == NULL);
            // Produce first child
            vector<bool> available(num_levels, true);
            b1 = new Genome(order.size());
            int rn = 0;
            for (int i = 0; i < b1.size(); i++) {
              rn = random() % 2;
              if (rn == 0) {
                // use Genome *this
===>                if (available[order[i]])
              } else {
                // use Genome a
              }
            }
            
            b2 = new Genome(order.size());
          }

        private:
          vector<int> order;
          double score;
          double wt_score;
      };
      class Generation {
        public:
          Generation(int pop_sz, int genome_sz) :
          this->pop_sz(pop_sz), this->genome_sz(genome_sz)
          {
            pop.resize(pop_sz);
            for (int i = 0; i < pop.size(); i++) {
              pop[i] = new Genome(genome_sz);
            }
          }
          Generation(const Generation& a) :
          pop_sz(a.pop_sz), genome_sz(a.genome_sz)
          {
            pop.resize(pop_sz);
            for (int i = 0; i < pop.size(); i++) {
              pop[i] = new Genome(*(a.pop[i]));
            }
          }
          Generation& operator=(const Generation& a)
          {
            pop_sz = a.pop_sz;
            genome_sz = a.genome_sz;
            pop.resize(pop_sz);
            for (int i = 0; i < pop.size(); i++) {
              pop[i] = new Genome(*(a.pop[i]));
            }
            return *this;
          }
        private:
          vector<Genome* > pop;
          int pop_sz;
          int genome_sz;
      }
      const int pop_size = 10;
      Genome **curr_gen = new (Genome*)[pop_size];


      // generate initial population
      for (int i = 0; i < sz; i++) {
        RandomizeLevels(num_levels/2);
        pop[i].
      }
#endif
    } else {
      double score, wt_score;
      for (int i = 0; i < iterations; i++) {
        score = wt_score = 0;
        AnalyzeNSF(&score, &wt_score, sleep_interval);
        if (score > 0) {
          ShowLevelInfo();
          fprintf(stderr,
              "NSF nodes: score %1.6e, weighted score %1.6e\n",
              score, wt_score);
          fflush(stderr);
        }
        RandomizeLevels(num_swaps);
      }
    }
  } else {
    // directed search
    // calculate score for initial_state (curr_state)
    // determine next_state
    // if next_state has not been visited
    //   best_state = curr_state
    //   best_score = curr_score
    //   curr_state = next_state
    //   determine curr_score
    //   if curr_score is better than prev_score
    //     update best_state and best_score
    // determine next state and repeat
    // if no more next states exist, stop
    double best_score, curr_score;
    double best_wt_score, curr_wt_score;
    best_score = curr_score = best_wt_score = curr_wt_score = 0;

    // do once to set the bar

    SetLevelSizes();
    InitForest();
    BuildNSF();
    best_score = NodeCount(nsf);
    best_wt_score = WeightedNodeCount(nsf);
    assert (best_score != -1 && best_wt_score != -1);
    ShowLevelInfo();
    fprintf(stderr,
        "NSF nodes: score %1.6e, weighted score %1.6e\n",
        best_score, best_wt_score);
    fflush(stderr);
    CleanUpForest();

    for (int i = 1; i <= num_levels; i++) {
      int best_j = i;
      for (int j = 1; j <= num_levels; j++) {
        if (i == j) continue;

        // SwapComponents(i, j);
        SwapLevels(i, j);

        AnalyzeNSF(&curr_score, &curr_wt_score, sleep_interval);

        if (curr_score > 0) {
          ShowLevelInfo();
          fprintf(stderr,
              "NSF nodes: score %1.6e, weighted score %1.6e",
              curr_score, curr_wt_score);
          if (curr_score < best_score ||
              (curr_score == best_score && curr_wt_score < best_wt_score)) {
            // save this ordering; best so far
            best_j = j;
            best_score = curr_score;
            best_wt_score = curr_wt_score;
            fprintf(stderr, " -- best score");
          }
          fprintf(stderr, "\n");
          fflush(stderr);
        }
        // swap back for next iteration
        // SwapComponents(i, j);
        SwapLevels(i, j);
      }
      if (i != best_j) {
        // SwapComponents(i, best_j);
        SwapLevels(i, best_j);
      }
    }
  }

  if (debug_level > 1) {
    fprintf(stderr, "\n\nDONE\n");
  }
  cleanup();
  return 0;
}


void AnalyzeNSF(double *score, double *wt_score, int sleep_interval)
{
  SetLevelSizes();

  swab_buffer();
  *score = 0;
  *wt_score = 0;

  pipe(fd);
  pid_t pid = fork();
  assert(pid != -1);

  if (pid == 0) {
    void (*prev_fn)(int);
    prev_fn = signal (SIGTERM, terminate);
    assert (prev_fn != SIG_ERR);

    // child process
    close(fd[0]);

    // Create domain and forest for the given variable-ordering
    InitForest();

    // Build next-state function in this domain
    BuildNSF();

    // Analyze the next-state function
    if (debug_level > 0) {
      fprintf(stderr, "Created next-state function\n");
    }
    *score = NodeCount(nsf);
    *wt_score = WeightedNodeCount(nsf);
    assert (*score != -1);
    if (debug_level > 0) {
      fprintf(stderr,
          "(Child) NSF nodes: score %1.6e, weighted score %1.6e\n",
          *score, *wt_score);
      fflush(stderr);
    }

    write_score(*score, *wt_score);
    close(fd[1]);

    // Discard this domain and its forests
    CleanUpForest();

    // done with child, exit
    exit(0);
  }

  // parent process
  close(fd[1]);

  // sleep
  sleep(sleep_interval);
  // if child process is not yet done, end it
  kill(pid, SIGTERM);
  read_score(score, wt_score);
  waitpid(pid, NULL, 0);
}


void BuildNSF() {
  // Build transitions
  const int num_moves = 6;
  dd_edge* move[num_moves][2];

  move[F][CW] = NULL;
  move[B][CW] = NULL;
  move[L][CW] = NULL;
  move[R][CW] = NULL;
  move[U][CW] = NULL;
  move[D][CW] = NULL;
  move[F][CCW] = NULL;
  move[B][CCW] = NULL;
  move[L][CCW] = NULL;
  move[R][CCW] = NULL;
  move[U][CCW] = NULL;
  move[D][CCW] = NULL;

  // Build overall next-state function
  nsf = NULL;

  if (enable_F) {
    move[F][CW] = Front(CW);
    if (nsf == NULL) {
      nsf = move[F][CW];
    } else {
      assert(SUCCESS == 
          ApplyBinary(OP_UNION, nsf, move[F][CW], nsf));
    }
    if (debug_level > 0) {
      fprintf(stderr, "Union f-cw: ");
      fflush(stderr);
    }
  }

  if (enable_U) {
    move[U][CW] = Up(CW);
    if (nsf == NULL) {
      nsf = move[U][CW];
    } else {
      assert(SUCCESS == 
          ApplyBinary(OP_UNION, nsf, move[U][CW], nsf));
    }
    if (debug_level > 0) {
      fprintf(stderr, "Union u-cw: ");
      fflush(stderr);
    }
  }

  if (enable_R) {
    move[R][CW] = Right(CW);
    if (nsf == NULL) {
      nsf = move[R][CW];
    } else {
      assert(SUCCESS == 
          ApplyBinary(OP_UNION, nsf, move[R][CW], nsf));
    }
    if (debug_level > 0) {
      fprintf(stderr, "Union r-cw: ");
      fflush(stderr);
    }
  }

  if (enable_L) {
    move[L][CW] = Left(CW);
    if (nsf == NULL) {
      nsf = move[L][CW];
    } else {
      assert(SUCCESS == 
          ApplyBinary(OP_UNION, nsf, move[L][CW], nsf));
    }
    if (debug_level > 0) {
      fprintf(stderr, "Union l-cw: ");
      fflush(stderr);
    }
  }

  if (enable_B) {
    move[B][CW] = Back(CW);
    if (nsf == NULL) {
      nsf = move[B][CW];
    } else {
      assert(SUCCESS == 
          ApplyBinary(OP_UNION, nsf, move[B][CW], nsf));
    }
    if (debug_level > 0) {
      fprintf(stderr, "Union b-cw: ");
      fflush(stderr);
    }
  }

  if (enable_D) {
    move[D][CW] = Down(CW);
    if (nsf == NULL) {
      nsf = move[D][CW];
    } else {
      assert(SUCCESS == 
          ApplyBinary(OP_UNION, nsf, move[D][CW], nsf));
    }
    if (debug_level > 0) {
      fprintf(stderr, "Union d-cw: ");
      fflush(stderr);
    }
  }

  direction dir = (flip)? FLIP: CCW;
  dd_edge *nsf1 = NULL;
  if (enable_f) {
    move[F][CCW] = Front(dir);
    if (nsf1 == NULL) {
      nsf1 = move[F][CCW];
    } else {
      assert(SUCCESS == 
          ApplyBinary(OP_UNION, nsf1, move[F][CCW], nsf1));
    }
    if (debug_level > 0) {
      fprintf(stderr, "Union f-ccw: ");
      fflush(stderr);
    }
  }

  if (enable_u) {
    move[U][CCW] = Up(dir);
    if (nsf1 == NULL) {
      nsf1 = move[U][CCW];
    } else {
      assert(SUCCESS == 
          ApplyBinary(OP_UNION, nsf1, move[U][CCW], nsf1));
    }
    if (debug_level > 0) {
      fprintf(stderr, "Union u-ccw: ");
      fflush(stderr);
    }
  }

  if (enable_r) {
    move[R][CCW] = Right(dir);
    if (nsf1 == NULL) {
      nsf1 = move[R][CCW];
    } else {
      assert(SUCCESS == 
          ApplyBinary(OP_UNION, nsf1, move[R][CCW], nsf1));
    }
    if (debug_level > 0) {
      fprintf(stderr, "Union r-ccw: ");
      fflush(stderr);
    }
  }

  if (enable_l) {
    move[L][CCW] = Left(dir);
    if (nsf1 == NULL) {
      nsf1 = move[L][CCW];
    } else {
      assert(SUCCESS == 
          ApplyBinary(OP_UNION, nsf1, move[L][CCW], nsf1));
    }
    if (debug_level > 0) {
      fprintf(stderr, "Union l-ccw: ");
      fflush(stderr);
    }
  }

  if (enable_b) {
    move[B][CCW] = Back(dir);
    if (nsf1 == NULL) {
      nsf1 = move[B][CCW];
    } else {
      assert(SUCCESS == 
          ApplyBinary(OP_UNION, nsf1, move[B][CCW], nsf1));
    }
    if (debug_level > 0) {
      fprintf(stderr, "Union b-ccw: ");
      fflush(stderr);
    }
  }

  if (enable_d) {
    move[D][CCW] = Down(dir);
    if (nsf1 == NULL) {
      nsf1 = move[D][CCW];
    } else {
      assert(SUCCESS == 
          ApplyBinary(OP_UNION, nsf1, move[D][CCW], nsf1));
    }
    if (debug_level > 0) {
      fprintf(stderr, "Union d-ccw: ");
      fflush(stderr);
    }
  }

  if (nsf1 != NULL) {
    if (nsf != NULL) {
      assert(SUCCESS == ApplyBinary(OP_UNION, nsf, nsf1, nsf));
    } else {
      nsf = nsf1;
    }
  }

  if (union_flip) {
    dd_edge *nsf2 = NULL;
    dir = FLIP;
    if (enable_f || enable_F) {
      move[F][CCW] = Front(dir);
      if (nsf2 == NULL) {
        nsf2 = move[F][CCW];
      } else {
        assert(SUCCESS == 
            ApplyBinary(OP_UNION, nsf2, move[F][CCW], nsf2));
      }
      if (debug_level > 0) {
        fprintf(stderr, "Union f-ccw: ");
        fflush(stderr);
      }
    }

    if (enable_u || enable_U) {
      move[U][CCW] = Up(dir);
      if (nsf2 == NULL) {
        nsf2 = move[U][CCW];
      } else {
        assert(SUCCESS == 
            ApplyBinary(OP_UNION, nsf2, move[U][CCW], nsf2));
      }
      if (debug_level > 0) {
        fprintf(stderr, "Union u-ccw: ");
        fflush(stderr);
      }
    }

    if (enable_r || enable_R) {
      move[R][CCW] = Right(dir);
      if (nsf2 == NULL) {
        nsf2 = move[R][CCW];
      } else {
        assert(SUCCESS == 
            ApplyBinary(OP_UNION, nsf2, move[R][CCW], nsf2));
      }
      if (debug_level > 0) {
        fprintf(stderr, "Union r-ccw: ");
        fflush(stderr);
      }
    }

    if (enable_l || enable_L) {
      move[L][CCW] = Left(dir);
      if (nsf2 == NULL) {
        nsf2 = move[L][CCW];
      } else {
        assert(SUCCESS == 
            ApplyBinary(OP_UNION, nsf2, move[L][CCW], nsf2));
      }
      if (debug_level > 0) {
        fprintf(stderr, "Union l-ccw: ");
        fflush(stderr);
      }
    }

    if (enable_b || enable_B) {
      move[B][CCW] = Back(dir);
      if (nsf2 == NULL) {
        nsf2 = move[B][CCW];
      } else {
        assert(SUCCESS == 
            ApplyBinary(OP_UNION, nsf2, move[B][CCW], nsf2));
      }
      if (debug_level > 0) {
        fprintf(stderr, "Union b-ccw: ");
        fflush(stderr);
      }
    }

    if (enable_d || enable_D) {
      move[D][CCW] = Down(dir);
      if (nsf2 == NULL) {
        nsf2 = move[D][CCW];
      } else {
        assert(SUCCESS == 
            ApplyBinary(OP_UNION, nsf2, move[D][CCW], nsf2));
      }
      if (debug_level > 0) {
        fprintf(stderr, "Union d-ccw: ");
        fflush(stderr);
      }
    }

    if (nsf2 != NULL) {
      if (nsf != NULL) {
        assert(SUCCESS == ApplyBinary(OP_UNION, nsf, nsf2, nsf));
      } else {
        nsf = nsf2;
      }
    }
  }

  if (NULL == nsf) {
    fprintf(stderr, "Couldn't create next-state function\n");
    exit(1);
  }
}


void CleanUpForest() {
  if (debug_level > 0) {
    DestroyForest(relation, true);
  } else {
    DestroyForest(relation, false);
  }
  if (INVALID_FOREST != relation) {
    fprintf(stderr, "Couldn't destroy forest of relations\n");
    exit(1);
  } else {
    if (debug_level > 1) {
      fprintf(stderr, "Destroyed forest of relations\n");
    }
  }
  DestroyDomain(d);
}


void InitForest() {
  d = CreateDomain(num_levels, sizes, variables);
  if (NULL == d) {
    fprintf(stderr, "Couldn't create domain\n");
    exit(1);
  }
  CheckVars();
#if 1
  relation =
    CreateForest(d, MXD, false, forest::IDENTITY_REDUCED, FULL_OR_SPARSE_STORAGE);
#else
  relation =
    CreateForest(d, MXD, false, forest::IDENTITY_REDUCED, FULL_STORAGE);
#endif

  if (INVALID_FOREST == relation) {
    fprintf(stderr, "Couldn't create forest of relations\n");
    exit(1);
  } else {
    if (debug_level > 1)
      fprintf(stderr, "Created forest of relations\n");
  }
}


dd_edge* Front(direction dir) { return DoMove(F, dir); }
dd_edge* Back(direction dir) { return DoMove(B, dir); }
dd_edge* Left(direction dir) { return DoMove(L, dir); }
dd_edge* Right(direction dir) { return DoMove(R, dir); }
dd_edge* Up(direction dir) { return DoMove(U, dir); }
dd_edge* Down(direction dir) { return DoMove(D, dir); }

dd_edge* DoMove(face f, direction d) {
  dd_edge* result = NULL;
#if FROM_TO
  if (d != FLIP) { d = (d == CW)? CCW: CW; }
#endif
  switch (f) {
    case U:
      if (d == CW) {
        result = DoMoveHelper(4, 5, 5, 6, 1, 0, 0, 4);
      } else if (d == CCW) {
        result = DoMoveHelper(0, 4, 1, 0, 5, 6, 4, 5);
      } else {
        result = DoFlipHelper(4, 5, 5, 6, 1, 0, 0, 4);
      }
      break;
    case D:
      if (d == CW) {
        result =  DoMoveHelper(3, 2, 2, 8, 6, 9, 7, 10);
      } else if (d == CCW) {
        result =  DoMoveHelper(7, 10, 6, 9, 2, 8, 3, 2);
      } else {
        result =  DoFlipHelper(3, 2, 2, 8, 6, 9, 7, 10);
      }
      break;
    case L:
      if (d == CW) {
        result =  DoMoveHelper(0, 3, 3, 10, 7, 11, 4, 4);
      } else if (d == CCW) {
        result =  DoMoveHelper(4, 4, 7, 11, 3, 10, 0, 3);
      } else {
        result =  DoFlipHelper(0, 3, 3, 10, 7, 11, 4, 4);
      }
      break;
    case R:
      if (d == CW) {
        result =  DoMoveHelper(1, 1, 5, 6, 6, 7, 2, 8);
      } else if (d == CCW) {
        result =  DoMoveHelper(2, 8, 6, 7, 5, 6, 1, 1);
      } else {
        result =  DoFlipHelper(1, 1, 5, 6, 6, 7, 2, 8);
      }
      break;
    case F:
      if (d == CW) {
        result = DoMoveHelper(0, 0, 1, 1, 2, 2, 3, 3);
      } else if (d == CCW) {
        result = DoMoveHelper(3, 3, 2, 2, 1, 1, 0, 0);
      } else {
        result = DoFlipHelper(0, 0, 1, 1, 2, 2, 3, 3);
      }
      break;
    case B:
      if (d == CW) {
        result =  DoMoveHelper(5, 5, 4, 11, 7, 9, 6, 7);
      } else if (d == CCW) {
        result =  DoMoveHelper(6, 7, 7, 9, 4, 11, 5, 5);
      } else {
        result =  DoFlipHelper(5, 5, 4, 11, 7, 9, 6, 7);
      }
      break;
  }
  return result;
}


dd_edge* DoMoveHelper(
  int type3_a,
  int type2_a,
  int type3_b,
  int type2_b,
  int type3_c,
  int type2_c,
  int type3_d,
  int type2_d
  )
{
  const int sz = num_levels + 1;
  int from[sz];  // num_levels is a constant
  int to[sz];

  // face is ordered like this:
  // type3, type2, type3, type2, type3, type2, type3, type2

  // transform to levels
  int a2 = get_component_level(2, type2_a);
  int b2 = get_component_level(2, type2_b);
  int c2 = get_component_level(2, type2_c);
  int d2 = get_component_level(2, type2_d);
  int a3 = get_component_level(3, type3_a);
  int b3 = get_component_level(3, type3_b);
  int c3 = get_component_level(3, type3_c);
  int d3 = get_component_level(3, type3_d);

  if (debug_level > 1) {
    fprintf(stderr, "type2_a, a2 = %d, %d\n", type2_a, a2);
    fprintf(stderr, "type2_b, b2 = %d, %d\n", type2_b, b2);
    fprintf(stderr, "type2_c, c2 = %d, %d\n", type2_c, c2);
    fprintf(stderr, "type2_d, d2 = %d, %d\n", type2_d, d2);
    fprintf(stderr, "type3_a, a3 = %d, %d\n", type3_a, a3);
    fprintf(stderr, "type3_b, b3 = %d, %d\n", type3_b, b3);
    fprintf(stderr, "type3_c, c3 = %d, %d\n", type3_c, c3);
    fprintf(stderr, "type3_d, d3 = %d, %d\n", type3_d, d3);
  }

  // create node at level 13
  dd_tempedge *temp13 = CreateTempEdge(relation, NULL);

  // Set all levels (except term) to don't care
  fill_n(from + 1, sz - 1, -2);
  fill_n(to + 1, sz - 1, -2);
  from[variables[a3]] = -1; to[variables[a3]] = -1;
  from[variables[b3]] = -1; to[variables[b3]] = -1;
  from[variables[c3]] = -1; to[variables[c3]] = -1;
  from[variables[d3]] = -1; to[variables[d3]] = -1;
  from[variables[a2]] = -1; to[variables[a2]] = -1;
  from[variables[b2]] = -1; to[variables[b2]] = -1;
  from[variables[c2]] = -1; to[variables[c2]] = -1;
  from[variables[d2]] = -1; to[variables[d2]] = -1;

  for (int i=0; i<type3; i++) {
    from[variables[d3]] = i;
    to[variables[a3]] = i;
#if FROM_TO
    AddMatrixElement(temp13, from, to, sz, true);
#else
    AddMatrixElement(temp13, to, from, sz, true);
#endif
  }

  dd_edge *e13 = CreateEdge(temp13);

  // create node at level 14
  dd_tempedge *temp14 = CreateTempEdge(relation, NULL);

  // Set all levels (except term) to don't care
  fill_n(from + 1, sz - 1, -2);
  fill_n(to + 1, sz - 1, -2);
  from[variables[a3]] = -1; to[variables[a3]] = -1;
  from[variables[b3]] = -1; to[variables[b3]] = -1;
  from[variables[c3]] = -1; to[variables[c3]] = -1;
  from[variables[d3]] = -1; to[variables[d3]] = -1;
  from[variables[a2]] = -1; to[variables[a2]] = -1;
  from[variables[b2]] = -1; to[variables[b2]] = -1;
  from[variables[c2]] = -1; to[variables[c2]] = -1;
  from[variables[d2]] = -1; to[variables[d2]] = -1;

  for (int i=0; i<type3; i++) {
    from[variables[a3]] = i;
    to[variables[b3]] = i;
#if FROM_TO
    AddMatrixElement(temp14, from, to, sz, true);
#else
    AddMatrixElement(temp14, to, from, sz, true);
#endif
  }

  dd_edge *e14 = CreateEdge(temp14);

  // create node at level 15
  dd_tempedge *temp15 = CreateTempEdge(relation, NULL);

  // Set all levels (except term) to don't care
  fill_n(from + 1, sz - 1, -2);
  fill_n(to + 1, sz - 1, -2);
  from[variables[a3]] = -1; to[variables[a3]] = -1;
  from[variables[b3]] = -1; to[variables[b3]] = -1;
  from[variables[c3]] = -1; to[variables[c3]] = -1;
  from[variables[d3]] = -1; to[variables[d3]] = -1;
  from[variables[a2]] = -1; to[variables[a2]] = -1;
  from[variables[b2]] = -1; to[variables[b2]] = -1;
  from[variables[c2]] = -1; to[variables[c2]] = -1;
  from[variables[d2]] = -1; to[variables[d2]] = -1;

  for (int i=0; i<type3; i++) {
    from[variables[b3]] = i;
    to[variables[c3]] = i;
#if FROM_TO
    AddMatrixElement(temp15, from, to, sz, true);
#else
    AddMatrixElement(temp15, to, from, sz, true);
#endif
  }

  dd_edge *e15 = CreateEdge(temp15);

  // create node at level 16
  dd_tempedge *temp16 = CreateTempEdge(relation, NULL);

  // Set all levels (except term) to don't care
  fill_n(from + 1, sz - 1, -2);
  fill_n(to + 1, sz - 1, -2);
  from[variables[a3]] = -1; to[variables[a3]] = -1;
  from[variables[b3]] = -1; to[variables[b3]] = -1;
  from[variables[c3]] = -1; to[variables[c3]] = -1;
  from[variables[d3]] = -1; to[variables[d3]] = -1;
  from[variables[a2]] = -1; to[variables[a2]] = -1;
  from[variables[b2]] = -1; to[variables[b2]] = -1;
  from[variables[c2]] = -1; to[variables[c2]] = -1;
  from[variables[d2]] = -1; to[variables[d2]] = -1;

  for (int i=0; i<type3; i++) {
    from[variables[c3]] = i;
    to[variables[d3]] = i;
#if FROM_TO
    AddMatrixElement(temp16, from, to, sz, true);
#else
    AddMatrixElement(temp16, to, from, sz, true);
#endif
  }

  dd_edge *e16 = CreateEdge(temp16);
  
  // create node at level 1
  dd_tempedge *temp1 = CreateTempEdge(relation, NULL);

  // Set all levels (except term) to don't care
  fill_n(from + 1, sz - 1, -2);
  fill_n(to + 1, sz - 1, -2);
  from[variables[a3]] = -1; to[variables[a3]] = -1;
  from[variables[b3]] = -1; to[variables[b3]] = -1;
  from[variables[c3]] = -1; to[variables[c3]] = -1;
  from[variables[d3]] = -1; to[variables[d3]] = -1;
  from[variables[a2]] = -1; to[variables[a2]] = -1;
  from[variables[b2]] = -1; to[variables[b2]] = -1;
  from[variables[c2]] = -1; to[variables[c2]] = -1;
  from[variables[d2]] = -1; to[variables[d2]] = -1;

  for (int i=0; i<type2; i++) {
    from[variables[d2]] = i;
    to[variables[a2]] = i;
#if FROM_TO
    AddMatrixElement(temp1, from, to, sz, true);
#else
    AddMatrixElement(temp1, to, from, sz, true);
#endif
  }

  dd_edge *e1 = CreateEdge(temp1);

  // create node at level 2
  dd_tempedge *temp2 = CreateTempEdge(relation, NULL);

  // Set all levels (except term) to don't care
  fill_n(from + 1, sz - 1, -2);
  fill_n(to + 1, sz - 1, -2);
  from[variables[a3]] = -1; to[variables[a3]] = -1;
  from[variables[b3]] = -1; to[variables[b3]] = -1;
  from[variables[c3]] = -1; to[variables[c3]] = -1;
  from[variables[d3]] = -1; to[variables[d3]] = -1;
  from[variables[a2]] = -1; to[variables[a2]] = -1;
  from[variables[b2]] = -1; to[variables[b2]] = -1;
  from[variables[c2]] = -1; to[variables[c2]] = -1;
  from[variables[d2]] = -1; to[variables[d2]] = -1;

  for (int i=0; i<type2; i++) {
    from[variables[a2]] = i;
    to[variables[b2]] = i;
#if FROM_TO
    AddMatrixElement(temp2, from, to, sz, true);
#else
    AddMatrixElement(temp2, to, from, sz, true);
#endif
  }

  dd_edge *e2 = CreateEdge(temp2);

  // create node at level 3
  dd_tempedge *temp3 = CreateTempEdge(relation, NULL);

  // Set all levels (except term) to don't care
  fill_n(from + 1, sz - 1, -2);
  fill_n(to + 1, sz - 1, -2);
  from[variables[a3]] = -1; to[variables[a3]] = -1;
  from[variables[b3]] = -1; to[variables[b3]] = -1;
  from[variables[c3]] = -1; to[variables[c3]] = -1;
  from[variables[d3]] = -1; to[variables[d3]] = -1;
  from[variables[a2]] = -1; to[variables[a2]] = -1;
  from[variables[b2]] = -1; to[variables[b2]] = -1;
  from[variables[c2]] = -1; to[variables[c2]] = -1;
  from[variables[d2]] = -1; to[variables[d2]] = -1;

  for (int i=0; i<type2; i++) {
    from[variables[b2]] = i;
    to[variables[c2]] = i;
#if FROM_TO
    AddMatrixElement(temp3, from, to, sz, true);
#else
    AddMatrixElement(temp3, to, from, sz, true);
#endif
  }

  dd_edge *e3 = CreateEdge(temp3);
  
  // create node at level 4
  dd_tempedge *temp4 = CreateTempEdge(relation, NULL);

  // Set all levels (except term) to don't care
  fill_n(from + 1, sz - 1, -2);
  fill_n(to + 1, sz - 1, -2);
  from[variables[a3]] = -1; to[variables[a3]] = -1;
  from[variables[b3]] = -1; to[variables[b3]] = -1;
  from[variables[c3]] = -1; to[variables[c3]] = -1;
  from[variables[d3]] = -1; to[variables[d3]] = -1;
  from[variables[a2]] = -1; to[variables[a2]] = -1;
  from[variables[b2]] = -1; to[variables[b2]] = -1;
  from[variables[c2]] = -1; to[variables[c2]] = -1;
  from[variables[d2]] = -1; to[variables[d2]] = -1;

  for (int i=0; i<type2; i++) {
    from[variables[c2]] = i;
    to[variables[d2]] = i;
#if FROM_TO
    AddMatrixElement(temp4, from, to, sz, true);
#else
    AddMatrixElement(temp4, to, from, sz, true);
#endif
  }

  dd_edge *e4 = CreateEdge(temp4);
  // ShowDDEdge(stderr, e4);

  dd_edge *result = NULL;

  assert(SUCCESS == ApplyBinary(OP_INTERSECTION, e1, e2, result));
  assert(SUCCESS == ApplyBinary(OP_INTERSECTION, result, e3, result));
  assert(SUCCESS == ApplyBinary(OP_INTERSECTION, result, e4, result));
  assert(SUCCESS == ApplyBinary(OP_INTERSECTION, result, e13, result));
  assert(SUCCESS == ApplyBinary(OP_INTERSECTION, result, e14, result));
  assert(SUCCESS == ApplyBinary(OP_INTERSECTION, result, e15, result));
  assert(SUCCESS == ApplyBinary(OP_INTERSECTION, result, e16, result));
  ReleaseEdge(e1);
  ReleaseEdge(e2);
  ReleaseEdge(e3);
  ReleaseEdge(e4);
  ReleaseEdge(e13);
  ReleaseEdge(e14);
  ReleaseEdge(e15);
  ReleaseEdge(e16);
  return result;
}


dd_edge* DoFlipHelper(
  int type3_a,
  int type2_a,
  int type3_b,
  int type2_b,
  int type3_c,
  int type2_c,
  int type3_d,
  int type2_d
  )
{
  const int sz = num_levels + 1;
  int from[sz];  // num_levels is a constant
  int to[sz];

  // face is ordered like this:
  // type3, type2, type3, type2, type3, type2, type3, type2

  // transform to levels
  int a2 = get_component_level(2, type2_a);
  int b2 = get_component_level(2, type2_b);
  int c2 = get_component_level(2, type2_c);
  int d2 = get_component_level(2, type2_d);
  int a3 = get_component_level(3, type3_a);
  int b3 = get_component_level(3, type3_b);
  int c3 = get_component_level(3, type3_c);
  int d3 = get_component_level(3, type3_d);

  if (debug_level > 1) {
    fprintf(stderr, "type2_a, a2 = %d, %d\n", type2_a, a2);
    fprintf(stderr, "type2_b, b2 = %d, %d\n", type2_b, b2);
    fprintf(stderr, "type2_c, c2 = %d, %d\n", type2_c, c2);
    fprintf(stderr, "type2_d, d2 = %d, %d\n", type2_d, d2);
    fprintf(stderr, "type3_a, a3 = %d, %d\n", type3_a, a3);
    fprintf(stderr, "type3_b, b3 = %d, %d\n", type3_b, b3);
    fprintf(stderr, "type3_c, c3 = %d, %d\n", type3_c, c3);
    fprintf(stderr, "type3_d, d3 = %d, %d\n", type3_d, d3);
  }

  // create node at level 13
  dd_tempedge *temp13 = CreateTempEdge(relation, NULL);

  // Set all levels (except term) to don't care
  fill_n(from + 1, sz - 1, -2);
  fill_n(to + 1, sz - 1, -2);
  from[variables[a3]] = -1; to[variables[a3]] = -1;
  from[variables[b3]] = -1; to[variables[b3]] = -1;
  from[variables[c3]] = -1; to[variables[c3]] = -1;
  from[variables[d3]] = -1; to[variables[d3]] = -1;
  from[variables[a2]] = -1; to[variables[a2]] = -1;
  from[variables[b2]] = -1; to[variables[b2]] = -1;
  from[variables[c2]] = -1; to[variables[c2]] = -1;
  from[variables[d2]] = -1; to[variables[d2]] = -1;

  for (int i=0; i<type3; i++) {
    from[variables[d3]] = i;
    to[variables[b3]] = i;
    for (int j=0; j<type3; j++) {
      if (i == j) continue;
      from[variables[b3]] = j;
      to[variables[d3]] = j;
#if FROM_TO
      AddMatrixElement(temp13, from, to, sz, true);
#else
      AddMatrixElement(temp13, to, from, sz, true);
#endif
    }
  }

  dd_edge *e13 = CreateEdge(temp13);

  // create node at level 14
  dd_tempedge *temp14 = CreateTempEdge(relation, NULL);

  // Set all levels (except term) to don't care
  fill_n(from + 1, sz - 1, -2);
  fill_n(to + 1, sz - 1, -2);
  from[variables[a3]] = -1; to[variables[a3]] = -1;
  from[variables[b3]] = -1; to[variables[b3]] = -1;
  from[variables[c3]] = -1; to[variables[c3]] = -1;
  from[variables[d3]] = -1; to[variables[d3]] = -1;
  from[variables[a2]] = -1; to[variables[a2]] = -1;
  from[variables[b2]] = -1; to[variables[b2]] = -1;
  from[variables[c2]] = -1; to[variables[c2]] = -1;
  from[variables[d2]] = -1; to[variables[d2]] = -1;

  for (int i=0; i<type3; i++) {
    from[variables[a3]] = i;
    to[variables[c3]] = i;
    for (int j=0; j<type3; j++) {
      if (i == j) continue;
      from[variables[c3]] = j;
      to[variables[a3]] = j;
#if FROM_TO
      AddMatrixElement(temp14, from, to, sz, true);
#else
      AddMatrixElement(temp14, to, from, sz, true);
#endif
    }
  }

  dd_edge *e14 = CreateEdge(temp14);
 
  // create node at level 1
  dd_tempedge *temp1 = CreateTempEdge(relation, NULL);

  // Set all levels (except term) to don't care
  fill_n(from + 1, sz - 1, -2);
  fill_n(to + 1, sz - 1, -2);
  from[variables[a3]] = -1; to[variables[a3]] = -1;
  from[variables[b3]] = -1; to[variables[b3]] = -1;
  from[variables[c3]] = -1; to[variables[c3]] = -1;
  from[variables[d3]] = -1; to[variables[d3]] = -1;
  from[variables[a2]] = -1; to[variables[a2]] = -1;
  from[variables[b2]] = -1; to[variables[b2]] = -1;
  from[variables[c2]] = -1; to[variables[c2]] = -1;
  from[variables[d2]] = -1; to[variables[d2]] = -1;

  for (int i=0; i<type2; i++) {
    from[variables[d2]] = i;
    to[variables[b2]] = i;
    for (int j=0; j<type2; j++) {
      if (i == j) continue;
      from[variables[b2]] = j;
      to[variables[d2]] = j;
#if FROM_TO
      AddMatrixElement(temp1, from, to, sz, true);
#else
      AddMatrixElement(temp1, to, from, sz, true);
#endif
    }
  }

  dd_edge *e1 = CreateEdge(temp1);

  // create node at level 2
  dd_tempedge *temp2 = CreateTempEdge(relation, NULL);

  // Set all levels (except term) to don't care
  fill_n(from + 1, sz - 1, -2);
  fill_n(to + 1, sz - 1, -2);
  from[variables[a3]] = -1; to[variables[a3]] = -1;
  from[variables[b3]] = -1; to[variables[b3]] = -1;
  from[variables[c3]] = -1; to[variables[c3]] = -1;
  from[variables[d3]] = -1; to[variables[d3]] = -1;
  from[variables[a2]] = -1; to[variables[a2]] = -1;
  from[variables[b2]] = -1; to[variables[b2]] = -1;
  from[variables[c2]] = -1; to[variables[c2]] = -1;
  from[variables[d2]] = -1; to[variables[d2]] = -1;

  for (int i=0; i<type2; i++) {
    from[variables[a2]] = i;
    to[variables[c2]] = i;
    for (int j=0; j<type2; j++) {
      if (i == j) continue;
      from[variables[c2]] = j;
      to[variables[a2]] = j;
#if FROM_TO
      AddMatrixElement(temp2, from, to, sz, true);
#else
      AddMatrixElement(temp2, to, from, sz, true);
#endif
    }
  }

  dd_edge *e2 = CreateEdge(temp2);

  dd_edge *result = NULL;
  assert(SUCCESS == ApplyBinary(OP_INTERSECTION, e1, e2, result));
  assert(SUCCESS == ApplyBinary(OP_INTERSECTION, result, e13, result));
  assert(SUCCESS == ApplyBinary(OP_INTERSECTION, result, e14, result));
  ReleaseEdge(e1);
  ReleaseEdge(e2);
  ReleaseEdge(e13);
  ReleaseEdge(e14);
  return result;
}


void ShowLevelInfo()
{
  if (debug_level >= 0) {
    fprintf(stderr, "Type 3: [%d", get_component_level(3, 0));
    for (int i = 1; i < type3; i++) {
      fprintf(stderr, ", %d", get_component_level(3, i));
    }
    fprintf(stderr, "]\nType 2: [%d", get_component_level(2, 0));
    for (int i = 1; i < type2; i++) {
      fprintf(stderr, ", %d", get_component_level(2, i));
    }
    fprintf(stderr, "]\n");
    fflush(stderr);
  }
}


void SetLevelSizes()
{
  assert(num_levels == 20);
  sizes[0] = 0;
  for (int i = 1; i < (num_levels + 1); i++) {
    sizes[i] = getLevelSize(i);
    if (debug_level > 1)
      fprintf(stderr, "sizes[%d] = %d\n", i, sizes[i]);
  }
  if (debug_level > 1) fflush(stderr);
}


void InitMemory()
{
  assert(num_levels == (type2 + type3));

  // store for level handles
  variables = (level *) malloc((num_levels + 1) * sizeof(level));
  assert(variables != NULL);
  memset(variables, 0, (num_levels + 1) * sizeof(level));
  
  // node size for each level
  sizes = (int *) malloc((num_levels + 1) * sizeof(int));
  assert(sizes != NULL);
}


void CheckVars()
{
  // Sanity check
  for (int i = num_levels; i > 0; i--) 
    if ((variables[i] > num_levels) || (variables[i] < 1)) {
      fprintf(stderr, "Level handle for variable %d is %d, ",
          i, variables[i]);
      fprintf(stderr, "outside of expected range\n");
      exit(1);
    }
}


void SwapComponents(int comp1, int comp2)
{
  assert(comp1 > 0 && comp1 <= num_levels);
  assert(comp2 > 0 && comp2 <= num_levels);
  int l1 = get_component_level(comp1);
  int l2 = get_component_level(comp2);
  set_component_level(comp1, l2);
  set_component_level(comp2, l1);
  set_level_component(l2, comp1);
  set_level_component(l1, comp2);
}


void SwapComponents(int n1_type, int n1_index, int n2_type, int n2_index)
{
  SwapComponents(get_component(n1_type, n1_index),
      get_component(n2_type, n2_index));
}


void SwapLevels(int l1, int l2)
{
  assert(l1 > 0 && l1 <= num_levels);
  assert(l2 > 0 && l2 <= num_levels);
  int comp1 = get_level_component(l1);
  int comp2 = get_level_component(l2);
  set_component_level(comp1, l2);
  set_component_level(comp2, l1);
  set_level_component(l2, comp1);
  set_level_component(l1, comp2);
}


void RandomizeLevels(int n)
{
  // generate n pairs of numbers
  int n1, n2;
  for (int i = 0; i < n; i++) {
    // generate random number in the interval (0,num_levels-1)
    n1 = random() % num_levels;
    n2 = random() % num_levels;
    if (n1 != n2) SwapLevels(n1 + 1, n2 + 1);
  }
}


inline int get_component_type (int comp_level)
{
  assert(comp_level > 0 && comp_level <= num_levels);
  return (level_map[comp_level] > 12)? 3: 2;
}

inline int get_component_size (int comp_type)
{
  assert(comp_type == 2 || comp_type == 3);
  return (comp_type == 3)? type3: type2;
}

inline int getLevelSize (int comp_level)
{
  return get_component_size(get_component_type(comp_level));
}

inline int get_component (int comp_type, int index)
{
  assert((comp_type == 3 && index >= 0 && index <= 7) ||
      (comp_type == 2 && index >= 0 && index <= 11));
  return (index + ((comp_type == 3)? 13: 1));
}

inline int get_component_level (int comp)
{
  assert(comp > 0 && comp <= num_levels);
  return component_map[comp];
}

inline void set_component_level (int comp, int level)
{
  assert(comp > 0 && comp <= num_levels);
  assert(level > 0 && level <= num_levels);
  component_map[comp] = level;
}

inline int get_level_component (int level)
{
  assert(level > 0 && level <= num_levels);
  return level_map[level];
}

inline void set_level_component (int level, int comp)
{
  assert(comp > 0 && comp <= num_levels);
  assert(level > 0 && level <= num_levels);
  level_map[level] = comp;
}

inline int get_component_level (int comp_type, int index)
{
  return get_component_level(get_component(comp_type, index));
}


void swab_buffer ()
{
  memset(buf, 0, buf_sz);
}
void terminate (int param)
{
  if (debug_level > 1)
    fprintf(stderr, "Terminating program...\n");
  sprintf(buf, "%f ", double(0));
  write(fd[1], buf, buf_sz);
  close(fd[1]);
  exit(1);
}
void write_score (double score, double wt_score)
{
  sprintf(buf, "%lf %lf ", score, wt_score);
  write(fd[1], buf, buf_sz);
}
void read_score (double *score, double *wt_score)
{
  read(fd[0], buf, buf_sz);
  sscanf(buf, "%lf %lf", score, wt_score);
}


