
# Symbolic Top-k Planner [![Linux build](https://github.com/speckdavid/symk/workflows/Linux%20build/badge.svg)](https://github.com/speckdavid/symk/actions?query=workflow%3A%22Linux+build%22)
# Symbolic Top-k Planner [![MacOS build](https://github.com/speckdavid/symk/workflows/MacOS%20build/badge.svg)](https://github.com/speckdavid/symk/actions?query=workflow%3A%22MacOS+build%22)

Sym-k is a state-of-the-art top-k planner. The objective of top-k planning is to determine a set of k different plans with lowest cost for a given planning task.

Main source:
 - Speck, D.; Mattmüller, R.; and Nebel, B. 2020. Symbolic top-k planning. In Proceedings of the Thirty-Second AAAI Conference on Artificial Intelligence (AAAI 2020), S. 9967-9974. AAAI Press. ([pdf](http://gki.informatik.uni-freiburg.de/papers/speck-etal-aaai2020.pdf))

Based on:
 - Symbolic Fast Downward: https://fai.cs.uni-saarland.de/torralba/software.html
 - Speck, D.; Geißer, F.; Mattmüller, R.; and Torralba, Á. 2019. Symbolic planning with axioms. In Lipovetzky, N.; Onaindia, E.; and Smith, D. E., eds., Proceedings of the Twenty-Ninth International Conference on Automated Planning and Scheduling (ICAPS 2019), 464–572. AAAI Press. ([pdf](http://gki.informatik.uni-freiburg.de/papers/speck-etal-icaps2019.pdf))
 - Fast Downward: http://www.fast-downward.org/
 
## Dependencies
Currently we only support Linux systems. The following should install all necessary dependencies.
```console
$ sudo apt-get -y install cmake g++ make python autotools-dev automake gcc g++-multilib
```
 
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

## Generate-and-Test Plans Framework

It is possible to create plans until a plan is found that meets complex requirements.
For this purpose it is possible to write your own plan selector. During the search, plans are created and handed over to a plan selector with an anytime behavior. 
Two examples of plan selectors are the [ top_k_selector](src/search/symbolic/plan_selection/top_k_selector.cc) and
the [top_k_even_selector](src/search/symbolic/plan_selection/top_k_even_selector.cc).
The most important function is *add_plan*, in which you can specify whether a newly generated plan shall be accepted or rejected.
To create your own plan selector, you can copy the *.cc* and *.h* files of one of these two selectors and adjust them accordingly. Also add the new file name to [DownwardFiles.cmake](src/search/DownwardFiles.cmake), similar to the other selection files.
Finally, if you want to find a plan with your *awesome_selector* selector (the name of the selector you specified for the plugin in the *.cc* file), you can use the following command. 

```console
$ ./fast-downward.py domain.pddl problem.pddl --search "symk-bd(plan_selection=awesome_selector(num_plans=1))"
```

Note, that you can also search for the best **k** plans using your selector.

# Fast Downward

Fast Downward is a domain-independent planning system.

For documentation and contact information see http://www.fast-downward.org/.

The following directories are not part of Fast Downward as covered by this
license:

* ./src/search/ext

For the rest, the following license applies:

```
Fast Downward is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

Fast Downward is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see <http://www.gnu.org/licenses/>.
```

