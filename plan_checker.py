import filecmp

for i in range(1,2001):
    for j in range(i+1,2001):
        if filecmp.cmp("sas_plan." + str(i), "sas_plan." + str(j)):
            print("sas_plan." + str(i) + " and " "sas_plan." + str(j) + "are identical!")
