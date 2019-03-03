import subprocess
plan_num = 384

for i in range(1,plan_num+1):
    file1 = open('found_plans/sas_plan.' + str(i)).read().splitlines()
    for j in range(i+1, plan_num+1):
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
             
cmd = ["VAL/validate", "benchmarks/gripper/domain.pddl", "benchmarks/gripper/prob01.pddl"] + ["found_plans/sas_plan." + str(x) for x in range(1,plan_num+1)]
print(' '.join(cmd))
