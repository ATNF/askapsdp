Before you do anything else you must build the recursivebuild python module. To
do that execute the following commands:
cd recursivebuild
su
[password]
python setup.py install
exit
cd ..

This will install the tiny module in the standard python path so that it can be
in the many setup.py scripts to hierarchically build the repository.
