#import os
#s = os.system("./procsim -r12 -f4 -j1 -k2 -l3 -p32 < traces/gcc.100k.trace ")
#print s;

import commands
#s = commands.getstatusoutput("./procsim -r12 -f4 -j1 -k2 -l3 -p32 < traces/gcc.100k.trace ")


def getIPC(a):
	splitout = a[1].split('\n')
	line = splitout[len(splitout)-2]
	ipcstring = line.split(': ')
	ipcfloat = float(ipcstring[1])
	return ipcfloat;
#tracename = raw_input("insert trace name: ")
traces = ["gcc", "hmmer", "gobmk", "mcf"]

for tracename in traces:
	r=12
	f=4
	j=3
	k=2
	l=1
	p=32


	#print type(tracename)
	s = commands.getstatusoutput("./procsim -r"+str(r)+" -f"+str(f)+" -j"+str(j)+" -k"+str(k)+" -l"+str(l)+" -p"+str(p)+" < traces/"+tracename+".100k.trace ")

	default = getIPC(s)
	threshold = 0.98*default
	minr = 12
	for r in range(1,13):
		ipc = getIPC(commands.getstatusoutput("./procsim -r"+str(r)+" -f"+str(f)+" -j"+str(j)+" -k"+str(k)+" -l"+str(l)+" -p"+str(p)+" < traces/"+tracename+".100k.trace "))
		if (ipc > threshold):
			minr = r
			break
	sum = f+j+k+l
	r = 12
	minf = f
	minj = j
	mink = k
	minl = l
	for f in range(1,5):
		for j in range(1,4):
			for k in range(1,3):
				ipc = getIPC(commands.getstatusoutput("./procsim -r"+str(r)+" -f"+str(f)+" -j"+str(j)+" -k"+str(k)+" -l"+str(l)+" -p"+str(p)+" < traces/"+tracename+".100k.trace "))
				if(ipc > threshold):
					newsum = f+j+k+l
					if(newsum < sum):
						minf = f
						minj = j
						mink = k
						minl = l
						sum = newsum
						#print minf,minj,mink,minl,newsum

	print tracename,":"
	rob = 2*(minj+mink+minl)
	print "experiment 1\n","f: ",minf,"k0:",minj,"k1:",mink,"k3:",minl,"rob:",rob
	print "experiment 2\n","rob: ",minr,"\n"