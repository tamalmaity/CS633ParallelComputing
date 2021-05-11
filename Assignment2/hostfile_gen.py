import subprocess
import sys
import os
import random

try:
    os.remove("hostfile")
except OSError:
    pass
f1 = open("nodefile.txt", "r")
no_of_groups = int(sys.argv[1])
nodes_per_group = int(sys.argv[2])
cores = int(sys.argv[3])

lines = f1.readlines()
lines.reverse() # to get less used nodes at the end

total_groups = len(lines)
available_groups = 0
groups_checked = 0
final_result = []

#Since each line denotes a group and for P=4 we can have 2 processes in 2 groups 
#but those need not be the same 2 groups every time, so we introduce a probability factor
#Similarly for P=16 we can have 4 processes in 4 groups and those groups needn't be the
#same every time too.
count = -1;
for line in lines:
    #first condition randomly lets nodes from this group choosen or not and
    #second condition checks if it is safe to NOT choose any group because 
    #we have to mandatorily choose the group if we have left too many groups unchosen
    count+= 1
    if(random.randint(0,1)) and ((len(lines)-count)>no_of_groups): 
        continue
    nodes = line.strip().split(",")
    reachable = []
    nodes_in_this_group = 0

    for node in nodes:
        status = subprocess.call(["ssh", node, "uptime"])
        if status == 0:
            nodes_in_this_group += 1
            reachable.append(node)
        if nodes_in_this_group == nodes_per_group:
            break

    if nodes_in_this_group == nodes_per_group:
        final_result.append(reachable)
        available_groups += 1
        groups_checked += 1
        if available_groups == no_of_groups:
            break
    else:
        groups_checked += 1


subprocess.call(["clear"])
count = 0
if groups_checked == total_groups and available_groups < no_of_groups:
    print ("Not enough nodes available")
else:
    f2 = open("hostfile", "w+")
    for group in final_result:
        count += 1
        for node in group:
            temp = ""
            for x in node[5:]:
                temp+= x
            f2.write("172.27.19." + temp + ":" + str(cores) + "\n")
        



