#import os
#s = os.system("./procsim -r12 -f4 -j1 -k2 -l3 -p32 < traces/gcc.100k.trace ")
#print s;

import commands
#s = commands.getstatusoutput("./procsim -r12 -f4 -j1 -k2 -l3 -p32 < traces/gcc.100k.trace ")
s = commands.getstatusoutput("./procsim -r999 -f999 -j999 -k999 -l999 -p999 < traces/gcc.100k.trace ")
print s[1]