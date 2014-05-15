`ice-session.py`
================

In a test environment this package provides a script `ice-session.py` which
can be used to create a self contained ice environment with registry.
The session can be terminated with *Ctrl-C* or via a *SIGTERM* and will clean 
up all process. The session will also terminate when any of the subprocesses
terminate.

Example:

.. code-block:: sh

    ice-session.py &
    PID=$!
    echo "doing stuff ..." 
    sleep 10
    kill $PID

The current directory will contain combined stderr/out log files named 
*executable.log* for each executable.

Any application can be a subprocess not only ice-based ones.

Requirements
------------

*ICE_CONFIG* needs to be set in the environment, containing the configuration 
for ALL applications to be run in the session.

Optional
--------

By default `ice-session.py` will only start the registry. To add other 
applications create a text file containing one application andf its arguments 
per line.

.. code-block:: sh
     
     icebox
     fcm.py --log-config=askap.pylog_cfg --config=fcm_init.parset

