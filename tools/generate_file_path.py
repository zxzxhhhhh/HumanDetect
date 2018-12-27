import os

def absoluteFilePaths(directory):
   for dirpath,_,filenames in os.walk(directory):
       for f in filenames:
           yield os.path.abspath(os.path.join(dirpath, f)) # return a iterable generator, minimize the memory use.


pwd = os.getcwd()

path = absoluteFilePaths(pwd)

for i in path:
	print(i)



#############################################################################

# os.listdir() will get you everything that's in a directory - files and directories.

# with open("Output.txt", "w") as text_file:
#     print("Purchase Amount: {}".format(TotalAmount), file=text_file)
import os
path = os.listdir()
with open('file.txt', 'w') as f:
    for item in path:
        f.write("%s\n" % item)