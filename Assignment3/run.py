import matplotlib
matplotlib.use('Agg')

import os
import sys
import matplotlib.pyplot as plt

filename = 'tdata.csv'
nodes = [1,2]
ppn = [1,2,4]
ip = "src.c"
op = "src.x "
os.popen("make").read()
#print("hello")

T = [] #stores the time corresponding to the 6 combinations

for x in nodes:
	for y in ppn:
		count = 0
		os.popen("python3 hostfile_generator.py " + str(x) + " " + str(y)).read() #to generate hostfile
		#print("python3 hostfile_generator.py " + str(x) + " " + str(y))
		command = "mpirun -np " + str(x*y) + " -f hostfile" + " ./" + op + filename 
		#print(command)
		os.popen(command).read() # to run src.c
		#reading the time and storing it for plotting
		f = open('output.txt', "r")
		for line in f:
			count+= 1
			if count==3:
				T.append((float)(line.split()[-1])) #reading the 3rd line containing time
				#print(T)

			

# plot the script
X = ['(1,1)', '(1,2)', '(1,4)', '(2,1)', '(2,2)', '(2,4)']
plt.xlabel('(Nodes,Cores)')
plt.ylabel('Time taken in secs')
plt.title('Plot for time taken against nodes and core combination')
plt.plot(X,T,'-bo')
plt.savefig('plot.jpg')
