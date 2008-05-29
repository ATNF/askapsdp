from recursivebuild import build
from recursivebuild.dependencies import q_print

import os
pwd = os.getcwd()
node = os.path.sep.join(pwd.split(os.path.sep)[-2:])
q_print("warn: Not building %s as the clean target fails." %  node)
###build(['trunk/build.py']) 
