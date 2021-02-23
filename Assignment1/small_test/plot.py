import matplotlib
#matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
import math

process_no = [4,9]

for g in process_no:
      st = 'data'+str(g)+'.txt'
      f = open(st, "r")

      send = [[None]*5 for _ in range(2)] #7 rows corresponding to different data points and 5 column each for 5 different runtimes
      pack = [[None]*5 for _ in range(2)]
      datatype = [[None]*5 for _ in range(2)]


      i = 0
      touch = 0
      for x in f:
      	row = (int)(i/15)
      	col = (int)((i%15)/3)
      	if i%3==0: #corresponding to send_receive
      		touch+= 1
      		send[row][col] = math.log2((float)(x.split()[-1]))

      	elif i%3==1: #corresponding to pack_unpack
      		pack[row][col] = math.log2((float)(x.split()[-1]))

      	else: #corresponding to MPI_Datatype
      		datatype[row][col] = math.log2((float)(x.split()[-1]))

      	i+= 1

      '''
      for i in range (7):
      	for j in range (5):
      		print(datatype[i][j],)
      	print()
      '''

      fig, ax = plt.subplots(1,1,figsize=(16,9))

      positions = (1,2)
      labels = ["2^2","4^2"]

      ax.set_xlabel('Data points per process')
      ax.set_ylabel('log2 of time taken in secs')


      ### Plotting the boxplots
      c1 = "orange"
      im1 = plt.boxplot(send, patch_artist=True,
                  boxprops=dict(facecolor="white", color=c1),
                  capprops=dict(color=c1),
                  whiskerprops=dict(linestyle = '--',color=c1),
                  flierprops=dict(color=c1, markeredgecolor=c1),
                  medianprops=dict(color="blue"), zorder = 1
                  )

      med1 = []
      for med in im1['medians']:
      	med1.append(med.get_ydata()[0])

      plt.plot(positions, med1, marker = 'o', color = 'orange',zorder = 2)


      c2 = "green"
      im2 = plt.boxplot(pack, patch_artist=True,
                  boxprops=dict(facecolor="white", color=c2),
                  capprops=dict(color=c2),
                  whiskerprops=dict(linestyle = '--', color=c2),
                  flierprops=dict(color=c2, markeredgecolor=c2),
                  medianprops=dict(color="maroon"), zorder = 1
                  )

      med2 = []
      for med in im2['medians']:
      	med2.append(med.get_ydata()[0])

      plt.plot(positions, med2, marker = 'o', color = 'green',zorder = 2)

      c3 = "red"
      im3 = plt.boxplot(datatype, patch_artist=True,
                  boxprops=dict(facecolor="white", color=c3),
                  capprops=dict(color=c3),
                  whiskerprops=dict(linestyle = '--',color=c3),
                  flierprops=dict(color=c3, markeredgecolor=c3),
                  medianprops=dict(color="black"), zorder = 1
                  )

      med3 = []
      for med in im3['medians']:
      	med3.append(med.get_ydata()[0])

      plt.plot(positions, med3, marker = 'o', color = 'red',zorder = 2)


      item1 = mpatches.Patch(color=c1, label = 'Send_Receive')
      item2 = mpatches.Patch(color=c2, label = 'Pack_Unpack')
      item3 = mpatches.Patch(color=c3, label = 'Datatype')
      plt.legend(handles = [item1, item2, item3])
               
      plt.xticks(positions, labels);
      plt.title("Box plot for " + str(g) + " processes")
      #plt.show()
      string = 'plot' + str(g) + '.jpg'
      fig.savefig(string)
