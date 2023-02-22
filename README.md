# Symk [![Linux build](https://github.com/speckdavid/symk/workflows/Linux%20build/badge.svg)](https://github.com/speckdavid/symk/actions?query=workflow%3A%22Linux+build%22) [![MacOS build](https://github.com/speckdavid/symk/workflows/MacOS%20build/badge.svg)](https://github.com/speckdavid/symk/actions?query=workflow%3A%22MacOS+build%22)

Symk is a state-of-the-art classical *optimal* and *top-k planner* based on symbolic search.


With Symk, it is possible to find a *single optimal plan* or a *set of k different best plans* with the lowest cost for a given planning task. 
In addition, Symk natively supports a variety of PDDL features that are rarely supported by other planners, such as conditional effects, derived predicates with axioms, and state-dependent action costs.
See this readme file for more information on running Symk and the various configurations. 
We appreciate citations when SymK is used in a scientific context (see [References](#references) for more details).

## Table of Contents  
- [Getting Started](#getting-started)
  - [Dependencies](#dependencies)
  - [Compiling the Symk Planner](#compiling-the-symk-planner)
  - [Apptainer Image](#apptainer-image)
- [Generating A Single Optimal Solution](#generating-a-single-optimal-solution)
- [Generating Multiple Solutions](#generating-multiple-solutions)
  - [Top-k Configurations](#top-k-configurations)
  - [Top-q Configurations](#top-q-configurations)
  - [Loopless Planning](#loopless-planning)
  - [Other Configurations](#other-configurations)
- [Plan Selection Framework](#plan-selection-framework)
  - [Unordered Plan Selector](#unordered-plan-selector)
  - [New Plan Selector](#new-plan-selector)
- [Pitfalls and Troubleshooting](#pitfalls-and-troubleshooting)
- [References](#references)

## Getting Started

### Dependencies
Currently we only support Linux systems. The following should install all necessary dependencies.
```console
sudo apt-get -y install cmake g++ make python3 autoconf automake
```

Symk should compile on MacOS with the GNU C++ compiler and clang with the same instructions described above.
 
### Compiling the Symk Planner
```console
./build.py 
```

### Apptainer Image
To simplify the installation process, we alternatively provide an executable [Apptainer](https://apptainer.org/) container (formerly known as Singularity). It accepts the same arguments as Symk (`fast-downward.py` script; see below).

```console
# Download the image,
apptainer pull symk.sif oras://ghcr.io/speckdavid/symk:latest

# or build it yourself.
apptainer build symk.sif Apptainer

# Then run the desired configuration (for other configurations see below).
./symk.sif domain.pddl problem.pddl --search "sym-bd()"
```

## Generating A Single Optimal Solution
We recommend to use the following configuration which uses bidirectional search.

```console
./fast-downward.py domain.pddl problem.pddl --search "sym-bd()"
```

Other configurations are forward or backward search: `--search "sym-fw()"` or `--search "sym-bw()"`.

## Generating Multiple Solutions

### Top-k Configurations

We recommend to use the following configuration which uses bidirectional search and 
reports the best **k** plans. Note that you can also specify `num_plans=infinity` if you want to find all possible plans.

```console
./fast-downward.py domain.pddl problem.pddl --search "symk-bd(plan_selection=top_k(num_plans=**k**))"
```

### Top-q Configurations
We recommend to use the following configuration which uses bidirectional search and
reports the **k** plans with quality bound **q**. Quality `1<=q<=infinity` is a multiplier that is multiplied to the cost of the cheapest solution. 
For example, `q=1` reports only the cheapest plans, where `quality=infinity` corresponds to the top-k planning.

```console
./fast-downward.py domain.pddl problem.pddl --search "symq-bd(plan_selection=top_k(num_plans=**k**),quality=**q**)"
```

### Loopless Planning
It is possible to generate loopless/simple plans, i.e., plans that do not visit any state more than once. In general, the option to consider and generate only simple plans can be combined with any Symk search engine and with different plan selectors by setting the `simple` parameter to true. See the following two examples and our [ICAPS 2022 Paper](https://gki.informatik.uni-freiburg.de/papers/vontschammer-etal-icaps2022.pdf).

```console
./fast-downward.py domain.pddl problem.pddl --search "symk-bd(simple=true,plan_selection=top_k(num_plans=**k**))"
```

```console
./fast-downward.py domain.pddl problem.pddl --search "symq-bd(simple=true,plan_selection=top_k(num_plans=**k**),quality=**q**)"
```
### Other Configurations
It is possible to run Symk also with forward or backward search instead of bidirectional search, e.g., with `--search "symk-fw(...)"` or `--search "symk-bw(...)"`. Depending on the domain, one of these configurations may be faster than bidirectional search (`"--search symk-bd(...)"`).

## Plan Selection Framework
It is possible to create plans until a number of plans or simply a single plan is found that meets certain requirements.
For this purpose it is possible to write your own plan selector. During the search, plans are created and handed over to a plan selector with an anytime behavior. 

### Unordered Plan Selector
An example of a plan selector is the [unordered_selector](src/search/symbolic/plan_selection/unordered_selector.cc), which considers two plans as equivalent if their action multi-sets are equivalent. In other words, plans with the same multi-set of actions form an equivalence class and only one representative plan is reported for each equivalence class.
Note that plan selectors can be combined with the different planning configurations.

We recommend to use the following configurations which use bidirectional search.

#### Unordered Top-k:
```console
./fast-downward.py domain.pddl problem.pddl --search "symk-bd(plan_selection=unordered(num_plans=**k**))"
```
#### Unordered Top-q:
```console
./fast-downward.py domain.pddl problem.pddl --search "symq-bd(plan_selection=unordered(num_plans=**k**),quality=**q**)"
```

### New Plan Selector
Two simple examples of plan selectors are the [top_k_selector](src/search/symbolic/plan_selection/top_k_selector.cc) and
the [top_k_even_selector](src/search/symbolic/plan_selection/top_k_even_selector.cc).
For this purpose it is possible to write your own plan selector.
The most important function is *add_plan*, in which you can specify whether a newly generated plan shall be accepted or rejected.
To create your own plan selector, you can copy the *.cc* and *.h* files of one of these two selectors and adjust them accordingly. Also add the new file name to [DownwardFiles.cmake](src/search/DownwardFiles.cmake), similar to the other selection files.
Finally, if you want to find a plan with your *awesome_selector* selector (the name of the selector you specified for the plugin in the *.cc* file), you can use the following command. 

```console
./fast-downward.py domain.pddl problem.pddl --search "symk-bd(plan_selection=awesome_selector(num_plans=1))"
```

Note, that you can also search for the best **k** plans using your selector.

## Pitfalls and Troubleshooting
By default, the planner performs a relevance analysis and removes components such as variables and actions that are irrelevant to achieving the goal. Although such variables and actions can in principle lead to further (simple) plans, they are classified as irrelevant and removed when translating PDDL to SAS+. If you wish to **obtain all plans** (even the non-relevant ones), please use the following options:

```console
./fast-downward.py --translate --search domain.pddl problem.pddl --translate-options --keep-unimportant-variables --search-options --search "symk-bd(plan_selection=top_k(num_plans=**k**))
```

# References
Note that several components of SymK have been developed and published separately. 
We appreciate citations of these sources when used.

### Main source

 1. David Speck, Robert Mattmüller, Bernhard Nebel: Symbolic Top-k Planning. AAAI 2020: 9967-9974 [[pdf]](https://rlplab.com/papers/speck-et-al-aaai2020.pdf) [[bib]](https://rlplab.com/papers/speck-et-al-aaai2020.html)

### Loopless Top-k planning

 2. Julian von Tschammer, Robert Mattmüller, David Speck: Loopless Top-K Planning. ICAPS 2022: 380-384 [[pdf]](https://rlplab.com/papers/vontschammer-et-al-icaps2022.pdf) [[bib]](https://rlplab.com/papers/vontschammer-et-al-icaps2022.html)

### Axiom and derived predicate support

 3. David Speck, Florian Geißer, Robert Mattmüller, Álvaro Torralba: Symbolic Planning with Axioms. ICAPS 2019: 464-472 [[pdf]](https://rlplab.com/papers/speck-et-al-icaps2019.pdf) [[bib]](https://rlplab.com/papers/speck-et-al-icaps2019.html)

### State-dependent action cost support

 4. David Speck: Symbolic Search for Optimal Planning with Expressive Extensions. Ph.D. thesis: University of Freiburg (2022) [[pdf]](https://rlplab.com/papers/speck-phd2022.pdf) [[bib]](https://rlplab.com/papers/speck-phd2022.html)
 
 You can find examples of domains with state-dependent action cost [here](https://github.com/speckdavid/SDAC-Benchmarks).

Finally, we want to acknowledge that Symk is based on:
 - Fast Downward: http://www.fast-downward.org/ (22.06)
 - Symbolic Fast Downward: https://people.cs.aau.dk/~alto/software.html
