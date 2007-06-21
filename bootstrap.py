import os

os.system("cd Tools/Dev/setuptools; python bootstrap.py")
os.system("cd Tools/Dev/recursivebuild; python setup.py -q install")
