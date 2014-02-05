Quality evaluation tools
========================


The python script **plotEval.py** provides some graphical feedback on how well the lists match. It produces something like this [attachment:"plotEval.png" image]. The image is displayed in a python image window, and saved to a file **plotEval.png**. The meaning of these plots are as follows:

* Top row: position and flux information:

 * Left: The positional offsets in arcsec for the matched sources. The red ones are those matched first by the Groth algorithm, while the magenta ones are the subsequent matches. Also plotted are red lines indicating the mean x and y offset, and circles at 2, 4, 6, 8 and 10 arcsec (just to provide a guide).
 * Centre: All sources plotted at their position in the field. The red and magenta circles (colours as for top left plot) are sources that appear in both lists and have matching positions. The size of the symbols are scaled by the positional offset (larger symbol = larger offset). The blue symbols are points from **srcFile** that were not matched, while green symbols are points from **refFile** that were not matched.
 * Right: All detected points are plotted with size given by their local image RMS. The distribution of image RMSs is shown in the insert. Colours of the points are as for the previous plot.

* Middle row:

 * Left and centre: The flux differences for the matched sources as a function of position, scaled by either the absolute (left) or relative (middle) size of the flux difference. Here, the squares indicate F~src~ < F~ref~ while circles indicate F~src~ > F~ref~. The distribution of flux differences is also shown, both for absolute and relative values, in each plot.

* Bottom row: shape information:

 * Left: The differences in major axis length for the matched sources as a function of position, with the distributions of differences.
 * Centre: The differences in minor axis length for the matched sources as a function of position, with the distributions of differences.
 * Left: The differences in position angle for the matched sources as a function of position, with the distributions of differences.

The scripts are located in the askap.analysis.evaluation package. This means **DIR** needs to be either **ASKAP_ROOT/Code/Components/Analysis/evaluation/trunk/** or **ASKAP_ROOT/Code/Components/Analysis/evaluation/tags/evaluation-end2end1/**
To run the script, ...

There is a second script, **fluxEval.py** in the same directory that is designed to provide diagnostic plots looking at the fluxes of matched points. It plots the relative difference in the fluxes (fitted - catalogue) against a range of parameters. It produces an image [attachment:fluxEval.png fluxEval.png] and is displayed in a python window. Note that the colours of the points are the same for a given source from one plot to another. It is run in the same way as **plotEval.py**. 

