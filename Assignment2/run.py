#!/usr/bin/python
import os
import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns
import numpy as np
import random

P = [4,16]
PPN = [1,8]
D = [16, 256, 2048]
ip = "src.c"
op = "src.x "
os.popen("make").read() #make to compile the program
#print("hello")
groups = -1
nodes = -1

data = ['Bcast', 'Reduce', 'Gather', 'Alltoallv'] # data files are deleted if existing previously
for item in data:
	file = 'data_'+item+'.txt'
	if os.path.isfile(file):
		os.remove(file)

#3 for loops to run all combinations
for x in range(len(P)): 
	if P[x]==4:
		groups = 2
		nodes = 2
	if P[x]==16:
		groups = 4
		nodes = 4
	for y in range(len(PPN)):
		os.popen("python3 hostfile_gen.py " + str(groups) + ' ' + str(nodes) + ' ' + str(PPN[y])).read()
		for z in range(len(D)):
			root = random.randint(0, (P[x]*PPN[y])-1)
			command = "for i in `seq 10`; do mpirun -np " + str(P[x]*PPN[y]) + " -f hostfile ./" + op + " " + str(root) + " " + str(D[z])  + " " + str(PPN[y]) + "; done"
			print(command)
			os.popen(command).read()

# plot the script
#os.popen("python3 plot.py")
