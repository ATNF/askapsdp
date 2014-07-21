Correlator Simulator (Playback Mode)
====================================

The purpose of this software is to simulate the ASKAP correlator for the
purposes of testing the central processor. This simulator described here is
actually a playback simulator and relies on other software (e.g. csimulator) to
actually create a simulated measurement set which will be played back by the
software described here.

.. warning:: This software is still in development and should be considered volatile.

Below are the feature highlights:
* Replays data from a measurement set

* Creates a simulated metadata stream (i.e. a simulated TOS)

* Creates a simulated visibility stream (i.e. a simulated correlator)


Running the program
-------------------

The code should be compiled with the ASKAPsoft build system::

    cd $ASKAP_ROOT
    rbuild Code/Components/CP/correlatorsim/current

It can then be run as follows::

    mpirun -np 3 $ASKAP_ROOT/Code/Components/CP/correlatorsim/current/apps/playback.sh -c config.in 

The parameter "-np 3" specifies how many processes to start. This number must be
equal to the number of correlator shelves being simulated, plus one. The number
of correlator shelves being simulated is specified by the
"playback.corrsim.n_shelves" described below.

The first MPI process (rank == 0) is responsible for publishing the TOS metadata
stream, while the rest are responsible for simulating the actual correlator
shelves.


Configuration Parameters
------------------------

Parameter Description
~~~~~~~~~~~~~~~~~~~~~

Note: (1) Where there is no default, this indicates the field is required

+------------------------------------------+------------+--------------+-----------------------------+
|**Parameter**                             |**Type**    |**Default(1)**|**Description**              |
+==========================================+============+==============+=============================+
|playback.tossim.ice.locator_host          |String      |None          |Host name of the ICE locator |
|                                          |            |              |service                      |
+------------------------------------------+------------+--------------+-----------------------------+
|playback.tossim.ice.locator_port          |Integer     |None          |Port number of the ICE       |
|                                          |            |              |locator service              |
+------------------------------------------+------------+--------------+-----------------------------+
|playback.tossim.icestorm.topicmanager     |String      |None          |Name of the topic manager    |
|                                          |            |              |which will handle the        |
|                                          |            |              |metadata stream              |
+------------------------------------------+------------+--------------+-----------------------------+
|playback.tossim.icestorm.topic            |String      |None          |Name of the topic on which to|
|                                          |            |              |publish the metadata stream  |
+------------------------------------------+------------+--------------+-----------------------------+
|playback.corrsim.n_shelves                |Integer     |None          |The number of correlator     |
|                                          |            |              |shelves being simulated      |
+------------------------------------------+------------+--------------+-----------------------------+
|playback.tossim.random_metadata_send_fail |Double      |0.0           |The chance a metadata message|
|                                          |            |              |will not be sent. A failure  |
|                                          |            |              |is simulated by simply not   |
|                                          |            |              |attempting the send. A value |
|                                          |            |              |of of 0.0 results in no      |
|                                          |            |              |failures, while 1.0 results  |
|                                          |            |              |in all message sends failing.|
+------------------------------------------+------------+--------------+-----------------------------+
|playback.corrsim.random_vis_send_fail     |Double      |0.0           |The chance a VisChunk        |
|                                          |            |              |datagram will not be sent. A |
|                                          |            |              |failure is simulated by      |
|                                          |            |              |simply not attempting the    |
|                                          |            |              |send. A value of of 0.0      |
|                                          |            |              |results in no failures, while|
|                                          |            |              |1.0 results in all message   |
|                                          |            |              |sends failing.               |
+------------------------------------------+------------+--------------+-----------------------------+


The following entries must exist for all values of n from 1 to n_shelves inclusive:

