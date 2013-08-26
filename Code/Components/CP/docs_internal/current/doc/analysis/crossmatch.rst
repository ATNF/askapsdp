Cross-matching
==============

The **crossmatch** application allows one to match the catalogue of detections produced by Selavy with a known reference catalogue. This reference could be a sky model catalogue, a catalogue from a previous run of Selavy, or a known model sky used in a simulation.

The code to match the list of points is based on the algorithm of `Groth 1986, AJ 91, 1244`_. It matches triangles formed from triplets of points from each list, and finds the set of points that appear in both lists. It can then find the mean offsets in each direction (and even any rotation or expansion that is present). A subsequent search for matches is then done, looking for the closest match (taking into account the mean offsets) out of the reference list for the remaining un-matched sources.

.. _Groth 1986, AJ 91, 1244: http://adsabs.harvard.edu/abs/1986AJ.....91.1244G

The positions are taken as the world coordinates, usually in degrees. Offsets between entries in the two catalogues are calculated using the angular separation (ie. not just flat cartesian separation).

Input parameters
----------------

The main program to do the matching is **crossmatch.sh**, which just runs in serial mode. The parameters it accepts are listed here. The first 6 are required for each of the source and reference catalogues - replace **<cattype>** in the parameter name with **source** or **reference**.

+---------------------+----------+----------------------------+---------------------------------------------------------------------------------------+
|*Parameter*          |*Type*    |*Default*                   |*Description*                                                                          |
+=====================+==========+============================+=======================================================================================+
|<cattype>.filename   |string    |""                          |The file containing the catalogue in question                                          |
+---------------------+----------+----------------------------+---------------------------------------------------------------------------------------+
|<cattype>.database   |string    |Continuum                   |The type of catalogue                                                                  |
+---------------------+----------+----------------------------+---------------------------------------------------------------------------------------+
|<cattype>.trimSize   |int       |0                           |The length to which the point list is truncated prior to calculating the triangles for |
|                     |          |                            |matching. A value of zero means the entire list is used (generating a lot of triangles |
|                     |          |                            |for typical catalogue sizes!).                                                         |
|                     |          |                            |                                                                                       |
+---------------------+----------+----------------------------+---------------------------------------------------------------------------------------+
|<cattype>.ratioLimit |float     |10.                         |The maximum value for the triangle's ratio between its largest and smallest size. See  |
|                     |          |                            |Groth 1986. Default is a good value.                                                   |
|                     |          |                            |                                                                                       |
+---------------------+----------+----------------------------+---------------------------------------------------------------------------------------+
|<cattype>.raRef      |string    |""                          |Reference value for the RA of the catalogue. Source positions used in the calculations |
|                     |          |                            |will be offsets from this.                                                             |
|                     |          |                            |                                                                                       |
+---------------------+----------+----------------------------+---------------------------------------------------------------------------------------+
|<cattype>.decRef     |string    |""                          |Reference value for the Declination of the catalogue. Source positions used in the     |
|                     |          |                            |calculations will be offsets from this.                                                |
|                     |          |                            |                                                                                       |
+---------------------+----------+----------------------------+---------------------------------------------------------------------------------------+
|positionUnits        |string    |deg                         |Units for the source positions. Typically the catalogues will provide RA & Dec in      |
|                     |          |                            |degrees, so no change is necessary. If you have catalogues in pixel values, give this  |
|                     |          |                            |as a blank string.                                                                     |
|                     |          |                            |                                                                                       |
+---------------------+----------+----------------------------+---------------------------------------------------------------------------------------+
|epsilon              |string    |*no default*                |The epsilon parameter used in the Groth algorithm. Essentially an error parameter      |
|                     |          |                            |governing how close points have to be to be called a match. Can be quoted as a string  |
|                     |          |                            |with units, eg. 30arcsec. Calculations will be done in units of the source positions   |
|                     |          |                            |(positionUnits), but offsets between catalogues will be quoted in the same units as    |
|                     |          |                            |epsilon.                                                                               |
|                     |          |                            |                                                                                       |
|                     |          |                            |                                                                                       |
+---------------------+----------+----------------------------+---------------------------------------------------------------------------------------+
|radius               |float     |-1.                         |If positive, only those points within this radius of the reference location will be    |
|                     |          |                            |considered.                                                                            |
+---------------------+----------+----------------------------+---------------------------------------------------------------------------------------+
|matchFile            |string    |matches.txt                 |The output file with the information on the matching source and reference objects      |
|                     |          |                            |                                                                                       |
+---------------------+----------+----------------------------+---------------------------------------------------------------------------------------+
|missFile             |string    |misses.txt                  |The output file with the information on objects in the source and reference lists that |
|                     |          |                            |were not matched                                                                       |
+---------------------+----------+----------------------------+---------------------------------------------------------------------------------------+
|srcSummaryFile       |string    |match-summary-sources.txt   |A summary of the source catalogue, showing which sources got matched and to what.      |
|                     |          |                            |                                                                                       |
+---------------------+----------+----------------------------+---------------------------------------------------------------------------------------+
|refSummaryFile       |string    |match-summary-reference.txt |A summary of the reference catalogue, showing which sources got matched and to what.   |
|                     |          |                            |                                                                                       |
+---------------------+----------+----------------------------+---------------------------------------------------------------------------------------+

The **epsilon** parameter determines how strict or slack the matching algorithm is in determining if a given triangle matches another. A larger value means it will accept a larger mismatch and so should accept more matches. It may also accept more incorrect matches!


Outputs
-------

The application **crossmatch** provides several output files upon completion, specified by the **matchFile**, **missFile**, **srcSummaryFile** and **refSummaryFile** parameters. 

The **matchFile** file contains a list of the matching points from the two input lists, with the type of match (1=matched by the initial pass of the Groth algorithm, 2=subsequent match, given the initial identifications), object ID from each catalogue (a string with the number in the list and the position) and the offset between the two matching catalogue entries (in units of the epsilon parameter). Here is an example:

::

  1  333a 2851a 4.513098
  1  434a 3248a 2.186833
  1  454a 3357a 3.232754
  1  273a 2543a 0.023051

The **missFile** file contains a list of the points that were not matched, from both input lists. Its columns are: the origin of the point (S=source list, R=reference list), the object ID, positions and flux. An example is as follows:

::

 R    1a    193.864    -44.744 0.00200226
 R    1b    193.864    -44.754 0.00009849
 R    2a    194.295    -48.288 0.00294294
 S   12a    191.339    -45.090 0.01122288
 S   58a    190.902    -47.651 0.00946127
 S   67a    190.740    -47.217 0.02138739

Finally, the two summary files, specified by **srcSummaryFile** and **refSummaryFile**, summarise the entire source and reference catalogues respectively, listing the object ID, the matching object ID (or --- if it wasn't matched), plus the object's RA, Dec and flux. Here's an example:

::

   1a 1254a   191.5428  -45.89932 0.016341681
   2a 1257a   191.4461  -44.59917 0.0053537302
   3a 1259a   191.4611  -44.85273 0.0049734898
   4a 1272a   191.4517  -45.07527 0.01785611
   5a 1284a   191.3025  -43.18213 0.025264779
   6a 1286a   191.4785  -45.95161 0.0058576302
   7a   ---   194.3774  -48.96545 0.0052827201
