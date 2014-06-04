ADE Parser
==========

XML Tags
++++++++

The follow XML tag attributes are common to all tags

=================   =================================
Common Attributes   Description
=================   =================================
name                name of item
type                type of item
comment             description of item.  May
                    be used to provide text
                    to GUI elements constructed
                    from the tag
=================   =================================

The following XML tags are used


+---------------+-------------------+--------------------------------------------------------+
|Tag            |  Attributes       | Description                                            |
+===============+===================+========================================================+
|iocStructure   |                   | represents a C/C++ style structure                     |
+---------------+-------------------+--------------------------------------------------------+
|               | type="top"        | flags that EPICS records should be                     |
|               |                   | generated with this structure                          |
|               |                   | serving as the root.                                   |
+---------------+-------------------+--------------------------------------------------------+
|               | type="struct"     | can only be used inside a "top" iocStructure           |
|               |                   | .specifies that this iocStructure should be resolved   |
|               |                   | from a iocStructure definition with name="struct"      |
+---------------+-------------------+--------------------------------------------------------+
|iocArray       |                   | represents a std::vector array of iocPoints or         |
|               |                   | iocStructures                                          |
+---------------+-------------------+--------------------------------------------------------+
|               | type="array"      | can only be used inside a "top" iocStructure           |
|               |                   | .specifies that this iocArray should be resolved       |
|               |                   | from a iocArray definition with name="array"           |
+---------------+-------------------+--------------------------------------------------------+
|iocPoint       |                   | the basic point type.  For each iocPoint an EPICS      |
|               |                   | record will be generated                               |
+---------------+-------------------+--------------------------------------------------------+
|               | lookup="enumName" | flags that this iocPoint is an enum style point whose  |
|               |                   | enumeration values are defined by the iocEnum with the |
|               |                   | name attribute of "enumName"                           |
+---------------+-------------------+--------------------------------------------------------+
|               | size="num"        | for std::vector array records elements to treat as     |
|               |                   | and EPICS waveform record                              |
+---------------+-------------------+--------------------------------------------------------+
|               | dir="in"          | a input to the IOC which becomes                       |
|               |                   | an EPICS output record                                 |
+---------------+-------------------+--------------------------------------------------------+
|               | dir="out"         | an output from the IOC which                           |
|               |                   | are EPICS input records.  This is the default          |
+---------------+-------------------+--------------------------------------------------------+
|               | dir="both"        | an input to the IOC with a corresponding readback      |
|               |                   | record.  i.e. two EPICS records are generated          |
+---------------+-------------------+--------------------------------------------------------+
|iocEnum        |                   |                                                        |
+---------------+-------------------+--------------------------------------------------------+
|iocEnumValue   |                   |                                                        |
+---------------+-------------------+--------------------------------------------------------+
|iocEnumBit     |                   |                                                        |
+---------------+-------------------+--------------------------------------------------------+
|iocFunction    |                   | IOC functions serve as the way to tie                  |
|               |                   | EPICS records together with C++ ioc                    |
|               |                   | methods.                                               |
+---------------+-------------------+--------------------------------------------------------+
|               | type="direct"     | this IOC function is a "direct"                        |
|               |                   | function meaning that each contained                   |
|               |                   | iocPoint dir="in" point is intended                    |
|               |                   | to be tied directly to a driver                        |
|               |                   | method                                                 |
+---------------+-------------------+--------------------------------------------------------+
|iocStatus      |                   | A boolean iocPoint which also a parent to              | 
|               |                   | set of points whose status is deterined by this point  |
+---------------+-------------------+--------------------------------------------------------+


Detailed Description
++++++++++++++++++++

direction
---------

The XML is written from point of view of being downstream of the IOC whereas EPICS defines
it's record directions upstream of the IOC.  Therefore inputs to the IOC (dir="in") become
output records in the worlds of EPICS.  Similarly outputs (dir="out" or the default) become
input records (i.e. input from hardware to IOC).

declaration, definition for iocArray, iocStructure

iocStructure
++++++++++++

The iocStructure tag is used to defined C/C++ style structures.
When generating EPICS records from iocStructures, the heirarchical
information is presevered in the PV name


iocArray
++++++++

iocPoint
++++++++

iocEnum
+++++++

iocEnumValue
++++++++++++

iocEnumBit
++++++++++

iocFunction
+++++++++++

iocFunctions are used to define RPC type methods via EPICS records.  The methods
can be used in two different ways

For functions that may take multiple parameters, the iocFunction can be defined like this


progress update callbacks records

Debuging
========

Debugging XML errors
++++++++++++++++++++

extracted_Object.xml & XML Editor

Full Example
============






