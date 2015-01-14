Central Processor Manager
=========================

The *Central Processor Manager* is the local monitoring and control agent for
the Central Processor sub-system. Its role is two-fold:

1) It interfaces to the ASKAP Telescope Operating System (TOS), the telescope
   monitoring and control sub-system

2) It is responsible for orchestrating the components of the Central Processor
   in both data acquisition and processing

At the moment the *Central Processor Manager* only controls the acquisition
phase; that is, it only controls the ingest pipeline.

The Central Processor (CP) Manager interfaces with the following software
components:

* **Ingest Pipeline** - The CP manager controls the startup of the ingest
  pipeline. It also has the ability to abort the ingest pipeline, however under
  typical operation it does not "stop" the ingest pipeline, it stops when the
  metadata flowing from the Telescope Operating System's Executive component
  indicates an observation has concluded.

* **Facility Configuration Manager** - The facility configuration manager (FCM)
  hosts system configuration information, such as antenna positions and some
  Ingest Pipeline configuration. The CP manager retrieves this information and
  uses it to configure the ingest pipeline.

Execution
---------

The *Central Processor Manager* is a standalone Java program. Assuming the
*CLASSPATH* environment variable contains the cpmanager.jar and the required
dependencies it can be started like so::

    java askap.cp.manager.CpManager -c cpmanager.in -l cpmanager.log_cfg 

Command Line Parameters
~~~~~~~~~~~~~~~~~~~~~~~

The CP manager accepts the following command line parameters:

+-------------------+----------------+-------------+----------------------------------------------------------------+
|**Long Form**      |**Short Form**  |**Required** |**Description**                                                 |
+===================+================+=============+================================================================+
| --config          | -c             | Yes         |After this parameter the file containing the program            |
|                   |                |             |configuration must be provided.                                 |
+-------------------+----------------+-------------+----------------------------------------------------------------+
| --log-config      | -l             | No          |After this optional parameter a Log4J configuration file is     |
|                   |                |             |specified. If this option is not set a default logger           |
|                   |                |             |is used.                                                        |
+-------------------+----------------+-------------+----------------------------------------------------------------+

Configuration Parameters
------------------------

The program requires a configuration file be provided on the command line. This
section describes the valid parameters. In addition to the CP manager specific
parameters, any parameters that begin with *ice_properties* are passed directly
to the ICE Communicator during initialisation.

+----------------------------+----------+------------+--------------------------------------------------------------+
|**Parameter**               |**Type**  |**Default** |**Description**                                               |
|                            |          |            |                                                              |
+============================+==========+============+==============================================================+
| ice.servicename            | string   | *None*     |The service name (i.e. Ice object identity) for the CP manager|
|                            |          |            |service interface.                                            |
+----------------------------+----------+------------+--------------------------------------------------------------+
| ice.adaptername            | string   | *None*     |The object adapter identity                                   |
+----------------------------+----------+------------+--------------------------------------------------------------+
| monitoring.enabled         | boolean  | false      |Controls the availability of the Ice monitoring provider      |
|                            |          |            |interface. If enabled an Ice interface permits tools such as  |
|                            |          |            |user interfaces and the monitoring archiver to collect        |
|                            |          |            |monitoring data from the service.                             |
+----------------------------+----------+------------+--------------------------------------------------------------+
| monitoring.ice.servicename | string   | *None*     |If monitoring is enabled, this parameter must be specified.   |
|                            |          |            |This parameter provides the name of the monitoring service    |
|                            |          |            |interface that will be registered in the Ice locator service. |
|                            |          |            |An example would be "CentralProcessoMonitoringService".       |
+----------------------------+----------+------------+--------------------------------------------------------------+
| monitoring.ice.adaptername | string   | *None*     |If monitoring is enabled, this parameter must be specified.   |
|                            |          |            |This parameter provides the name of the adapter on which the  |
|                            |          |            |monitoring service proxy object will be hosted. This adapeter |
|                            |          |            |must be configured in the Ice properties section (see example |
|                            |          |            |below).                                                       |
+----------------------------+----------+------------+--------------------------------------------------------------+
| fcm.ice.identity           | string   | *None*     |The Ice object identity of the Facility Configuration Manager |
|                            |          |            |(FCM). This should be qualified with an adapter name if the   |
|                            |          |            |FCM object is not registered as a "well known object".        |
+----------------------------+----------+------------+--------------------------------------------------------------+
| dataservice.ice.identity   | string   | *None*     |The Ice object identity of the Telescope Operating System     |
|                            |          |            |(TOS) Dataservice. This should be qualified with an adapter   |
|                            |          |            |name if the TOS Dataservice is not registeed as w "well known |
|                            |          |            |object."                                                      |
+----------------------------+----------+------------+--------------------------------------------------------------+
| ingest.workdir             | string   | *None*     |The working directory for the ingest pipeline instance. Within|
|                            |          |            |this directory a sub-directory will be created (one for each  |
|                            |          |            |scheduling block executed) for any output files such as       |
|                            |          |            |observation logs and datasets.                                |
+----------------------------+----------+------------+--------------------------------------------------------------+
| ingest.command             | string   | *None*     |The command required to execute the ingest pipeline.          |
+----------------------------+----------+------------+--------------------------------------------------------------+
| ingest.args                | string   | *None*     |The command line arguments to be passed to the ingest         |
|                            |          |            |pipeline.                                                     |
+----------------------------+----------+------------+--------------------------------------------------------------+

