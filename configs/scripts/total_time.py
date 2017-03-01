#!/usr/bin/env python
import os
#name1="/mnt/freenas/dyn-trace-log/callgraph.dat"

name2="/home/crdong/main/x86_32/result_TotalTime.dat"
fout1 = open(name2,"w")
dic_call={}
num=0
for i in range(0,1):
  name="/home/crdong/main/x86_32/result_time" 
  fin = open (name)  
  for line in fin:     
     gram=line.split(" ")
     if gram[0].split("PID:")[1].strip()=="0x0":
       continue 
     if gram[5].split("TO:")[1].strip() not in dic_call:
       num=num+1     
       dic_call[gram[5].split("TO:")[1].strip() ]=[num,gram[7].split(":")[1].strip()]       
     else:
       #print dic_call[gram[0].split("TO:")[1].strip() ]
       time=long(str(dic_call[gram[5].split("TO:")[1].strip() ]).split(", '")[1].split("']")[0])+long(gram[7].split(":")[1].strip())
       dic_call[gram[5].split("TO:")[1].strip() ]=[int(str(dic_call[gram[5].split("TO:")[1].strip() ]).split("[")[1].split(",")[0]),str(time)]

for key in dic_call:
  fout1.write(key+" "+str(dic_call[key]).split(", '")[1].split("']")[0]+os.linesep)

print "graph over"




