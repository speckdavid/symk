import subprocess

for i in range(1,31):
    file1 = open('found_plans/sas_plan.' + str(i)).read().splitlines()
    for j in range(i+1, 31):
        not_same = False
        file2 = open('found_plans/sas_plan.' + str(j)).read().splitlines()
        if len(file1) == len(file2):
            for l in range(0, len(file1)):
                if file1[l] != file2[l]:
                   not_same = True
                   break
            if not not_same:
                print(str(i) + " and " + str(j) + "are the same!!!")
                exit()
             
cmd = ["VAL/validate", "benchmarks/ged-opt14-strips/domain.pddl", "benchmarks/ged-opt14-strips/d-1-3.pddl"] + ["found_plans/sas_plan." + str(x) for x in range(1,31)]
print(' '.join(cmd))
