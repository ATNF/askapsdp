from recursivebuild import build

import os
pwd = os.getcwd()
node = os.path.sep.join(pwd.split(os.path.sep)[-4:])
print("warn: Nothing currently being built in the %s hierarchy." %  node)
