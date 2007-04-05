import os

os.system("cd Tools/Dev/setuptools; python ez_setup.py; cd ../../..")
os.system("cd Tools/Dev/recursivebuild; python setup.py -q install; cd ../../..")

from recursivebuild import build

build(['Tools/setup.py',
       'ThirdPartyLibraries/setup.py',
       'Code/setup.py'])

