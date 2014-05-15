Access to calibrator solutions
==============================

The code abstracts the way it accesses the calibration solution. It allows various ways of storing them (e.g. parset
file, casa table, external database) via a factory class. This page contains documentation how to set up the factory.
The parameters are shared by all synthesis programs and should be prefixed with the actual program name in the usual
form, i.e. **Cimager.xxx**

+--------------------------+--------------+----------------+-------------------------------------------------+
|**Parameter**             |**Type**      |**Default**     |**Description**                                  |
+==========================+==============+================+=================================================+
|calibaccess               |string        |"parset"        |The type of calibration solution source. Only    |
|                          |              |                |"parset" and "table" are currently implemented   |
+--------------------------+--------------+----------------+-------------------------------------------------+
|calibaccess.parset        |string        |"result.dat"    |Parset file name where the calibration solution  |
|                          |              |                |is stored. This parameter is meaningful only if  |
|                          |              |                |**calibaccess="parset"**. The keywords in this   |
|                          |              |                |parset file are given in the form **gain.g11.0.0 |
|                          |              |                |= [0.918308,0.000000]**, where g11 is for the    |
|                          |              |                |first parallel-hand polarisation and g22 is for  |
|                          |              |                |the second. The last numbers are the 0-based     |
|                          |              |                |antenna and beam numbers, and the value is a     |
|                          |              |                |complex value with real and imaginary parts given|
|                          |              |                |as a vector. Leakage parameters are given in the |
|                          |              |                |form **leakage.g12.x.y** and **leakage.g21.x.y**,|
|                          |              |                |respectively.                                    |
+--------------------------+--------------+----------------+-------------------------------------------------+
|calibaccess.table         |string        |"calibdata.tab" |The name of the casa table to be read or created.|
+--------------------------+--------------+----------------+-------------------------------------------------+
|calibaccess.table.reuse   |bool          |false           |Unless this flag is true, an old table or file,  |
|                          |              |                |which name is given by **calibaccess.table** is  |
|                          |              |                |removed if writing of the calibration solution is|
|                          |              |                |requested                                        |
+--------------------------+--------------+----------------+-------------------------------------------------+
|calibaccess.table.maxant  |uint          |36              |Maximum number of antennas allowed (this         |
|                          |              |                |parameter is required only if writing of the new |
|                          |              |                |calibration solutions is requested)              |
+--------------------------+--------------+----------------+-------------------------------------------------+
|calibaccess.table.maxbeam |uint          |30              |Maximum number of beams allowed (this parameter  |
|                          |              |                |is required only if writing of the new           |
|                          |              |                |calibration solutions is requested)              |
+--------------------------+--------------+----------------+-------------------------------------------------+
|calibaccess.table.maxchan |uint          |16416           |Maximum number of spectral channels allowed (this|
|                          |              |                |parameter is required only if writing of the new |
|                          |              |                |calibration solutions is requested)              |
+--------------------------+--------------+----------------+-------------------------------------------------+
