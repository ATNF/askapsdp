Exclusions from Duchamp
=======================

Selavy does not implement all of the features available in Duchamp. This is due to a number of factors:

* We do not have the Duchamp graphics capabilities enabled, since pgplot is not included in the askapsoft code tree.
* The FDR method is not implemented. This method uses all the data to find the optimum threshold, and it is not completely clear how this works in the distributed case, when a given worker only sees part of the data. It could potentially be included in the variable-threshold option, but would be very expensive computationally.
* The baseline-subtraction option has not been impelemented - it would require additional work to pass the baseline values between the workers and the master, and has not been a priority.
* Trimming of blank pixels has not been implemented.
* Reading previously-created smoothed or reconstructed versions of the image has not been implemented.
* Writing of FITS images (smoothed / reconstructed / mask / moment mask) is allowed, but **only** in the case of serial operation **and** when the input image is a FITS image (due to the way the code works - rewriting this has not been a priority). This was implemented to allow the export of the mask file in particular, to allow testing and comparison of the new algorithms.
* Reprocessing a source list from a previous Duchamp/Selavy run has not been made available, although it is possible to write the catalogue to a binary catalogue that can be used in Duchamp (to, for instance, examine the graphical output for detected sources).

The following table lists Duchamp input parameters that will not work in Selavy - if they are included in the input parameter set, a warning message will be written to the log.

+--------------------------+--------------------------------------------------------------+
| *Parameter*              |                    *Reason for exclusion*                    |
+==========================+==============================================================+
|flagPlotSpectra           |No graphics capabilities                                      |
+--------------------------+--------------------------------------------------------------+
|flagPlotIndividualSpectra |No graphics capabilities                                      |
+--------------------------+--------------------------------------------------------------+
|spectraFile               |No graphics capabilities                                      |
+--------------------------+--------------------------------------------------------------+
|flagMaps                  |No graphics capabilities                                      |
+--------------------------+--------------------------------------------------------------+
|detectMap                 |No graphics capabilities                                      |
+--------------------------+--------------------------------------------------------------+
|momentMap                 |No graphics capabilities                                      |
+--------------------------+--------------------------------------------------------------+
|flagXOutput               |No graphics capabilities                                      |
+--------------------------+--------------------------------------------------------------+
|drawBlankEdges            |No graphics capabilities                                      |
+--------------------------+--------------------------------------------------------------+
|spectralMethod            |No graphics capabilities                                      |
+--------------------------+--------------------------------------------------------------+
|flagFDR                   |FDR method not implemented                                    |
+--------------------------+--------------------------------------------------------------+
|alphaFDR                  |FDR method not implemented                                    |
+--------------------------+--------------------------------------------------------------+
|FDRnumCorChan             |FDR method not implemented                                    |
+--------------------------+--------------------------------------------------------------+
|flagBaseline              |Baseline-subtraction not impelemented                         |
+--------------------------+--------------------------------------------------------------+
|flagOutputBaseline        |Baseline-subtraction not impelemented                         |
+--------------------------+--------------------------------------------------------------+
|fileOutputBaseline        |Baseline-subtraction not impelemented                         |
+--------------------------+--------------------------------------------------------------+
|flagReconExists           |Reading of previously-made FITS files not implemented         |
+--------------------------+--------------------------------------------------------------+
|reconFile                 |Reading of previously-made FITS files not implemented         |
+--------------------------+--------------------------------------------------------------+
|flagSmoothExists          |Reading of previously-made FITS files not implemented         |
+--------------------------+--------------------------------------------------------------+
|smoothFile                |Reading of previously-made FITS files not implemented         |
+--------------------------+--------------------------------------------------------------+
|flagTrim                  |Trimming not implemented                                      |
+--------------------------+--------------------------------------------------------------+
|newFluxUnits              |Not implemented - not used in the ASKAP context.              |
+--------------------------+--------------------------------------------------------------+


