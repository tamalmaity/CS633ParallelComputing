import os
import matplotlib.pyplot as plt
import socket

processes = [4,9]
data_sizes = [2, 4]
inp = "src.c"
op = "src.x "
plot_script = "plot.py"
data_file = "data.txt"
os.popen("make").read()

for x in range(len(processes)):
	file = 'data' + str(processes[x]) + '.txt'
	if os.path.isfile(file):
		os.remove(file)


for x in range(len(processes)):
	#os.popen("python3 hostfile_generator.py " + str(processes[x])).read()
	for y in range(len(data_sizes)):
		command = "for i in `seq 5`; do mpirun -np " + str(processes[x]) + " ./" + op + str(data_sizes[y]) + "; done"
		data_time = os.popen(command).read()
		#os.popen("echo \"" + data_time + "\" > " + data_file)

# plot the script
os.popen("python3 " + plot_script)
#os.popen("python3 " + "plot.py")

