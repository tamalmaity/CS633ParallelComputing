import sys;
import os;

inp = int(sys.argv[1])
st = 'hostfile' + str(inp)
file = open (st, 'w+')
y = (int) (inp/8) + 1
l = []
for i in range(20,61):
	if (i==19 or i==21 or i==35 or i==36 or i==38 or i==40 or i==41 or i==42 or i==44 or i==47 or i==50 or i==57 or i==60):
		continue
	else:
		l.append(i)

string = list(filter(lambda x: "time=" in os.popen("ping -c 1 " + x).read(), map(lambda n: "172.27.19." + str(n), l)))[:y]
for i in range (0,len(string)-1):
	file.write("%s:8\n" %string[i])
extra = inp-(y-1)*8
if extra!=0:
	file.write("%s" %string[-1] + ":" + str(extra) + "\n")
file.close()
