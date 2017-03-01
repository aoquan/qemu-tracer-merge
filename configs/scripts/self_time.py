#!/usr/bin/env python
import os
stack=[]
dic_tid={}
dic_time={}
time1=0
time=0
time_d=0
i=0
j=0
name2="/home/crdong/main/x86_32/temp_out2.txt"
fout1 = open(name2, "w")

for i in range(0,1):
  name="/home/crdong/main/x86_32/result"
  name1="/home/crdong/main/x86_32/result_time"
 
  
  fin = open(name)
  fout = open(name1, "w")
 
  for line in fin:
    gram=line.split(",")
    stack=[]
    if len(gram)>4:
        m=line
        if gram[1].strip() not in dic_tid :
             stack.append(["base",0])             
             stack.append([line.strip(),0])
             time1=gram[0].strip()
             
             #print "********************************************8"
             j=0
             dic_tid[gram[1].strip()]=stack
             last_time=time
             time =0
             pid=gram[1].strip()
        else: 
            if pid== gram[1].strip()  :        
              stack=dic_tid[gram[1].strip()]
              p=str(stack.pop())
             # print p.strip(),p_t
              if p!="['base', 0]" : 
                 
                 stack.append(p)
                 pp=stack.pop()
                 time=gram[0].strip()
                 time_d=long(str(time),16)-long(str(time1),16)
                 #print pp+"kk"
                 try:
                    p_t=str(pp).split("',")[1].split("]")[0]
                 except:
                      print pp+"try"
                 p=str(pp).split("['")[1].split("',")[0]
                 time_d=time_d+long(str(p_t))
                 #print p+"ggggg"
                 time1=time
                 #print time,time1,time_d,p_t ,"hhh"   
                 stack.append([p,time_d])
                 #print stack
                 stack.append([line.strip(),0])
                 j=0
                 dic_tid[gram[1].strip()]=stack
              elif p=="['base', 0]":
               
                stack.append(p)
                #stack=dic_tid[gram[2].strip()]
                time1=gram[0].strip()
               #print time1
                j=0
                stack.append([line.strip(),0])
                dic_tid[gram[1].strip()]=stack
            else:
                stack=dic_tid[gram[1].strip()]
                stack.append([line.strip(),0])
                time1=gram[0].strip()
             
                #print "********************************************8"
                j=0
                dic_tid[gram[1].strip()]=stack
                last_time=time
                time =0
                pid=gram[1].strip()
                


        
            
          
            #print time1
    elif len(gram)==4:
         
         if j!=0 :
             stack=dic_tid[gram[1].strip()]
             t=str(stack.pop())
             p_t=str(t).split("',")[1].split("]")[0]
             if t!="['base', 0]":
                #stack1.append(p_t)
                time=gram[0].strip()
                time_d=long(str(time),16)-long(str(time1),16)
                
               # p_t=str(stack1.pop())
               
                time_d=time_d+long(str(p_t))
               # print time,time1,time_d,p_t,"ggg"
                time1=time
               
                p_n=str(t).split("['")[1].split("',")[0]
                
                #print p_n+"111"
                if p_n.split(",")[3].strip()==gram[3].strip() and p_n.split(",")[2].strip()==gram[2].strip():
                   p_a=str(stack.pop())
                   #print p_a
                   if p_a !="['base', 0]":
                    
                      p_na=str(p_a).split("['")[1].split("',")[0]
                      fout.write("PID:"+p_n.split(",")[1].strip()+" TID:"+p_n.split(",")[2].strip()+" CALL_TIME:"+p_n.split(",")[0].strip()+" RETURN_TIME:"+gram[0].strip()+" FROM:"+p_na.split(",")[5].strip()+" TO:"+p_n.split(",")[5].strip()+" AT:NULL"+" TIME:"+str(time_d)+os.linesep)
                      stack.append(p_a)
                   else:
                      fout.write("PID:"+p_n.split(",")[1].strip()+" TID:"+p_n.split(",")[2].strip()+" CALL_TIME:"+p_n.split(",")[0].strip()+" RETURN_TIME:"+gram[0].strip()+" FROM:"+"base"+" TO:"+p_n.split(",")[5].strip()+" AT:NULL"+" TIME:"+str(time_d)+os.linesep)
                      stack.append(p_a)
                else:
                    fout1.write(p_n+" "+line+os.linesep)
                dic_tid[gram[1].strip()]=stack
                
             else:
                  stack.append(t)
                  dic_tid[gram[1].strip()]=stack
                  
         elif j==0:
             try :
                stack=dic_tid[gram[1].strip()]
             except:
                print m,line
                #exit(0)
             
             time=gram[0].strip()
             time_d=long(str(time),16)-long(str(time1),16)
             time1=time
             p_n=str(stack.pop()).split("['")[1].split("',")[0]
             j=j+1
             #print p_n+"2222"
             if p_n.split(",")[3].strip()==gram[3].strip()and p_n.split(",")[2].strip()==gram[2].strip():
                p_a=str(stack.pop())  
                
                if p_a !="['base', 0]":
                   p_na=str(p_a).split("['")[1].split("',")[0]
                   fout.write("PID:"+p_n.split(",")[1].strip()+" TID:"+p_n.split(",")[2].strip()+" CALL_TIME:"+p_n.split(",")[0].strip()+" RETURN_TIME:"+gram[0].strip()+" FROM:"+p_na.split(",")[5].strip()+" TO:"+p_n.split(",")[5].strip()+" AT:NULL"+" TIME:"+str(time_d)+os.linesep)
                   stack.append(p_a)
                else:
                    
                    fout.write("PID:"+p_n.split(",")[1].strip()+" TID:"+p_n.split(",")[2].strip()+" CALL_TIME:"+p_n.split(",")[0].strip()+" RETURN_TIME:"+gram[0].strip()+" FROM:"+"base"+" TO:"+p_n.split(",")[5].strip()+" AT:NULL"+" TIME:"+str(time_d)+os.linesep)
                    
                    stack.append(p_a)
                
                
                dic_tid[gram[1].strip()]=stack
               
             else:
                 fout1.write(p_n+" "+line+os.linesep)
            
fout.close()
print "time over"
             