+------------------------------------------+------------+--------------+-----------------------------+
|**Parameter**                             |**Type**    |**Default(1)**|**Description**              |
+==========================================+============+==============+=============================+
| playback.corrsim.shelf[n].dataset        | String     | None         |File/path for the measurement|
|                                          |            |              |back                         |
+------------------------------------------+------------+--------------+-----------------------------+
| playback.corrsim.shelf[n].out.hostname   | String     | None         |Hostname or IP address to    |
|                                          |            |              |which the UDP visibility     |
|                                          |            |              |stream will be sent          |
+------------------------------------------+------------+--------------+-----------------------------+
| playback.corrsim.shelf[n].out.port       | Integer    | None         |UDP port number to which the |
|                                          |            |              |visibility stream will be    |
|                                          |            |              |sent                         |
+------------------------------------------+------------+--------------+-----------------------------+


Parameter Example
~~~~~~~~~~~~~~~~~

.. code-block:: bash

    #
    # Telescope Operating Simulator Configuration
    #
    playback.tossim.ice.locator_host        = localhost
    playback.tossim.ice.locator_port        = 4061
    playback.tossim.icestorm.topicmanager   = IceStorm/TopicManager
    playback.tossim.icestorm.topic          = tosmetadata

    #
    # Correlator Simulator Configuration
    #
    playback.corrsim.n_shelves              = 2

    # Correlator Shelf 1
    playback.corrsim.shelf1.dataset         = askap_correlator1.ms
    playback.corrsim.shelf1.out.hostname    = localhost
    playback.corrsim.shelf1.out.port        = 3001

    # Correlator Shelf 2
    playback.corrsim.shelf2.dataset         = askap_correlator2.ms
    playback.corrsim.shelf2.out.hostname    = localhost
    playback.corrsim.shelf2.out.port        = 3002
    </pre>

Input Measurement Sets
----------------------

No checks are made of the measurement sets to ensure they are a reasonable set.
It is expected they are all created by the csimulator and all have the same
parameters, but for different spectral windows. The correlator simulator will
take 1 coarse channel from the measurement set and use the same visibilities for
all 54 fine channels.

An example spectral window configuration (for the csimulator) is::

    spws.Coarse2_0      = [152, 1.420GHz, 1MHz, "XX XY YX YY"]
    spws.Coarse2_1      = [152, 1.572GHz, 1MHz, "XX XY YX YY"]

This reflects the expectation that the BETA correlator will consist of two
shelves each responsible for 8208 (152x54) fine channels.

For a full ASKAP configuration the below configuration is appropriate. The full
ASKAP configuration is expected to consist of 16 correlator shelves, each
responsible for 1026 (19x54) fine channels::

    spws.Coarse16_0     = [19, 1.420GHz, 1MHz, "XX XY YX YY"]
    spws.Coarse16_1     = [19, 1.439GHz, 1MHz, "XX XY YX YY"]
    spws.Coarse16_2     = [19, 1.458GHz, 1MHz, "XX XY YX YY"]
    spws.Coarse16_3     = [19, 1.477GHz, 1MHz, "XX XY YX YY"]
    spws.Coarse16_4     = [19, 1.496GHz, 1MHz, "XX XY YX YY"]
    spws.Coarse16_5     = [19, 1.515GHz, 1MHz, "XX XY YX YY"]
    spws.Coarse16_6     = [19, 1.534GHz, 1MHz, "XX XY YX YY"]
    spws.Coarse16_7     = [19, 1.553GHz, 1MHz, "XX XY YX YY"]
    spws.Coarse16_8     = [19, 1.572GHz, 1MHz, "XX XY YX YY"]
    spws.Coarse16_9     = [19, 1.591GHz, 1MHz, "XX XY YX YY"]
    spws.Coarse16_10    = [19, 1.610GHz, 1MHz, "XX XY YX YY"]
    spws.Coarse16_11    = [19, 1.629GHz, 1MHz, "XX XY YX YY"]
    spws.Coarse16_12    = [19, 1.648GHz, 1MHz, "XX XY YX YY"]
    spws.Coarse16_13    = [19, 1.667GHz, 1MHz, "XX XY YX YY"]
    spws.Coarse16_14    = [19, 1.686GHz, 1MHz, "XX XY YX YY"]
    spws.Coarse16_15    = [19, 1.705GHz, 1MHz, "XX XY YX YY"]
