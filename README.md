
# Symbolic Top-k Planner [![Linux build](https://github.com/speckdavid/symk/workflows/Linux%20build/badge.svg)](https://github.com/speckdavid/symk/actions?query=workflow%3A%22Linux+build%22) [![MacOS build](https://github.com/speckdavid/symk/workflows/MacOS%20build/badge.svg)](https://github.com/speckdavid/symk/actions?query=workflow%3A%22MacOS+build%22)

Sym-k is a state-of-the-art top-k planner. The objective of top-k planning is to determine a set of k different plans with lowest cost for a given planning task.

Main source:
 - Speck, D.; Mattmüller, R.; and Nebel, B. 2020. Symbolic top-k planning. In Proceedings of the 34th AAAI Conference on Artificial Intelligence (AAAI 2020), S. 9967-9974. AAAI Press. ([pdf](http://gki.informatik.uni-freiburg.de/papers/speck-etal-aaai2020.pdf))

```console
@InProceedings{speck-et-al-aaai2020,
  author =       "David Speck and Robert Mattm{\"u}ller and Bernhard Nebel",
  title =        "Symbolic Top-k Planning",
  editor =       "Vincent Conitzer and Fei Sha",
  booktitle =    "Proceedings of the Thirty-Fourth {AAAI} Conference on
                  Artificial Intelligence ({AAAI} 2020)",
  publisher =    "{AAAI} Press",
  year =         "2020",
  pages =        "9967--9974"
}
```

Based on:
 - Fast Downward: http://www.fast-downward.org/ (20.06)
 - Symbolic Fast Downward: https://people.cs.aau.dk/~alto/software.html
 
## Dependencies
Currently we only support Linux systems. The following should install all necessary dependencies.
```console
$ sudo apt-get -y install cmake g++ make python3 autoconf automake
```

Sym-k should compile on MacOS with the GNU C++ compiler and clang with the same instructions described above.
 
## Compiling the Top-k Planner

```console
$ ./build.py 
```

## Top-k Configurations

We recommend to use the following configuration which uses bidirectional search and 
reports the best **k** plans. Note that you can also specify `num_plans=infinity` if you want to find all possible plans.

```console
$ ./fast-downward.py domain.pddl problem.pddl --search "symk-bd(plan_selection=top_k(num_plans=**k**))"
```

Other configurations are as follows.

 
```console
# Forward Search
$ ./fast-downward.py domain.pddl problem.pddl --search "symk-fw(plan_selection=top_k(num_plans=**k**))"


# Backward Search
$ ./fast-downward.py domain.pddl problem.pddl --search "symk-bw(plan_selection=top_k(num_plans=**k**))"
```

## Top-q Configurations
We recommend to use the following configuration which uses bidirectional search and
reports the **k** plans with quality bound **q**. Quality `1<=q<=infinity` is a multiplier that is multiplied to the cost of the cheapest solution. 
For example, `q=1` reports only the cheapest plans, where `quality=infinity` corresponds to the top-k planning.

```console
$ ./fast-downward.py domain.pddl problem.pddl --search "symq-bd(plan_selection=top_k(num_plans=**k**),quality=**q**)"
```

Other configurations are as follows.


```console
# Forward Search
$ ./fast-downward.py domain.pddl problem.pddl --search "symq-fw(plan_selection=top_k(num_plans=**k**),quality=**q**)"


# Backward Search
$ ./fast-downward.py domain.pddl problem.pddl --search "symq-bw(plan_selection=top_k(num_plans=**k**),,quality=**q**)"
```

## Loopless/Simple Planning
It is possible to generate loopless/simple plans, i.e., plans that do not visit any state more than once. In general, the option to consider and generate only simple plans can be combined with any Sym-k search engine and with different plan selectors by setting the `simple` parameter to true. See the following two examples and our [ICAPS 2022 Paper](https://gki.informatik.uni-freiburg.de/papers/vontschammer-etal-icaps2022.pdf).

```console
$ ./fast-downward.py domain.pddl problem.pddl --search "symk-bd(simple=true,plan_selection=top_k(num_plans=**k**))"
```

```console
$ ./fast-downward.py domain.pddl problem.pddl --search "symq-bd(simple=true,plan_selection=top_k(num_plans=**k**),quality=**q**)"
```

## Ordinary Planning Configurations
We recommend to use the following configuration which uses bidirectional search.

```console
$ ./fast-downward.py domain.pddl problem.pddl --search "sym-bd()"
```

Other configurations are as follows.


```console
# Forward Search
$ ./fast-downward.py domain.pddl problem.pddl --search "sym-fw())"


# Backward Search
$ ./fast-downward.py domain.pddl problem.pddl --search "sym-bw()"
```
## Plan Selection - Generate-and-Test Plans Framework
It is possible to create plans until a number of plans or simply a single plan is found that meets certain requirements.
For this purpose it is possible to write your own plan selector. During the search, plans are created and handed over to a plan selector with an anytime behavior. 

### Unordered Plan Selector
An example of a plan selector is the [unordered_selector](src/search/symbolic/plan_selection/unordered_selector.cc), which consideres two plans as equivalent if their action multi-sets are equivalent. In other words, plans with the same multi-set of actions form an equivalence class and only one representative plan is reported for each equivalence class.
Note that plan selectors can be combined with the different planning configurations.

We recommend to use the following configurations which use bidirectional search.

#### Unordered Top-k:
```console
$ ./fast-downward.py domain.pddl problem.pddl --search "symk-bd(plan_selection=unordered(num_plans=**k**))"
```
#### Unordered Top-q:
```console
$ ./fast-downward.py domain.pddl problem.pddl --search "symq-bd(plan_selection=unordered(num_plans=**k**),quality=**q**)"
```

### New Plan Selector
Two simple examples of plan selectors are the [top_k_selector](src/search/symbolic/plan_selection/top_k_selector.cc) and
the [top_k_even_selector](src/search/symbolic/plan_selection/top_k_even_selector.cc).
For this purpose it is possible to write your own plan selector.
The most important function is *add_plan*, in which you can specify whether a newly generated plan shall be accepted or rejected.
To create your own plan selector, you can copy the *.cc* and *.h* files of one of these two selectors and adjust them accordingly. Also add the new file name to [DownwardFiles.cmake](src/search/DownwardFiles.cmake), similar to the other selection files.
Finally, if you want to find a plan with your *awesome_selector* selector (the name of the selector you specified for the plugin in the *.cc* file), you can use the following command. 

```console
$ ./fast-downward.py domain.pddl problem.pddl --search "symk-bd(plan_selection=awesome_selector(num_plans=1))"
```

Note, that you can also search for the best **k** plans using your selector.
