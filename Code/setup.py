from recursivebuild import build

build(['ThirdPartyLibraries/Python/setup.py',
       'Libraries/Python/setup.py',
       'Subsystems/setup.py',
       'Systems/setup.py'])
