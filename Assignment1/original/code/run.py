#!/usr/bin/python
import os
import matplotlib.pyplot as plt

'''
Main script to run the whole program. The overall algorithm of this file is as follows:
-> First we run make to compile the program.
-> Then we find other hosts which are up and running using the script "hostfile_generator.py", this generates hostfile.
-> The data sizes for which to run the program is also passed as command line argument to mpirun.
-> Then we take the output of this run, write it to a file "dataP.txt", here P represents the number of the processes.
-> Then we run the plotting script "plot.py" to plot the graph.
'''

processes = [16,36,49,64]
data_sizes = [16, 32, 64, 128, 256, 512, 1024]  #data_sizes are the square of these size.
ip = "src.c"
op = "src.x "
os.popen("make").read()  #compile the code

#delete the data files of previous runs if present 
for x in range(len(processes)):
	file = 'data' + str(processes[x]) + '.txt'
	if os.path.isfile(file):
		os.remove(file)


#print("hello")

for x in range(len(processes)): #iterating through the processes
	os.popen("python3 hostfile_generator.py " + str(processes[x])).read()  # running the hostfile_generator
	for y in range(len(data_sizes)): #iterating through data sizes
		command = "for i in `seq 5`; do mpirun -np " + str(processes[x]) + " -f hostfile" + str(processes[x]) + " ./" + op + str(data_sizes[y]) + "; done"
		#print(command)
		os.popen(command).read()  # 5 runs per process per data size

# for generating the plots
os.popen("python3 plot.py")
