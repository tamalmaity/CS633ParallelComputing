import sys;
import os;

inp = int(sys.argv[1])
cores = int(sys.argv[2])
st = 'hostfile'
file = open (st, 'w+')
l = []
for i in range(11,61):
	if (i==19 or i==21 or i==35 or i==36 or i==38 or i==40 or i==41 or i==42 or i==44 or i==47 or i==50 or i==57 or i==60):
		continue
	else:
		l.append(i)

string = list(filter(lambda x: "time=" in os.popen("ping -c 1 " + x).read(), map(lambda n: "172.27.19." + str(n), l)))[:inp]
#print(string)
for i in range (0,len(string)):
	file.write("%s:"%string[i] + str(cores) + "\n")
file.close()
