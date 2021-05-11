#!/usr/bin/env python
# coding: utf-8

# In[47]:


import pandas as pd
import seaborn as sns
import numpy as np
import matplotlib.pyplot as plt
import math

sns.set()

names = ['Bcast', 'Reduce', 'Gather', 'Alltoallv']


for item in names:
    L1 = []
    f = open('data_' + item + '.txt', "r")
    for line in f:
        L1.append(line)

    L1_new = []
    for i in range(10):
        for j in range(12):
            L1_new.append(L1[2*i+j*20])
            L1_new.append(L1[2*i+j*20+1])

    data = []
    for line in L1_new: # each containing D P PPN MODE TIME
        temp = line.split(' ') 
        for i in range (len(temp)):
            if i==len(temp)-1:
                temp[i] = float(temp[i][:-1])
            else:
                temp[i] = int(temp[i])
        #print(temp)
        data.append(temp)
    i=0
    #print(data)


    # In[48]:


    demo_input_format = pd.DataFrame.from_dict({
        "D": [],
        "P": [],
        "ppn": [],
        "mode": [],  # 1 --> optimized, 0 --> standard
        "time": [],
    })




    # In[49]:


    for execution in range(10):
        for P in [4, 16]:
            for ppn in [1, 8]:
                for D in [16, 256, 2048]:
                    # Change with the actual data
                    demo_input_format = demo_input_format.append({
                        "D": D, "P": P, "ppn": ppn, "mode": 'std', "time": math.log10(data[i][-1])
                    }, ignore_index=True)
                    i+=1
                    demo_input_format = demo_input_format.append({
                        "D": D, "P": P, "ppn": ppn, "mode": 'opt', "time": math.log10(data[i][-1])
                    }, ignore_index=True)
                    i+=1


    demo_input_format["(P, ppn)"] = list(map(lambda x, y: ("(" + x + ", " + y + ")"), map(str, demo_input_format["P"]), map(str, demo_input_format["ppn"])))
    #print(demo_input_format.to_string())

    
    fig = sns.catplot(x="(P, ppn)", y="time", data=demo_input_format, kind="bar", col="D", hue="mode", height=8, aspect = 0.97)
    fig.set_ylabels('log10 of time taken (in secs)')
    plt.suptitle("PLOT FOR " + item, y = 1.1, fontsize=16, fontweight='bold')
    #plt.show()
    string = 'plot_' + item + '.jpg'
    plt.savefig(string, dpi=97, pad_inches = 1, bbox_inches='tight') 

# In[ ]:




