/// @file
///
/// @brief Base class for parallel applications
/// @details
/// Supports algorithms by providing methods for initialization
/// of MPI connections, sending and models around.
/// There is assumed to be one master and many workers.
///
/// @copyright (c) 2007 CSIRO
/// Australia Telescope National Facility (ATNF)
/// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
/// PO Box 76, Epping NSW 1710, Australia
/// atnf-enquiries@csiro.au
///
/// This file is part of the ASKAP software distribution.
///
/// The ASKAP software distribution is free software: you can redistribute it
/// and/or modify it under the terms of the GNU General Public License as
/// published by the Free Software Foundation; either version 2 of the License,
/// or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
///
/// @author Matthew Whiting <matthew.whiting@csiro.au>
///

#include <Common/LofarTypedefs.h>
using namespace LOFAR::TYPES;
#include <Blob/BlobString.h>
#include <Blob/BlobIBufString.h>
#include <Blob/BlobOBufString.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>
#include <Common/Exceptions.h>

#include <casa/OS/Timer.h>
#include <casa/Utilities/Regex.h>
#include <casa/BasicSL/String.h>
#include <casa/OS/Path.h>
#include <images/Images/ImageOpener.h>
#include <casa/aipstype.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayPartMath.h>

// boost includes
#include <boost/shared_ptr.hpp>

#include <askap_analysis.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <askapparallel/AskapParallel.h>

#include <parallelanalysis/DuchampParallel.h>
#include <analysisutilities/AnalysisUtilities.h>
#include <analysisutilities/CasaImageUtil.h>
#include <analysisutilities/SubimageDef.h>
#include <sourcefitting/RadioSource.h>
#include <sourcefitting/FittingParameters.h>
#include <analysisutilities/NewArrayMath.h>
#include <analysisutilities/NewArrayPartMath.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <time.h>

#include <duchamp/duchamp.hh>
#include <duchamp/param.hh>
#include <duchamp/Cubes/cubes.hh>
#include <duchamp/Utils/Statistics.hh>
#include <duchamp/Utils/utils.hh>
#include <duchamp/Detection/detection.hh>
#include <duchamp/Detection/columns.hh>
#include <duchamp/PixelMap/Voxel.hh>
#include <duchamp/PixelMap/Object3D.hh>

///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".parallelanalysis");

using namespace std;
using namespace askap;
using namespace askap::mwbase;

using namespace duchamp;

namespace askap {
    namespace analysis {

        //**************************************************************//

        bool DuchampParallel::is2D()
        {
            /// @details Check whether the image is 2-dimensional, by
            /// looking at the dim array in the Cube object, and counting
            /// the number of dimensions that are greater than 1
            /// @todo Make use of the new Cube::is2D() function.
            int numDim = 0;
            long *dim = this->itsCube.getDimArray();

            for (int i = 0; i < this->itsCube.getNumDim(); i++) if (dim[i] > 1) numDim++;

            return numDim <= 2;
        }

        //**************************************************************//

        std::string DuchampParallel::workerPrefix()
        {
            std::stringstream ss;

            if (this->isParallel()) {
                if (this->isMaster())
                    ss << "MASTER: ";
                else if (this->isWorker())
                    ss << "Worker #" << this->itsRank << ": ";
            } else ss << "";

            return ss.str();
        }

        DuchampParallel::DuchampParallel(int argc, const char** argv)
                : AskapParallel(argc, argv)
        {
            this->itsFitter = sourcefitting::FittingParameters(LOFAR::ParameterSet());
        }
        //**************************************************************//

        DuchampParallel::DuchampParallel(int argc, const char** argv,
                                         const LOFAR::ParameterSet& parset)
                : AskapParallel(argc, argv)
        {
            /// @details The constructor reads parameters from the parameter
            /// set parset. This set can include Duchamp parameters, as well
            /// as particular cduchamp parameters such as masterImage and
            /// sectionInfo.

            // First do the setup needed for both workers and master
            this->itsCube.pars() = parseParset(parset);
            this->itsImage = substitute(parset.getString("image"));
            ImageOpener::ImageTypes imageType = ImageOpener::imageType(this->itsImage);
            this->itsIsFITSFile = (imageType == ImageOpener::FITS);
	    this->itsWeightImage = parset.getString("weightimage","");
	    if(this->itsWeightImage != "")
	      ASKAPLOG_INFO_STR(logger, "Using weights image: " << this->itsWeightImage);

            this->itsFlagDoMedianSearch = parset.getBool("doMedianSearch", false);
            this->itsMedianBoxWidth = parset.getInt16("medianBoxWidth", 50);

            this->itsFlagDoFit = parset.getBool("doFit", false);
            this->itsSummaryFile = parset.getString("summaryFile", "duchamp-Summary.txt");
            this->itsSubimageAnnotationFile = parset.getString("subimageAnnotationFile", "");
            this->itsFitAnnotationFile = parset.getString("fitAnnotationFile", "duchamp-Results-Fits.ann");
            LOFAR::ParameterSet fitParset = parset.makeSubset("Fitter.");
            this->itsFitter = sourcefitting::FittingParameters(fitParset);

            if (this->itsFitter.numFitTypes() == 0 && this->itsFlagDoFit)
                ASKAPLOG_WARN_STR(logger, "No valid fit types given, so setting doFit flag to false.");

            this->itsCube.pars().setFlagRobustStats(parset.getBool("flagRobust", true));
            // Now read the correct image name according to worker/master state.
            this->itsCube.pars().setImageFile(this->itsImage);

            if (this->isParallel()) {
                if (this->isMaster()) {
                    this->itsCube.pars().setLogFile(substitute(parset.getString("logFile", "duchamp-Logfile-Master.txt")));
                    this->itsSubimageDef = SubimageDef(parset);
                } else if (this->isWorker()) {
                    this->itsSubimageDef = SubimageDef(parset);
// 		    this->itsCube.pars().setFlagSubsection(true);
		      
                }
            }

            if (this->isWorker())
                this->itsCube.pars().setLogFile(substitute(parset.getString("logFile", "duchamp-Logfile-%w.txt")));

        }


        //**************************************************************//

        int DuchampParallel::getMetadata()
        {
            /// @details Provides a simple front-end to the correct
            /// metadata-reading function, according to whether the image is
            /// FITS data or a CASA image
            /// @return The return value of the function that was used:
            /// either duchamp::SUCCESS or duchamp::FAILURE
            if (this->itsIsFITSFile) return this->itsCube.getMetadata();
            else return casaImageToMetadata(this->itsCube, this->itsSubimageDef, this->itsRank - 1);
        }

        //**************************************************************//

        std::vector<float> DuchampParallel::getBeamInfo()
        {
            /// @details Returns a vector containing the beam parameters:
            /// major axis [deg], minor axis [deg], position angle [deg].
            std::vector<float> beam(3);
            beam[0] = this->itsCube.header().getBmajKeyword();
            beam[1] = this->itsCube.header().getBminKeyword();
            beam[2] = this->itsCube.header().getBpaKeyword();
            return beam;
        }
        //**************************************************************//


        void DuchampParallel::readData()
        {
            /// @details Reads in the data to the duchamp::Cube class. For
            /// the workers, this either uses the duchamp functionality, in
            /// the case of FITS data, or calls the routines in
            /// CasaImageUtil.cc in the case of casa (or other) formats. If
            /// reconstruction or smoothing are required, they are done in
            /// this function. For the master, the metadata only is read
            /// from the file, with the same choice based on the FITS status
            /// of the data file.
            if (this->isParallel() && this->isMaster()) {
                int result;
                ASKAPLOG_INFO_STR(logger,  this->workerPrefix() << "About to read metadata");

                if (this->itsIsFITSFile) {
                    this->itsSubimageDef.defineFITS(this->itsCube.pars().getImageFile());
                    this->itsSubimageDef.setImageDim(getFITSdimensions(this->itsCube.pars().getImageFile()));

                    if (!this->itsCube.pars().getFlagSubsection() || this->itsCube.pars().getSubsection() == "") {
                        this->itsCube.pars().setFlagSubsection(true);
                        this->itsCube.pars().setSubsection(nullSection(this->itsSubimageDef.getImageDim().size()));
                    }

                    if (this->itsCube.pars().verifySubsection() == duchamp::FAILURE)
                        ASKAPTHROW(AskapError, this->workerPrefix() << "Cannot parse the subsection string " << this->itsCube.pars().getSubsection());

                    result = this->itsCube.getMetadata();
		    // check the true dimensionality and set the 2D flag in the cube header.
		    int numDim=0;
		    long *dim = this->itsCube.getDimArray();
		    for(int i=0;i<this->itsCube.getNumDim();i++) if(dim[i]>1) numDim++;
		    this->itsCube.header().set2D(numDim<=2);
		    // set up the various flux units
		    if (this->itsCube.header().getWCS()->spec >= 0) this->itsCube.header().fixUnits(this->itsCube.pars());

                } else {
                    result = casaImageToMetadata(this->itsCube, this->itsSubimageDef, this->itsRank - 1);
                }

                ASKAPLOG_INFO_STR(logger, "Annotation file for subimages is \"" << this->itsSubimageAnnotationFile << "\".");

                if (this->itsSubimageAnnotationFile != "") {
                    ASKAPLOG_INFO_STR(logger, "Writing annotation file showing subimages to " << this->itsSubimageAnnotationFile);
                    this->itsSubimageDef.writeAnnotationFile(this->itsSubimageAnnotationFile, this->itsCube.pars().section(),  this->itsCube.header(), this->itsCube.pars().getImageFile(), this->itsNNode - 1);
                }

                if (result == duchamp::FAILURE) {
                    ASKAPLOG_ERROR_STR(logger, this->workerPrefix() << "Could not read in metadata from image " << this->itsImage << ".");
                    ASKAPTHROW(AskapError, this->workerPrefix() << "Unable to read image " << this->itsImage)
                } else {
                    ASKAPLOG_INFO_STR(logger,  this->workerPrefix() << "Read metadata from image " << this->itsImage);
                }

                ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Dimensions are "
                                      << this->itsCube.getDimX() << " " << this->itsCube.getDimY() << " " << this->itsCube.getDimZ());

                if (this->itsCube.getDimZ() == 1) this->itsCube.pars().setMinChannels(0);

//                 // Send out the OK to the workers, so that they access the file in turn
//                 bool OK;

//                 for (int i = 1; i < this->itsNNode; i++) {
//                     LOFAR::BlobString bs5;
//                     bs5.resize(0);
//                     LOFAR::BlobOBufString bob5(bs5);
//                     LOFAR::BlobOStream out5(bob5);
//                     out5.putStart("goInput", 1);
//                     out5 << i ;
//                     out5.putEnd();
//                     this->itsConnectionSet->writeAll(bs5);
//                     LOFAR::BlobString bs6;
//                     this->itsConnectionSet->read(i - 1, bs6);
//                     LOFAR::BlobIBufString bib6(bs6);
//                     LOFAR::BlobIStream in6(bib6);
//                     int version = in6.getStart("inputDone");
//                     ASKAPASSERT(version == 1);
//                     in6 >> OK;
//                     in6.getEnd();
//                 }

// 		// Broadcast a message to all workers, effectively saying that we're all done
//                 LOFAR::BlobString bs7;
//                 bs7.resize(0);
//                 LOFAR::BlobOBufString bob7(bs7);
//                 LOFAR::BlobOStream out7(bob7);
//                 out7.putStart("goInput", 1);
//                 out7 << this->itsNNode;
//                 out7.putEnd();
//                 this->itsConnectionSet->writeAll(bs7);
                //
                //
            } else if (this->isWorker()) {
                bool OK = true;
//                int rank;

//                 if (this->isParallel()) {
//                     do {
//                         LOFAR::BlobString bs1;
//                         bs1.resize(0);
//                         this->itsConnectionSet->read(0, bs1);
//                         LOFAR::BlobIBufString bib1(bs1);
//                         LOFAR::BlobIStream in1(bib1);
//                         std::stringstream ss;
//                         int version = in1.getStart("goInput");
//                         ASKAPASSERT(version == 1);
//                         in1 >> rank;
//                         in1.getEnd();
//                         OK = (rank == this->itsRank);
//                     } while (!OK);
//                 }

                if (OK) {
                    ASKAPLOG_INFO_STR(logger,  this->workerPrefix() << "About to read data from image " << this->itsCube.pars().getFullImageFile());

                    if (this->itsIsFITSFile) ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Reading with FITS code");
                    else                    ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Reading with CASA code");

                    int result;

                    if (this->itsIsFITSFile) {
                        this->itsSubimageDef.defineFITS(this->itsCube.pars().getImageFile());
                        this->itsSubimageDef.setImageDim(getFITSdimensions(this->itsCube.pars().getImageFile()));

                        if (!this->itsCube.pars().getFlagSubsection() || this->itsCube.pars().getSubsection() == "") {
                            this->itsCube.pars().setFlagSubsection(true);
                            this->itsCube.pars().setSubsection(nullSection(this->itsSubimageDef.getImageDim().size()));
                        }

                        if (this->isParallel()) {
                            duchamp::Section subsection = this->itsSubimageDef.section(this->itsRank - 1, this->itsCube.pars().getSubsection());
                            this->itsCube.pars().setSubsection(subsection.getSection());
			    this->itsCube.pars().setFlagSubsection(true);
                        }

                        if (this->itsCube.pars().verifySubsection() == duchamp::FAILURE)
                            ASKAPTHROW(AskapError, this->workerPrefix() << "Cannot parse the subsection string " << this->itsCube.pars().getSubsection());

                        ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Using subsection " << this->itsCube.pars().getSubsection());
                        ASKAPLOG_INFO_STR(logger,  this->workerPrefix()
                                              << "About to read data from image " << this->itsCube.pars().getFullImageFile());
                        result = this->itsCube.getCube();
                    } else { // if it's a CASA image
// 		      this->itsCube.pars().setFlagSubsection(false); // temporary - casaImageToCube will change this.
                        result = casaImageToCube(this->itsCube, this->itsSubimageDef, this->itsRank - 1);
                    }

                    if (result == duchamp::FAILURE) {
                        ASKAPLOG_ERROR_STR(logger, this->workerPrefix() << "Could not read in data from image " << this->itsImage);
                        ASKAPTHROW(AskapError, this->workerPrefix() << "Unable to read image " << this->itsImage);
                    } else {
                        ASKAPLOG_INFO_STR(logger,  this->workerPrefix() << "Read data from image " << this->itsImage);
                        ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Dimensions are "
                                              << this->itsCube.getDimX() << " " << this->itsCube.getDimY() << " " << this->itsCube.getDimZ());

                        if (this->itsCube.getDimZ() == 1) this->itsCube.pars().setMinChannels(0);
                    }

		if(this->itsWeightImage!=""){
		  ASKAPLOG_INFO_STR(logger, "Applying weights");
		  // use the same spatial section, but only the first plane
		  duchamp::Section wsec = this->itsCube.pars().section();
		  for(int i=2;i<=3;i++) {
		    wsec.setStart(i,0);
		    wsec.setEnd(i,0);
		  }
		  this->itsWeights = getPixelsInBox(this->itsWeightImage, subsectionToSlicer(wsec));
		  casa::Double maxweight = *std::max_element(this->itsWeights.begin(),this->itsWeights.end());
		  for(size_t i=0;i<this->itsWeights.size();i++) this->itsWeights[i] /= maxweight;
		  for(int z=0;z<this->itsCube.getDimZ();z++)
		    for(int i=0;i<this->itsCube.getDimX()*this->itsCube.getDimY();i++) 
		      this->itsCube.getArray()[i+z*this->itsCube.getDimX()*this->itsCube.getDimY()] *= this->itsWeights[i];

		}

                    if (this->itsCube.pars().getFlagATrous()) {
                        ASKAPLOG_INFO_STR(logger,  this->workerPrefix() << "Reconstructing");
                        this->itsCube.ReconCube();
                    } else if (this->itsCube.pars().getFlagSmooth()) {
                        ASKAPLOG_INFO_STR(logger,  this->workerPrefix() << "Smoothing");
                        this->itsCube.SmoothCube();
                    }


//                     // Return the OK to the master to say that we've read the image
//                     if (this->isParallel()) {
//                         LOFAR::BlobString bs2;
//                         bs2.resize(0);
//                         LOFAR::BlobOBufString bob2(bs2);
//                         LOFAR::BlobOStream out2(bob2);
//                         out2.putStart("inputDone", 1);
//                         out2 << OK;
//                         out2.putEnd();
//                         this->itsConnectionSet->write(0, bs2);

// 			// Wait to find out if all workers have finished
//                         do {
//                             LOFAR::BlobString bs3;
//                             bs3.resize(0);
//                             this->itsConnectionSet->read(0, bs3);
//                             LOFAR::BlobIBufString bib3(bs3);
//                             LOFAR::BlobIStream in3(bib3);
//                             std::stringstream ss;
//                             int version = in3.getStart("goInput");
//                             ASKAPASSERT(version == 1);
//                             in3 >> rank;
//                             in3.getEnd();
//                         }  while (rank != this->itsNNode);
//                     }
                } else {
                    ASKAPLOG_ERROR_STR(logger, this->workerPrefix() << "Could not read data from image " << this->itsImage << " as it's not ready.");
                    ASKAPTHROW(AskapError, this->workerPrefix() << "Unable to read image " << this->itsImage);

//                     // Return a message to the master to say that we've failed.
//                     if (isParallel()) {
//                         LOFAR::BlobString bs4;
//                         bs4.resize(0);
//                         LOFAR::BlobOBufString bob4(bs4);
//                         LOFAR::BlobOStream out4(bob4);
//                         out4.putStart("inputDone", 1);
//                         out4 << this->itsRank << false;
//                         out4.putEnd();
//                         this->itsConnectionSet->write(0, bs4);
//                     }
                }



	    }
        }

        //**************************************************************//

        void DuchampParallel::setupLogfile(int argc, const char** argv)
        {
            /// @details Opens the log file and writes the execution
            /// statement, the time, and the duchamp parameter set to it.
            if (this->itsCube.pars().getFlagLog()) {
                ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Setting up logfile " << this->itsCube.pars().getLogFile());
                std::ofstream logfile(this->itsCube.pars().getLogFile().c_str());
                logfile << "New run of the CDuchamp sourcefinder: ";
                time_t now = time(NULL);
                logfile << asctime(localtime(&now));
                // Write out the command-line statement
                logfile << "Executing statement : ";

                for (int i = 0; i < argc; i++) logfile << argv[i] << " ";

                logfile << std::endl;
                logfile << this->itsCube.pars();
                logfile.close();
            }
        }

        //**************************************************************//

        void DuchampParallel::findSources()
        {
            /// @details Searches the image/cube for objects, using the
            /// appropriate search function given the user
            /// parameters. Merging of neighbouring objects is then done,
            /// and all WCS parameters are calculated.
            ///
            /// This is only done on the workers.
            if (this->isWorker()) {
                // remove mininum size criteria, so we don't miss anything on the borders.
                int minpix = this->itsCube.pars().getMinPix();
                int minchan = this->itsCube.pars().getMinChannels();

                if (this->isParallel()) {
                    this->itsCube.pars().setMinPix(1);
                    this->itsCube.pars().setMinChannels(1);
                }


                if (this->itsCube.getSize() > 0) {
                    if (this->itsFlagDoMedianSearch) {
                        ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Searching after median filtering");
                        this->medianSearch2D();
                    } else if (this->itsCube.pars().getFlagATrous()) {
                        ASKAPLOG_INFO_STR(logger,  this->workerPrefix() << "Searching with reconstruction first");
                        this->itsCube.ReconSearch();
                    } else if (this->itsCube.pars().getFlagSmooth()) {
                        ASKAPLOG_INFO_STR(logger,  this->workerPrefix() << "Searching with smoothing first");
                        this->itsCube.SmoothSearch();
                    } else {
                        ASKAPLOG_INFO_STR(logger,  this->workerPrefix() << "Searching, no smoothing or reconstruction done.");
                        this->itsCube.CubicSearch();
                    }
                }

		if(this->itsWeightImage!=""){
		  ASKAPLOG_INFO_STR(logger, "Removing weights");
		  for(int z=0;z<this->itsCube.getDimZ();z++)
		    for(int i=0;i<this->itsCube.getDimX()*this->itsCube.getDimY();i++) 
		      this->itsCube.getArray()[i+z*this->itsCube.getDimX()*this->itsCube.getDimY()] /= this->itsWeights[i];
		}
		  

                // merge the objects, and grow them if necessary.
                this->itsCube.ObjectMerger();
                this->itsCube.calcObjectWCSparams();
                ASKAPLOG_INFO_STR(logger,  this->workerPrefix() << "Found " << this->itsCube.getNumObj() << " objects.");

                if (isParallel()) {
                    this->itsCube.pars().setMinPix(minpix);
                    this->itsCube.pars().setMinChannels(minchan);
                }
            }
        }

        //**************************************************************//

        void DuchampParallel::medianSearch2D()
        {

            ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "About to find median & MADFM arrays, and use these to search");
            casa::IPosition box(2, this->itsMedianBoxWidth, this->itsMedianBoxWidth);
            casa::IPosition shape(2, this->itsCube.getDimX(), this->itsCube.getDimY());

	    int spatSize = this->itsCube.getDimX() * this->itsCube.getDimY();
	    float *snrAll = new float[this->itsCube.getSize()];
	    long *imdim = new long[2];
	    imdim[0] = this->itsCube.getDimX(); imdim[1] = this->itsCube.getDimY();
	    duchamp::Image *chanIm = new duchamp::Image(imdim);

	    for(int z=0; z < this->itsCube.getDimZ(); z++){

	      chanIm->extractImage(this->itsCube, z);
	      casa::Array<Float> base(shape, chanIm->getArray());
	      ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Getting sliding median with box halfwidth = " << this->itsMedianBoxWidth);
	      casa::Array<Float> median = slidingArrayMath(base, box, MedianFunc<Float>());
	      ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Getting sliding MADFM with box halfwidth = " << this->itsMedianBoxWidth);
	      casa::Array<Float> madfm = slidingArrayMath(base, box, MadfmFunc<Float>()) / Statistics::correctionFactor;
	      ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Constructing SNR map");
	      casa::Array<Float> snr = (base - median);

	      // Make sure we don't divide by the zeros around the edge of madfm. Need to set those values to S/N=0.
	      uInt ntotal = snr.nelements();
	      Bool snrDelete, madfmDelete;
	      Float *snrStorage = snr.getStorage(snrDelete);
	      Float *ss = snrStorage;
	      const Float *madfmStorage = madfm.getStorage(madfmDelete);
	      const Float *ms = madfmStorage;
	      
	      while (ntotal--) {
		if (*ms > 0) *ss++ /= *ms++;
                else {
		  *ss++ = 0.;
		  ms++;
                }
	      }
	      
	      snr.putStorage(snrStorage, snrDelete);
	      madfm.freeStorage(madfmStorage, madfmDelete);

	      for(int i=0;i<spatSize;i++) snrAll[i+z*spatSize] = snr.data()[i];
	    }

            ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Saving SNR map");
            this->itsCube.saveRecon(snrAll, this->itsCube.getSize());
            this->itsCube.setReconFlag(true);

            if (!this->itsCube.pars().getFlagUserThreshold()) {
                ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Setting user threshold to " << this->itsCube.pars().getCut());
                this->itsCube.pars().setThreshold(this->itsCube.pars().getCut());
                this->itsCube.pars().setFlagUserThreshold(true);
            }

            ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Searching SNR map");
            this->itsCube.ReconSearch();
        }

        //**************************************************************//

        void DuchampParallel::fitSources()
        {
            /// @details The list of RadioSource objects is populated: one
            /// for each of the detected objects. If the 2D profile fitting
            /// is requested, all sources that are not on the image boundary
            /// are fitted by the RadioSource::fitGauss(float *, long *)
            /// function. The fitting for those on the boundary is left for
            /// the master to do after they have been combined with objects
            /// from other subimages.
            ///
            /// @todo Make the boundary determination smart enough to know
            /// which side is adjacent to another subimage.
            if (this->isWorker()) {
                // don't do fit if we have a spectral axis.
                bool flagIs2D = !this->itsCube.header().canUseThirdAxis() || this->is2D();
                this->itsFlagDoFit = this->itsFlagDoFit && flagIs2D;

                if (this->itsFlagDoFit)
                    ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Fitting source profiles.");

                duchamp::FitsHeader head = this->itsCube.getHead();
                float threshold = this->itsCube.stats().getThreshold();

                if (this->itsCube.pars().getFlagGrowth()) {
                    if (this->itsCube.pars().getFlagUserGrowthThreshold())
                        threshold = std::min(threshold, this->itsCube.pars().getGrowthThreshold());
                    else
                        threshold = std::min(threshold, this->itsCube.stats().snrToValue(this->itsCube.pars().getGrowthCut()));
                }

		long numObj = this->itsCube.getNumObj();

                for (int i = 0; i < numObj; i++) {
                    if (this->itsFlagDoFit)
                        ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Setting up source #" << i + 1 << " / " << numObj << ".");

                    sourcefitting::RadioSource src(this->itsCube.getObject(i));

                    // Fix S/Nmax for the case where we've used the medianSearch algorithm and define the effective detection threshold
                    float thresholdForFitting;

                    if (this->itsFlagDoMedianSearch) {
                        std::vector<PixelInfo::Voxel> voxSet = src.getPixelSet();
                        std::vector<PixelInfo::Voxel>::iterator vox = voxSet.begin();
                        float maxSNR = this->itsCube.getReconValue(vox->getX(), vox->getY(), vox->getZ());
                        thresholdForFitting = this->itsCube.getPixValue(vox->getX(), vox->getY(), vox->getZ());

                        for (; vox < voxSet.end(); vox++) {
                            maxSNR = std::max(maxSNR, this->itsCube.getReconValue(vox->getX(), vox->getY(), vox->getZ()));
                            thresholdForFitting = std::min(thresholdForFitting, this->itsCube.getPixValue(vox->getX(), vox->getY(), vox->getZ()));
                        }

                        src.setPeakSNR(maxSNR);
                    } else thresholdForFitting = threshold;

                    // Set up parameters for fitting.
                    src.setNoiseLevel(this->itsCube, this->itsFitter);
                    src.setDetectionThreshold(thresholdForFitting);
                    src.setHeader(head);
                    src.defineBox(this->itsCube.pars().section(), this->itsFitter, this->itsCube.header().getWCS()->spec);
                    // Only do fit if object is not next to boundary
                    src.setAtEdge(this->itsCube, this->itsSubimageDef, this->itsRank - 1);

                    if (this->itsNNode == 1) src.setAtEdge(false);

                    if (!src.isAtEdge() && this->itsFlagDoFit) {
                        ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Fitting source #" << i + 1 << " / " << numObj << ".");
                        src.fitGauss(this->itsCube.getArray(), this->itsCube.getDimArray(), this->itsFitter);
			src.findAlpha(this->itsImage);
			src.findBeta(this->itsImage);
                    }

                    this->itsSourceList.push_back(src);
                }
            }
        }

        //**************************************************************//

        void DuchampParallel::sendObjects()
        {
            /// @details The RadioSource objects on each worker, which
            /// contain each detected object, are sent to the Master node
            /// via LOFAR Blobs.
            ///
            /// In the non-parallel case, we put together a voxel list. Not
            /// sure whether this is necessary at this point.
            /// @todo Sort out voxelList necessity.
            if (this->isWorker()) {
	      int32 num = this->itsCube.getNumObj();
	      int16 rank = this->itsRank;

                if (this->isParallel()) {
                    LOFAR::BlobString bs;
                    bs.resize(0);
                    LOFAR::BlobOBufString bob(bs);
                    LOFAR::BlobOStream out(bob);
                    out.putStart("detW2M", 1);
                    out << rank << num;
                    // send the start positions of the subimage
                    out << this->itsCube.pars().section().getStart(0)
                        << this->itsCube.pars().section().getStart(1)
                        << this->itsCube.pars().section().getStart(this->itsCube.header().getWCS()->spec);
                    std::vector<sourcefitting::RadioSource>::iterator src = this->itsSourceList.begin();

                    for (; src < this->itsSourceList.end(); src++) {
                        // for each RadioSource object, send to master
                        out << *src;

                        if (src->isAtEdge()) {
                            int xmin, xmax, ymin, ymax, zmin, zmax;
                            xmin = std::max(0 , int(src->boxXmin()));
                            xmax = std::min(this->itsCube.getDimX() - 1, src->boxXmax());
                            ymin = std::max(0 , int(src->boxYmin()));
                            ymax = std::min(this->itsCube.getDimY() - 1, src->boxYmax());

                            if (this->is2D()) {
                                zmin = zmax = 0;
                            } else {
                                zmin = std::max(0 , int(src->boxZmin()));
                                zmax = std::min(this->itsCube.getDimZ() - 1, src->boxZmax());
                            }

                            int numVox = (xmax - xmin + 1) * (ymax - ymin + 1) * (zmax - zmin + 1);
                            out << numVox << this->itsFlagDoMedianSearch;

                            for (int32 x = xmin; x <= xmax; x++) {
                                for (int32 y = ymin; y <= ymax; y++) {
                                    for (int32 z = zmin; z <= zmax; z++) {
                                        bool inObject = src->pixels().isInObject(x, y, z);
                                        float flux = this->itsCube.getPixValue(x, y, z);
                                        out << inObject << x << y << z << flux;

                                        if (this->itsFlagDoMedianSearch) out << this->itsCube.getReconValue(x, y, z);
                                    }
                                }
                            }
                        }
                    }

                    out.putEnd();
                    this->itsConnectionSet->write(0, bs);
                    ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Sent detection list to the master");
                } else {
                }
            }
        }

        //**************************************************************//

        void DuchampParallel::receiveObjects()
        {
            /// @details On the Master node, receive the list of RadioSource
            /// objects sent by the workers. Also receives the list of
            /// detected and surrounding voxels - these will be used to
            /// calculate parameters of any merged boundary sources.
            /// @todo Voxellist is really only needed for the boundary sources.
            if (!this->isParallel() || this->isMaster()) {
                ASKAPLOG_INFO_STR(logger,  this->workerPrefix() << "Retrieving lists from workers");

                if (this->isParallel()) {
                    LOFAR::BlobString bs;
                    int16 rank;
		    int32 numObj;

		    // don't do fit if we have a spectral axis.
		    bool flagIs2D = !this->itsCube.header().canUseThirdAxis() || this->is2D();
		    this->itsFlagDoFit = this->itsFlagDoFit && flagIs2D;

                    for (int i = 1; i < this->itsNNode; i++) {
                        this->itsConnectionSet->read(i - 1, bs);
                        LOFAR::BlobIBufString bib(bs);
                        LOFAR::BlobIStream in(bib);
                        int version = in.getStart("detW2M");
                        ASKAPASSERT(version == 1);
                        in >> rank >> numObj;
                        ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Starting to read "
                                              << numObj << " objects from worker #" << rank);
                        int xstart, ystart, zstart;
                        in >> xstart >> ystart >> zstart;

                        for (int obj = 0; obj < numObj; obj++) {
                            sourcefitting::RadioSource src;
                            in >> src;
                            // Correct for any offsets.
                            // If the full cube is a subsection of a larger one, then we need to correct for what the master offsets are.
                            src.setXOffset(xstart - this->itsCube.pars().getXOffset());
                            src.setYOffset(ystart - this->itsCube.pars().getYOffset());
                            src.setZOffset(zstart - this->itsCube.pars().getZOffset());
                            src.addOffsets();
                            src.calcParams();

                            for (unsigned int f = 0; f < src.fitset("best").size(); f++) {
                                src.fitset("best")[f].setXcenter(src.fitset("best")[f].xCenter() + src.getXOffset());
                                src.fitset("best")[f].setYcenter(src.fitset("best")[f].yCenter() + src.getYOffset());
                            }

                            // And now set offsets to those of the full image as we are in the master cube
                            src.setOffsets(this->itsCube.pars());
                            src.defineBox(this->itsCube.pars().section(), this->itsFitter, this->itsCube.header().getWCS()->spec);
                            src.fitparams() = this->itsFitter;
                            this->itsSourceList.push_back(src);

                            if (src.isAtEdge()) {
                                int numVox;
                                bool haveSNRvalues;
                                in >> numVox >> haveSNRvalues;

                                for (int p = 0; p < numVox; p++) {
                                    int32 x, y, z;
                                    float flux, snr;
                                    bool inObj;
                                    in >> inObj >> x >> y >> z >> flux;

                                    if (haveSNRvalues) in >> snr;

                                    x += (xstart - this->itsCube.pars().getXOffset());
                                    y += (ystart - this->itsCube.pars().getYOffset());
                                    z += (zstart - this->itsCube.pars().getZOffset());
                                    PixelInfo::Voxel vox(x, y, z, flux);
                                    this->itsVoxelList.push_back(vox);

                                    if (haveSNRvalues) {
                                        PixelInfo::Voxel snrvox(x, y, z, snr);
                                        this->itsSNRVoxelList.push_back(snrvox);
                                    }
                                }
                            }
                        }

                        ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Received list of size "
                                              << numObj << " from worker #" << rank);
                        ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Now have "
                                              << this->itsSourceList.size() << " objects");
                        in.getEnd();
                    }
                }
            }
        }

        //**************************************************************//

        void DuchampParallel::cleanup()
        {
            /// @details Done on the Master node. This function gathers the
            /// sources that are marked as on the boundary of subimages, and
            /// combines them via the duchamp::Cubes::ObjectMerger()
            /// function. The resulting sources are then fitted (if so
            /// required) and have their WCS parameters calculated by the
            /// calcObjectParams() function.
            ///
            /// Once this is done, these sources are added to the cube
            /// detection list, along with the non-boundary objects. The
            /// final list of RadioSource objects is then sorted (by the
            /// Name field) and given object IDs.
            if (!this->isParallel() || this->isMaster()) {
                ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Beginning the cleanup");
                std::vector<sourcefitting::RadioSource> backuplist = this->itsSourceList;
                std::vector<sourcefitting::RadioSource> edgeSources, goodSources;
                std::vector<sourcefitting::RadioSource>::iterator src;

//  ASKAPLOG_DEBUG_STR(logger, "Source list size = " << this->itsSourceList.size());
                for (src = this->itsSourceList.begin(); src < this->itsSourceList.end(); src++) {
                    if (src->isAtEdge()) edgeSources.push_back(*src);
                    else goodSources.push_back(*src);
                }

                this->itsSourceList.clear();
                duchamp::FitsHeader head = this->itsCube.getHead();

                float threshold;

                if (this->itsCube.pars().getFlagUserThreshold()) threshold = this->itsCube.pars().getThreshold();
                else threshold = this->itsCube.stats().getThreshold();

//  ASKAPLOG_DEBUG_STR(logger, "Good list size = " << goodSources.size() << " Edge list size = " << edgeSources.size());
                if (edgeSources.size() > 0) { // if there are edge sources
                    for (src = edgeSources.begin(); src < edgeSources.end(); src++) this->itsCube.addObject(*src);

                    ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "num edge sources in cube = " << this->itsCube.getNumObj());
                    bool growthflag = this->itsCube.pars().getFlagGrowth();
                    this->itsCube.pars().setFlagGrowth(false);
                    this->itsCube.ObjectMerger();
                    this->itsCube.pars().setFlagGrowth(growthflag);
                    this->calcObjectParams();
                    ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "num edge sources in cube after merging = " << this->itsCube.getNumObj());

                    for (long i = 0; i < this->itsCube.getNumObj(); i++) {
                        if(this->itsFlagDoFit)
			  ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Fitting source #" << i + 1 << "/" << this->itsCube.getNumObj() << ".");

			sourcefitting::RadioSource src(this->itsCube.getObject(i));

                        // Fix S/Nmax for the case where we've used the medianSearch algorithm: the edge sources will be incorrect at this point.
                        // Also find the effective detection threshold
                        float thresholdForFitting=0.;

                        if (this->itsFlagDoMedianSearch) {
                            std::vector<PixelInfo::Voxel> voxSet = src.getPixelSet();
                            std::vector<PixelInfo::Voxel>::iterator vox = voxSet.begin();
                            float maxSNR=0.;

                            for (; vox < voxSet.end(); vox++) {
                                std::vector<PixelInfo::Voxel>::iterator pixvox = this->itsVoxelList.begin();

                                while (pixvox < this->itsVoxelList.end() && !vox->match(*pixvox)) pixvox++;

                                if (pixvox == this->itsVoxelList.end())
                                    ASKAPLOG_ERROR_STR(logger, "Missing a voxel in the pixel list comparison: (" << vox->getX() << "," << vox->getY() << ")");

                                if (vox == voxSet.begin()) thresholdForFitting = pixvox->getF();
                                else thresholdForFitting = std::min(thresholdForFitting, pixvox->getF());

                                //
                                std::vector<PixelInfo::Voxel>::iterator snrvox = this->itsSNRVoxelList.begin();

                                while (snrvox < this->itsSNRVoxelList.end() && !vox->match(*snrvox)) snrvox++;

                                if (snrvox == this->itsSNRVoxelList.end())
                                    ASKAPLOG_ERROR_STR(logger, "Missing a voxel in the SNR list comparison: (" << vox->getX() << "," << vox->getY() << ")");

                                if (vox == voxSet.begin()) maxSNR = snrvox->getF();
                                else maxSNR = std::max(maxSNR, snrvox->getF());
                            }

                            src.setPeakSNR(maxSNR);
                        } else thresholdForFitting = threshold;

                        float noise = findSurroundingNoise(this->itsCube.pars().getImageFile(), src.getXPeak(), src.getYPeak(), this->itsFitter.noiseBoxSize());
                        src.setNoiseLevel(noise);
                        src.setDetectionThreshold(thresholdForFitting);
                        src.setHeader(head);
                        src.defineBox(this->itsCube.pars().section(), this->itsFitter, this->itsCube.header().getWCS()->spec);
			
                        if (this->itsFlagDoFit){
			  src.fitGauss(&this->itsVoxelList, this->itsFitter);
			  src.findAlpha(this->itsImage);
			  src.findBeta(this->itsImage);
			}

                        this->itsSourceList.push_back(src);
                    }
                }

                ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Finished cleaning up edge sources");

                for (src = goodSources.begin(); src < goodSources.end(); src++) {
                    src->setHeader(head);

                    // Need to check that there are no small sources present that violate the minimum size criteria
                    if ((src->hasEnoughChannels(this->itsCube.pars().getMinChannels()))
                            && (src->getSpatialSize() >= this->itsCube.pars().getMinPix()))
                        this->itsSourceList.push_back(*src);
                }

                std::stable_sort(this->itsSourceList.begin(), this->itsSourceList.end());
                this->itsCube.clearDetectionList();

                for (src = this->itsSourceList.begin(); src < this->itsSourceList.end(); src++) {
                    src->setID(src - this->itsSourceList.begin() + 1);
//          if(this->itsCube.objAtSpatialEdge(*src)) src->addToFlagText("E");
                    src->setAtEdge(this->itsCube, this->itsSubimageDef, this->itsRank - 1);

                    if (src->isAtEdge()) src->addToFlagText("E");
                    else src->addToFlagText("-");

                    this->itsCube.addObject(duchamp::Detection(*src));
                }

                ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Finished adding sources to cube. Now have " << this->itsCube.getNumObj() << " objects.");

            }
        }

        //**************************************************************//

        void DuchampParallel::calcObjectParams()
        {
            /// @details A function to calculate object parameters
            /// (including WCS parameters), making use of the this->itsVoxelList
            /// set of voxels. The function finds the voxels that appear in
            /// each object in itsCube, making a vector of vectors of
            /// voxels, then passes this vector to
            /// duchamp::Cube::calcObjectWCSparams().
            int numVox = this->itsVoxelList.size();
            int numObj = this->itsCube.getNumObj();

            if (numObj > 0) {
                std::vector<PixelInfo::Voxel> templist[numObj];

                for (int i = 0; i < this->itsCube.getNumObj(); i++) {
		  
		  //		  if(this->itsCube.getObject(i).hasParams()){ // only do this for sources that have been merged. Those that haven't have already had their parameters calculated.

                    // for each object, make a vector list of voxels that appear in it.
                    std::vector<PixelInfo::Voxel>
		      objVoxList = this->itsCube.getObject(i).getPixelSet();
                    std::vector<PixelInfo::Voxel>::iterator vox;

                    // get the fluxes of each voxel
                    for (vox = objVoxList.begin(); vox < objVoxList.end(); vox++) {
		      int ct = 0;

		      while (ct < numVox && !vox->match(this->itsVoxelList[ct])) {
			ct++;
		      }

		      if (numVox != 0 && ct == numVox) { // there has been no match -- problem!
			ASKAPLOG_ERROR_STR(logger, this->workerPrefix() << "Found a voxel ("
					   << vox->getX() << "," << vox->getY() << ") in the object lists that doesn't appear in the base list.");
		      } else vox->setF(this->itsVoxelList[ct].getF());
                    }

                    templist[i] = objVoxList;
		    //		  }
		}

                std::vector< std::vector<PixelInfo::Voxel> > bigVoxSet(templist, templist + numObj);
                this->itsCube.calcObjectWCSparams(bigVoxSet);
            }
        }

        //**************************************************************//

        void DuchampParallel::printResults()
        {
            /// @details The final list of detected objects is written to
            /// the terminal and to the results file in the standard Duchamp
            /// manner.
            if (this->isMaster()) {
                ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Found " << this->itsCube.getNumObj() << " sources.");

                this->itsCube.prepareOutputFile();
                //  if(this->itsCube.getNumObj()>0){
                //    // no flag-setting, as it's hard to do when we don't have
                //    // all the pixels. Particularly the negative flux flags
                //    this->itsCube.sortDetections();
                //  }
                this->itsCube.outputDetectionList();

                if (this->itsCube.pars().getFlagKarma()) {
                    std::ofstream karmafile(this->itsCube.pars().getKarmaFile().c_str());
                    this->itsCube.outputDetectionsKarma(karmafile);
                    karmafile.close();
                }

                std::vector<std::string> outtypes = sourcefitting::availableFitTypes;
                outtypes.push_back("best");
                std::vector<duchamp::Column::Col> columns = this->itsCube.getFullCols();

                for (unsigned int t = 0; t < outtypes.size(); t++) {
                    std::ofstream summaryFile(sourcefitting::convertSummaryFile(this->itsSummaryFile.c_str(), outtypes[t]).c_str());
                    std::vector<sourcefitting::RadioSource>::iterator src = this->itsSourceList.begin();

                    for (; src < this->itsSourceList.end(); src++)
                        src->printSummary(summaryFile, columns, outtypes[t], src == this->itsSourceList.begin());

                    summaryFile.close();
                }

                if (this->itsFlagDoFit) this->writeFitAnnotation();

            } else {
            }
        }

        //**************************************************************//


        void DuchampParallel::writeFitAnnotation()
        {
            /// @details This function writes a Karma annotation file
            /// showing the location and shape of the fitted 2D Gaussian
            /// components. It makes use of the
            /// RadioSource::writeFitToAnnotationFile() function. The file
            /// written to is given by the input parameter
            /// fitAnnotationFile.
            if (this->itsSourceList.size() > 0) {
                std::ofstream outfile(this->itsFitAnnotationFile.c_str());
                ASKAPLOG_INFO_STR(logger, "Writing to annotation file: " << this->itsFitAnnotationFile);
                outfile << "COLOR BLUE\n";
                outfile << "COORD W\n";
                outfile << "PA SKY\n";
                outfile << "FONT lucidasans-12\n";
                std::vector<sourcefitting::RadioSource>::iterator src;

                for (src = this->itsSourceList.begin(); src < this->itsSourceList.end(); src++) {
                    outfile << "# Source " << int(src - this->itsSourceList.begin()) + 1 << ":\n";
                    src->writeFitToAnnotationFile(outfile);
                }

                outfile.close();
            }
        }

        //**************************************************************//

        void DuchampParallel::gatherStats()
        {
            /// @details A front-end function that calls all the statistics
            /// functions. Net effect is to find the mean/median and
            /// rms/MADFM for the entire dataset and store these values in
            /// the master's itsCube statsContainer.
            if (!this->itsFlagDoMedianSearch &&
                    (!this->itsCube.pars().getFlagUserThreshold() ||
                     (this->itsCube.pars().getFlagGrowth() && !this->itsCube.pars().getFlagUserGrowthThreshold()))) {
                findMeans();
                combineMeans();
                broadcastMean();
                findRMSs();
                combineRMSs();
            } else {
                if (this->itsFlagDoMedianSearch) this->itsCube.stats().setThreshold(this->itsCube.pars().getCut());
                else this->itsCube.stats().setThreshold(this->itsCube.pars().getThreshold());
            }
        }


        //**************************************************************//

        void DuchampParallel::findMeans()
        {
            /// @details In the parallel case, this finds the mean or median
            /// (according to the flagRobustStats parameter) of the worker's
            /// image/cube, then sends that value to the master via LOFAR
            /// Blobs.
            ///
            /// In the serial (non-parallel) case, all stats for the cube
            /// are calculated in the standard manner via the
            /// duchamp::Cube::setCubeStats() function.
            if (this->isWorker()) {
                if (this->isParallel()) {
                    ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Finding mean");

                    if (this->itsCube.pars().getFlagATrous()) this->itsCube.ReconCube();
                    else if (this->itsCube.pars().getFlagSmooth()) this->itsCube.SmoothCube();

                    int32 size = this->itsCube.getSize();
                    float mean = 0., rms;
                    float *array;
                    // make a mask in case there are blank pixels.
                    bool *mask = this->itsCube.pars().makeStatMask(this->itsCube.getArray(), this->itsCube.getDimArray());

                    if (size > 0) {
                        if (this->itsCube.pars().getFlagATrous())       array = this->itsCube.getArray();
                        else if (this->itsCube.pars().getFlagSmooth()) array = this->itsCube.getRecon();
                        else                                     array = this->itsCube.getArray();

                        // calculate both mean & rms, but ignore rms for the moment.
                        if (this->itsCube.pars().getFlagRobustStats()) findMedianStats(array, size, mask, mean, rms);
                        else                                    findNormalStats(array, size, mask, mean, rms);
                    }

                    double dmean = mean;
                    ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Mean = " << mean);
                    LOFAR::BlobString bs;
                    bs.resize(0);
                    LOFAR::BlobOBufString bob(bs);
                    LOFAR::BlobOStream out(bob);
                    out.putStart("meanW2M", 1);
                    int16 rank = this->itsRank;
                    out << rank << dmean << size;
                    out.putEnd();
                    this->itsConnectionSet->write(0, bs);
                    ASKAPLOG_INFO_STR(logger, "Sent mean to the master from worker " << this->itsRank);
                } else {
                    // serial case -- can just calculate all stats at once.
                    ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Calculating stats");
                    this->itsCube.setCubeStats();
                    ASKAPLOG_INFO_STR(logger, "Stats are as follows:");
                    std::cout << this->itsCube.stats();
                }
            } else {
            }
        }

        //**************************************************************//

        void DuchampParallel::findRMSs()
        {
            /// @details In the parallel case, this finds the rms or the
            /// median absolute deviation from the median (MADFM) (dictated
            /// by the flagRobustStats parameter) of the worker's
            /// image/cube, then sends that value to the master via LOFAR
            /// Blobs. To calculate the rms/MADFM, the mean of the full
            /// dataset must be read from the master (again passed via LOFAR
            /// Blobs). The calculation uses the findSpread() function.
            ///
            /// In the serial case, nothing is done, as we have already
            /// calculated the rms in the findMeans() function.
            if (this->isWorker() && this->isParallel()) {
                ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "About to calculate rms");
                // first read in the overall mean for the cube
                double mean = 0;

                if (isParallel()) {
                    LOFAR::BlobString bs1;
                    this->itsConnectionSet->read(0, bs1);
                    LOFAR::BlobIBufString bib(bs1);
                    LOFAR::BlobIStream in(bib);
                    int version = in.getStart("meanM2W");
                    ASKAPASSERT(version == 1);
                    in >> mean;
                    in.getEnd();
                } else {
                    mean = this->itsCube.stats().getMiddle();
                }

                // use it to calculate the rms for this section
                int32 size = this->itsCube.getSize();
                double rms = 0.;
                float *array;

                if (size > 0) {
                    if (this->itsCube.pars().getFlagATrous()) {
                        array = new float[size];

                        for (int i = 0; i < size; i++) array[i] = this->itsCube.getPixValue(i) - this->itsCube.getReconValue(i);
                    } else if (this->itsCube.pars().getFlagSmooth()) array = this->itsCube.getRecon();
                    else array = this->itsCube.getArray();

                    bool *mask = this->itsCube.pars().makeStatMask(array, this->itsCube.getDimArray());
                    rms = findSpread(this->itsCube.pars().getFlagRobustStats(), mean, size, array, mask);
                }

                if (size > 0 && this->itsCube.pars().getFlagATrous()) delete [] array;

                ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "rms = " << rms);
                // return it to the master
                LOFAR::BlobString bs2;
                bs2.resize(0);
                LOFAR::BlobOBufString bob(bs2);
                LOFAR::BlobOStream out(bob);
                out.putStart("rmsW2M", 1);
                int16 rank = this->itsRank;
                out << rank << rms << size;
                out.putEnd();
                this->itsConnectionSet->write(0, bs2);
                ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Sent local rms to the master");
            } else {
            }
        }

        //**************************************************************//

        void DuchampParallel::combineMeans()
        {
            /// @details The master reads the mean/median values from each
            /// of the workers, and combines them to form the mean/median of
            /// the full dataset. Note that if the median of the workers
            /// data has been provided, the values are treated as estimates
            /// of the mean, and are combined as if they were means (ie. the
            /// overall value is the weighted (by size) average of the
            /// means/medians of the individual images). The value is stored
            /// in the StatsContainer in itsCube.
            if (this->isMaster() && this->isParallel()) {
                // get the means from the workers
                ASKAPLOG_INFO_STR(logger,  this->workerPrefix() << "Receiving Means and combining");
                LOFAR::BlobString bs1;
                int64 size = 0;
                double av = 0;

                for (int i = 1; i < itsNNode; i++) {
                    this->itsConnectionSet->read(i - 1, bs1);
                    LOFAR::BlobIBufString bib(bs1);
                    LOFAR::BlobIStream in(bib);
                    int version = in.getStart("meanW2M");
                    ASKAPASSERT(version == 1);
                    double newav;
                    int32 newsize;
                    int16 rank;
                    in >> rank >> newav >> newsize;
                    in.getEnd();
                    ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Received mean from worker " << rank);
                    size += newsize;
                    av += newav * newsize;
                }

                if (size > 0) {
                    av /= double(size);
                }

                ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "OVERALL SIZE = " << size);
                ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "OVERALL MEAN = " << av);
                this->itsCube.stats().setMean(av);
            } else {
            }
        }

        //**************************************************************//

        void DuchampParallel::broadcastMean()
        {
            /// @details The mean/median value of the full dataset is sent
            /// via LOFAR Blobs to the workers.
            if (this->isMaster() && this->isParallel()) {
                // now send the overall mean to the workers so they can calculate the rms
                double av = this->itsCube.stats().getMean();
                LOFAR::BlobString bs2;
                bs2.resize(0);
                LOFAR::BlobOBufString bob(bs2);
                LOFAR::BlobOStream out(bob);
                out.putStart("meanM2W", 1);
                out << av;
                out.putEnd();
                this->itsConnectionSet->writeAll(bs2);
                ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Broadcast overal mean from master to workers.");
            } else {
            }
        }

        //**************************************************************//

        void DuchampParallel::combineRMSs()
        {
            /// @details The master reads the rms/MADFM values from each of
            /// the workers, and combines them to produce an estimate of the
            /// rms for the full cube. Again, if MADFM values have been
            /// calculated on the workers, they are treated as estimates of
            /// the rms and are combined as if they are rms values. The
            /// overall value is stored in the StatsContainer in itsCube.
            if (this->isMaster() && this->isParallel()) {
                // get the means from the workers
                ASKAPLOG_INFO_STR(logger,  this->workerPrefix() << "Receiving RMS values and combining");
                LOFAR::BlobString bs;
                int64 size = 0;
                double rms = 0;

                for (int i = 1; i < itsNNode; i++) {
                    this->itsConnectionSet->read(i - 1, bs);
                    LOFAR::BlobIBufString bib(bs);
                    LOFAR::BlobIStream in(bib);
                    int version = in.getStart("rmsW2M");
                    ASKAPASSERT(version == 1);
                    double newrms;
                    int32 newsize;
                    int16 rank;
                    in >> rank >> newrms >> newsize;
                    in.getEnd();
                    ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Received RMS from worker " << rank);
                    size += newsize;
                    rms += (newrms * newrms * (newsize - 1));
                }

                if (size > 0) {
                    rms = sqrt(rms / double(size - 1));
                }

                this->itsCube.stats().setStddev(rms);
                this->itsCube.stats().setRobust(false);

                if (!this->itsCube.pars().getFlagUserThreshold()) {
                    this->itsCube.stats().setThresholdSNR(this->itsCube.pars().getCut());
                    this->itsCube.pars().setFlagUserThreshold(true);
                    this->itsCube.pars().setThreshold(this->itsCube.stats().getThreshold());
                }

                ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "OVERALL RMS = " << rms);
            }
        }

        //**************************************************************//

        void DuchampParallel::broadcastThreshold()
        {
            /// @details The detection threshold value (which has been
            /// already calculated) is sent to the workers via LOFAR Blobs.
            if (this->isMaster() && this->isParallel()) {
                // now send the overall mean to the workers so they can calculate the rms
                LOFAR::BlobString bs;
                bs.resize(0);
                LOFAR::BlobOBufString bob(bs);
                LOFAR::BlobOStream out(bob);
                out.putStart("threshM2W", 1);
                double threshold = this->itsCube.stats().getThreshold();
                double mean = this->itsCube.stats().getMiddle();
                double rms = this->itsCube.stats().getSpread();
                out << threshold << mean << rms;
                out.putEnd();
                this->itsConnectionSet->writeAll(bs);
                ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Sent threshold ("
                                      << this->itsCube.stats().getThreshold() << ") from the master");
            } else {
            }
        }


        //**************************************************************//

        void DuchampParallel::receiveThreshold()
        {
            /// @details The workers read the detection threshold sent via LOFAR Blobs from the master.
            if (this->isWorker()) {
                double threshold, mean, rms;

                if (this->isParallel()) {
                    LOFAR::BlobString bs;
                    this->itsConnectionSet->read(0, bs);
                    LOFAR::BlobIBufString bib(bs);
                    LOFAR::BlobIStream in(bib);
                    int version = in.getStart("threshM2W");
                    ASKAPASSERT(version == 1);
                    in >> threshold >> mean >> rms;
                    in.getEnd();
                    this->itsCube.stats().setRobust(false);
                    this->itsCube.stats().setMean(mean);
                    this->itsCube.stats().setStddev(rms);

                    if (!this->itsCube.pars().getFlagUserThreshold() && !this->itsFlagDoMedianSearch)
                        ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Setting mean to be " << mean << " and rms " << rms);

                    this->itsCube.pars().setFlagUserThreshold(true);
                } else {
                    if (this->itsCube.pars().getFlagUserThreshold())
                        threshold = this->itsCube.pars().getThreshold();
                    else
                        threshold = this->itsCube.stats().getMiddle() + this->itsCube.stats().getSpread() * this->itsCube.pars().getCut();
                }

                ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Setting threshold to be " << threshold);
                this->itsCube.pars().setThreshold(threshold);
            }
        }


    }
}
