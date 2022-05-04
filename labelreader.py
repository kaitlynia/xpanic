import sys

fw=open(sys.argv[1],'r')
m=fw.readlines()
fw.close()
fw=open(sys.argv[1]+".txt",'w')

for i in m:
    f=i.find(r'_("')
    if f+1:
        fw.writelines(i[f+3:i.find(r'")')]+'\n')

fw.close()
print('success to write labels at labels.txt')