Below are the required ICE parameters:

+---------------------------------------+---------+-----------+-----------------------------------------------------+
|**Parameter**                          |**Type** |**Default**|**Description**                                      |
+=======================================+=========+===========+=====================================================+
|ice_properties.Ice.Default.Locator     | string  | *None*    |Identifies the Ice Locator. This will be of the form:|
|                                       |         |           |*IceGrid/Locator:tcp -h <hostname> -p 4061*          |
+---------------------------------------+---------+-----------+-----------------------------------------------------+
|ice_properties.CentralProcessorAdapter\| string  | *None*    |Configures the adapter endpoint that will host the   |
|.Endpoints                             |         |           |actual manager service. Typically this will be: *tcp*|
+---------------------------------------+---------+-----------+-----------------------------------------------------+
|ice_properties.CentralProcessorAdapter\| string  | *None*    |This is the name of the adapter as it is registered  |
|.AdapterId                             |         |           |in the Ice locator service. This will typically be:  |
|                                       |         |           |*CentralProcessorAdapter*                            |
+---------------------------------------+---------+-----------+-----------------------------------------------------+
|ice_properties.CentralProcessorMonitor\| string  | *None*    |Configures the adapter endpoint that will host the   |
|ingAdapter.Endpoints                   |         |           |monitoring provider interface. Typically this will   |
|                                       |         |           |be: *tcp*                                            |
+---------------------------------------+---------+-----------+-----------------------------------------------------+
|ice_properties.CentralProcessorMonitor\| string  | *None*    |This is the name of the adapter (for the monitoring  |
|ingAdapter.AdapterId                   |         |           |provider interface) as it will be registered in the  |
|                                       |         |           |Ice locator service. This will typically be:         |
|                                       |         |           |*CentralProcessorMonitoringAdapter*                  |
+---------------------------------------+---------+-----------+-----------------------------------------------------+

Example
~~~~~~~

.. code-block:: bash

    ########################## Ice Properties ##############################

    # Registry location
    ice_properties.Ice.Default.Locator                  = IceGrid/Locator:tcp -h aktos01 -p 4061

    # Primary object adapter
    ice_properties.CentralProcessorAdapter.Endpoints    = tcp
    ice_properties.CentralProcessorAdapter.AdapterId    = CentralProcessorAdapter

    # Monitoring object adapter
    ice_properties.CentralProcessorMonitorAdapter.Endpoints    = tcp
    ice_properties.CentralProcessorMonitorAdapter.AdapterId    = CentralProcessorMonitorAdapter

    # Other misc parameters
    ice_properties.Ice.MessageSizeMax                   = 131072
    ice_properties.Ice.ThreadPool.Server.Size           = 4
    ice_properties.Ice.ThreadPool.Server.SizeMax        = 16

    ################## CP Manager Specific Properties ######################

    # Object identity and proxy to use for the CP manager ICE object
    ice.servicename                 = CentralProcessorService
    ice.adaptername                 = CentralProcessorAdapter

    # Monitoring provider configuration
    monitoring.enabled              = true
    monitoring.ice.servicename      = CentralProcessorMonitorService
    monitoring.ice.adaptername      = CentralProcessorMonitorAdapter

    # FCM config
    fcm.ice.identity                = FCMService@FCMAdapter

    # Scheduling block service
    dataservice.ice.identity        = SchedulingBlockService@DataServiceAdapter

    # Ingest working directory
    ingest.workdir                  = /scratch2/datasets

    # Ingest pipeline command and arguments
    ingest.command                  = /askap/cp/cpingest.sh
    ingest.args                     = -s -c cpingest.in -l /askap/cp/config/cpingest.log_cfg
