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
            this->itsFitParams = sourcefitting::FittingParameters(LOFAR::ParameterSet());
	    this->itsWeighter = new Weighter(*this);
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
            ImageOpener::ImageTypes imageType = ImageOpener::imageType(this->itsCube.pars().getImageFile());
            this->itsIsFITSFile = (imageType == ImageOpener::FITS);
	    bool useCasa = parset.getBool("useCASAforFITS",true);
	    this->itsIsFITSFile = this->itsIsFITSFile && !useCasa;
	    bool flagSubsection = parset.getBool("flagSubsection",false);
	    this->itsBaseSubsection = parset.getString("subsection","");
	    if(!flagSubsection) this->itsBaseSubsection = "";
            this->itsWeightImage = parset.getString("weightsimage", "");

            if (this->itsWeightImage != ""){
                ASKAPLOG_INFO_STR(logger, "Using weights image: " << this->itsWeightImage);
		this->itsWeighter = new Weighter(*this);
	    }

            this->itsFlagDoMedianSearch = parset.getBool("doMedianSearch", false);
            this->itsMedianBoxWidth = parset.getInt16("medianBoxWidth", 50);

            this->itsFlagDoFit = parset.getBool("doFit", false);
	    this->itsFlagDistribFit = parset.getBool("distribFit",true);
            this->itsFlagFitJustDetection = parset.getBool("fitJustDetection", false);
            this->itsFlagFindSpectralIndex = parset.getBool("findSpectralIndex", false);
	    if(parset.isDefined("summaryFile")){
	      this->itsFitSummaryFile = parset.getString("summaryFile", "duchamp-fitResults.txt");
	      ASKAPLOG_WARN_STR(logger, "We've changed the name of the 'summaryFile' parameter to 'fitResultsFile'. Using your parameter " << this->itsFitSummaryFile << " for now, but please change your parset!");
	    }
            this->itsFitSummaryFile = parset.getString("fitResultsFile", "duchamp-fitResults.txt");
            this->itsFitAnnotationFile = parset.getString("fitAnnotationFile", "duchamp-fitResults.ann");
            this->itsFitBoxAnnotationFile = parset.getString("fitBoxAnnotationFile", this->itsFitAnnotationFile);
            LOFAR::ParameterSet fitParset = parset.makeSubset("Fitter.");
            this->itsFitParams = sourcefitting::FittingParameters(fitParset);
	    this->itsFitParams.useBoxFlux(!this->itsFlagFitJustDetection);

            if (this->itsFitParams.numFitTypes() == 0 && this->itsFlagDoFit)
                ASKAPLOG_WARN_STR(logger, "No valid fit types given, so setting doFit flag to false.");

            this->itsSubimageAnnotationFile = parset.getString("subimageAnnotationFile", "");

            if (this->isParallel()) {
                if (this->isMaster()) {
                    this->itsCube.pars().setLogFile(substitute(parset.getString("logFile", "duchamp-Logfile-Master.txt")));
                    this->itsSubimageDef = SubimageDef(parset);
                } else if (this->isWorker()) {
                    this->itsSubimageDef = SubimageDef(parset);
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
                    int numDim = 0;
                    long *dim = this->itsCube.getDimArray();

                    for (int i = 0; i < this->itsCube.getNumDim(); i++) if (dim[i] > 1) numDim++;

                    this->itsCube.header().set2D(numDim <= 2);

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
                    ASKAPLOG_ERROR_STR(logger, this->workerPrefix() << "Could not read in metadata from image " << this->itsCube.pars().getImageFile() << ".");
                    ASKAPTHROW(AskapError, this->workerPrefix() << "Unable to read image " << this->itsCube.pars().getImageFile())
                } else {
                    ASKAPLOG_INFO_STR(logger,  this->workerPrefix() << "Read metadata from image " << this->itsCube.pars().getImageFile());
                }

                ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Dimensions are "
                                      << this->itsCube.getDimX() << " " << this->itsCube.getDimY() << " " << this->itsCube.getDimZ());

                if (this->itsCube.getDimZ() == 1) this->itsCube.pars().setMinChannels(0);

            } else if (this->isWorker()) {

                ASKAPLOG_INFO_STR(logger,  this->workerPrefix() << "About to read data from image " << this->itsCube.pars().getFullImageFile());

                if (this->itsIsFITSFile) ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Reading with FITS code");
                else                     ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Reading with CASA code");

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
                    result = casaImageToCube(this->itsCube, this->itsSubimageDef, this->itsRank - 1);
                }

                if (result == duchamp::FAILURE) {
                    ASKAPLOG_ERROR_STR(logger, this->workerPrefix() << "Could not read in data from image " << this->itsCube.pars().getImageFile());
                    ASKAPTHROW(AskapError, this->workerPrefix() << "Unable to read image " << this->itsCube.pars().getImageFile());
                } else {
                    ASKAPLOG_INFO_STR(logger,  this->workerPrefix() << "Read data from image " << this->itsCube.pars().getImageFile());
                    ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Dimensions are "
                                          << this->itsCube.getDimX() << " " << this->itsCube.getDimY() << " " << this->itsCube.getDimZ());

                    if (this->itsCube.getDimZ() == 1) this->itsCube.pars().setMinChannels(0);
                }

		if(this->itsCube.pars().getFlagNegative()){
		  ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Inverting cube");
		  this->itsCube.invert();
		}

                if (this->itsCube.pars().getFlagATrous()) {
                    ASKAPLOG_INFO_STR(logger,  this->workerPrefix() << "Reconstructing");
                    this->itsCube.ReconCube();
                } else if (this->itsCube.pars().getFlagSmooth()) {
                    ASKAPLOG_INFO_STR(logger,  this->workerPrefix() << "Smoothing");
                    this->itsCube.SmoothCube();
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
            /// This is only done on the workers, although if we use
            /// the weight search the master needs to do the
            /// initialisation of itsWeighter.
	  if(this->isMaster()){
	    if(!this->itsFlagDoMedianSearch && this->itsWeightImage != ""){
		      this->itsWeighter->initialise(this->itsWeightImage, this->itsCube.pars().section(), !(this->isParallel()&&this->isMaster()));
	    }
	  }	      
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
                        // this->medianSearch2D();
                        this->medianSearch();
		    } else if (this->itsWeightImage != "" ){
		      ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Searching after weighting");
		      this->itsWeighter->initialise(this->itsWeightImage, this->itsCube.pars().section());
		      this->weightSearch();
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
       void DuchampParallel::weightSearch()
      {
	
	ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Allocating SNR array");
	float *snrAll = new float[this->itsCube.getSize()];
	ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Defining SNR array");
	for(size_t i=0; i<size_t(this->itsCube.getSize());i++){
	  snrAll[i] = this->itsCube.getPixValue(i)*this->itsWeighter->weight(i);
	}
	ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Saving SNR map");
	this->itsCube.saveRecon(snrAll, this->itsCube.getSize());
	this->itsCube.setReconFlag(true);
		
	ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Searching SNR map to threshold " << this->itsCube.stats().getThreshold());
	this->itsCube.ObjectList() = searchReconArray(this->itsCube.getDimArray(),this->itsCube.getArray(),this->itsCube.getRecon(),this->itsCube.pars(),this->itsCube.stats());
	this->itsCube.updateDetectMap();
	if(this->itsCube.pars().getFlagLog())
	  this->itsCube.logDetectionList();
	
	delete [] snrAll;
	delete this->itsWeighter; // don't need it anymore.
      }

      
      void findSNR(float *input, float *output, casa::IPosition shape, casa::IPosition box, int loc, bool isSpatial, int spatSize, int specSize)
      {
	casa::Array<Float> base(shape, input);
	// ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Getting sliding median with box halfwidth = " << this->itsMedianBoxWidth << " for z = " << z);
	casa::Array<Float> median = slidingArrayMath(base, box, MedianFunc<Float>());
	// ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Getting sliding MADFM with box halfwidth = " << this->itsMedianBoxWidth << " for z = " << z);
	casa::Array<Float> madfm = slidingArrayMath(base, box, MadfmFunc<Float>()) / Statistics::correctionFactor;
	// ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Constructing SNR map");
	casa::Array<Float> snr = (base - median);
	
	// Make sure we don't divide by the zeros around the edge of madfm. Need to set those values to S/N=0.
	Float* madfmData=madfm.data();
	Float *snrData=snr.data();
	int imax = isSpatial ? spatSize : specSize;
	for(int i=0;i<imax;i++){
	  int pos;
	  if(isSpatial) pos = i+loc*spatSize;
	  else pos = loc+i*spatSize;
	  if(madfmData[i]>0) output[pos] = snrData[i]/madfmData[i];
	  else output[pos] = 0.;
	}
	
      }


      void DuchampParallel::medianSearch()
      {
	ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "About to find median & MADFM arrays, and use these to search");
	int spatSize = this->itsCube.getDimX() * this->itsCube.getDimY();
	int specSize = this->itsCube.getDimZ();
	float *snrAll = new float[this->itsCube.getSize()];
	long *imdim = new long[2];
	
	if(this->itsCube.pars().getSearchType()=="spatial"){
	  casa::IPosition box(2, this->itsMedianBoxWidth, this->itsMedianBoxWidth);
	  casa::IPosition shape(2, this->itsCube.getDimX(), this->itsCube.getDimY());
	  imdim[0] = this->itsCube.getDimX(); imdim[1] = this->itsCube.getDimY();
	  duchamp::Image *chanIm = new duchamp::Image(imdim);
	  for (int z = 0; z < specSize; z++) {
	    chanIm->extractImage(this->itsCube, z);
	    findSNR(chanIm->getArray(),snrAll,shape,box,z,true,spatSize,specSize);
	  }
	}
	else if(this->itsCube.pars().getSearchType()=="spectral"){
	  casa::IPosition box(1, this->itsMedianBoxWidth);
	  casa::IPosition shape(1, this->itsCube.getDimZ());
	  imdim[0] = this->itsCube.getDimZ(); imdim[1] = 1;
	  duchamp::Image *chanIm = new duchamp::Image(imdim);
	  for (int i = 0; i < spatSize; i++) {
	    chanIm->extractSpectrum(this->itsCube, i);
	    findSNR(chanIm->getArray(),snrAll,shape,box,i,false,spatSize,specSize);
	  }
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
	this->itsCube.ObjectList() = searchReconArray(this->itsCube.getDimArray(),this->itsCube.getArray(),this->itsCube.getRecon(),this->itsCube.pars(),this->itsCube.stats());
	this->itsCube.updateDetectMap();
	if(this->itsCube.pars().getFlagLog())
	  this->itsCube.logDetectionList();
	
	delete [] snrAll;
	delete [] imdim;
      }

      


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

            for (int z = 0; z < this->itsCube.getDimZ(); z++) {

                chanIm->extractImage(this->itsCube, z);
                casa::Array<Float> base(shape, chanIm->getArray());
                ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Getting sliding median with box halfwidth = " << this->itsMedianBoxWidth << " for z = " << z);
                casa::Array<Float> median = slidingArrayMath(base, box, MedianFunc<Float>());
                ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Getting sliding MADFM with box halfwidth = " << this->itsMedianBoxWidth << " for z = " << z);
                casa::Array<Float> madfm = slidingArrayMath(base, box, MadfmFunc<Float>()) / Statistics::correctionFactor;
                ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Constructing SNR map");
                casa::Array<Float> snr = (base - median);

                // Make sure we don't divide by the zeros around the edge of madfm. Need to set those values to S/N=0.
		Float* madfmData=madfm.data();
		Float *snrData=snr.data();
		for(int i=0;i<spatSize;i++){
		  if(madfmData[i]>0) snrAll[i+z*spatSize] = snrData[i]/madfmData[i];
		  else snrAll[i+z*spatSize] = 0.;
		}
		ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "SNR map @ z="<<z<<" has max of " << *std::max_element(snrAll+z*spatSize,snrAll+(z+1)*spatSize) << " and min of " << *std::min_element(snrAll+z*spatSize,snrAll+(z+1)*spatSize));

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
            // this->itsCube.ReconSearch();
	    this->itsCube.ObjectList() = searchReconArray(this->itsCube.getDimArray(),this->itsCube.getArray(),this->itsCube.getRecon(),this->itsCube.pars(),this->itsCube.stats());
	    this->itsCube.updateDetectMap();
	    if(this->itsCube.pars().getFlagLog())
	      this->itsCube.logDetectionList();


	    delete [] snrAll;
	    delete [] imdim;
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

                for (int i = 0; i < this->itsCube.getNumObj(); i++) {
                    if (this->itsFlagDoFit)
                        ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Setting up source #" << i + 1 << " / " << this->itsCube.getNumObj() 
					  << ", size " << this->itsCube.getObject(i).getSize() 
					  << ", peaking at (x,y)=("<<this->itsCube.getObject(i).getXPeak()+this->itsCube.getObject(i).getXOffset()
					  << "," << this->itsCube.getObject(i).getYPeak()+this->itsCube.getObject(i).getYOffset() << ")");

                    sourcefitting::RadioSource src(this->itsCube.getObject(i));

		    this->prepareSourceForFit(src,true);
		    // Only do fit if object is not next to boundary
		    src.setAtEdge(this->itsCube, this->itsSubimageDef, this->itsRank - 1);
		    
		    if (this->itsNNode == 1) src.setAtEdge(false);

		    if (!src.isAtEdge() && this->itsFlagDoFit) 
		      this->fitSource(src, true);

                    this->itsSourceList.push_back(src);
                }
            }
        }

        //**************************************************************//

      void DuchampParallel::prepareSourceForFit(sourcefitting::RadioSource &src, bool useArray)
      {

	// Set up parameters for fitting.
	if(useArray) src.setNoiseLevel(this->itsCube, this->itsFitParams);
	else {
	  // if need to use the surrounding noise, we have to go extract it from the image
	  if (this->itsFitParams.useNoise() // && !this->itsCube.pars().getFlagUserThreshold()
	      ) {
	    float noise = findSurroundingNoise(this->itsCube.pars().getImageFile(), src.getXPeak(), src.getYPeak(), this->itsFitParams.noiseBoxSize());
	    src.setNoiseLevel(noise);
	  } else src.setNoiseLevel(1);
	}

	if(useArray || !this->itsFlagDoMedianSearch) src.setDetectionThreshold(this->itsCube, this->itsFlagDoMedianSearch);
	else src.setDetectionThreshold(this->itsVoxelList, this->itsSNRVoxelList, this->itsFlagDoMedianSearch);

	src.setHeader(this->itsCube.getHead());
	src.defineBox(this->itsCube.pars().section(), this->itsFitParams, this->itsCube.header().getWCS()->spec);

      }

       //**************************************************************//

      void DuchampParallel::fitSource(sourcefitting::RadioSource &src, bool useArray)
      {

	if (this->itsFlagFitJustDetection) {
	  ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Fitting to detected pixels");
	  std::vector<PixelInfo::Voxel> voxlist;
	  if(useArray) voxlist = src.getPixelSet(this->itsCube.getArray(), this->itsCube.getDimArray());
	  else {
	    voxlist = src.getPixelSet();
	    std::vector<PixelInfo::Voxel>::iterator vox,voxcomp;
	    for ( vox=voxlist.begin(); vox < voxlist.end(); vox++) {
	      voxcomp = this->itsVoxelList.begin();
		
	      while (voxcomp < this->itsVoxelList.end() && !vox->match(*voxcomp))
		voxcomp++;
		
	      if (voxcomp == this->itsVoxelList.end())
		ASKAPLOG_ERROR_STR(logger, "Voxel lists mismatch: source pixel " << *vox << " does not have a match");
	      else vox->setF(voxcomp->getF());
	    }
	  }
	  //	  ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Calling fit funtion with voxel list of size " << voxlist.size() << " cf source size of " << src.getSize());
	  src.fitGaussNew(&voxlist, this->itsFitParams);
	} else {
	  //	  ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Fitting to set of surrounding pixels");
	  if(useArray) src.fitGauss(this->itsCube.getArray(), this->itsCube.getDimArray(), this->itsFitParams);
	  else src.fitGauss(&this->itsVoxelList, this->itsFitParams);
	}

	src.findAlpha(this->itsCube.pars().getImageFile(), this->itsFlagFindSpectralIndex);
	src.findBeta(this->itsCube.pars().getImageFile(), this->itsFlagFindSpectralIndex);

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
                                        bool inObject = src->isInObject(x, y, z);
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
		src.defineBox(this->itsCube.pars().section(), this->itsFitParams, this->itsCube.header().getWCS()->spec);
		src.fitparams() = this->itsFitParams;
		this->itsSourceList.push_back(src);

		if (src.isAtEdge()) {
		  int numVox;
		  bool haveSNRvalues;
		  in >> numVox >> haveSNRvalues;

		  for (int p = 0; p < numVox; p++) {
		    int32 x, y, z;
		    float flux, snr;
		    /// @todo Remove inObj as not used.
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

	if(this->isParallel() && this->isMaster()) 
	  ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Distributing full voxel list, of size " << this->itsVoxelList.size() << " to the workers.");
	else if(this->isParallel() && this->isWorker())
	  ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "About to receive the voxel list from the Master");
	this->distributeVoxelList();
	if(this->isParallel() && this->isMaster()) 
	  ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Voxel list distributed");

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
	  if(this->isParallel() && this->isWorker()){
	    // need to call calcObjectParams only, so that the distributed calculation works
	    ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Calculating the parameters in a distributed manner via calcObjectParams()");
	    this->calcObjectParams();
	  }


            if (!this->isParallel() || this->isMaster()) {
                ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Beginning the cleanup");
                std::vector<sourcefitting::RadioSource> edgeSources, goodSources;
                std::vector<sourcefitting::RadioSource>::iterator src;

                for (src = this->itsSourceList.begin(); src < this->itsSourceList.end(); src++) {
                    if (src->isAtEdge()) edgeSources.push_back(*src);
                    else goodSources.push_back(*src);
                }

                this->itsSourceList.clear();
		this->itsCube.clearDetectionList();
                duchamp::FitsHeader head = this->itsCube.getHead();

                float threshold;

                if (this->itsCube.pars().getFlagUserThreshold()) threshold = this->itsCube.pars().getThreshold();
                else threshold = this->itsCube.stats().getThreshold();

                if (edgeSources.size() > 0) { // if there are edge sources
                    for (src = edgeSources.begin(); src < edgeSources.end(); src++) this->itsCube.addObject(*src);

                    ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "num edge sources in cube = " << this->itsCube.getNumObj());
                    bool growthflag = this->itsCube.pars().getFlagGrowth();
                    this->itsCube.pars().setFlagGrowth(false);  // can't grow as don't have flux array in itsCube
                    ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Merging edge sources");
                    this->itsCube.ObjectMerger();
                    ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "num edge sources in cube after merging = " << this->itsCube.getNumObj());
                    this->itsCube.pars().setFlagGrowth(growthflag);
		    this->calcObjectParams();
                    ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "num edge sources in cube after param calcs = " << this->itsCube.getNumObj());

                    for (long i = 0; i < this->itsCube.getNumObj(); i++) {
                        sourcefitting::RadioSource src(this->itsCube.getObject(i));

			if(!this->itsFlagDistribFit){
			  if (this->itsFlagDoFit)
                            ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Fitting source #" << i + 1 << "/" << this->itsCube.getNumObj() << ".");

			  // Fix S/Nmax for the case where we've used the medianSearch algorithm: the edge sources will be incorrect at this point.
			  // Also find the effective detection threshold
			  float thresholdForFitting = 0.;

			  if (this->itsFlagDoMedianSearch) {
                            std::vector<PixelInfo::Voxel> voxSet = src.getPixelSet();
                            std::vector<PixelInfo::Voxel>::iterator vox = voxSet.begin();
                            float maxSNR = 0.;

                            for (; vox < voxSet.end(); vox++) {
			      std::vector<PixelInfo::Voxel>::iterator pixvox = this->itsVoxelList.begin();

			      while (pixvox < this->itsVoxelList.end() && !vox->match(*pixvox)) pixvox++;

			      if (pixvox == this->itsVoxelList.end())
				ASKAPLOG_ERROR_STR(logger, "Missing a voxel in the pixel list comparison: (" << vox->getX() << "," << vox->getY() << ")");

			      if (vox == voxSet.begin()) thresholdForFitting = pixvox->getF();
			      else thresholdForFitting = std::min(thresholdForFitting, pixvox->getF());

			      std::vector<PixelInfo::Voxel>::iterator snrvox = this->itsSNRVoxelList.begin();

			      while (snrvox < this->itsSNRVoxelList.end() && !vox->match(*snrvox)) snrvox++;

			      if (snrvox == this->itsSNRVoxelList.end())
				ASKAPLOG_ERROR_STR(logger, "Missing a voxel in the SNR list comparison: (" << vox->getX() << "," << vox->getY() << ")");

			      if (vox == voxSet.begin()) maxSNR = snrvox->getF();
			      else maxSNR = std::max(maxSNR, snrvox->getF());
                            }

                            src.setPeakSNR(maxSNR);
			  } else thresholdForFitting = threshold;

			  if (this->itsFitParams.useNoise() // && !this->itsCube.pars().getFlagUserThreshold()
			      ) {
                            float noise = findSurroundingNoise(this->itsCube.pars().getImageFile(), src.getXPeak(), src.getYPeak(), this->itsFitParams.noiseBoxSize());
                            src.setNoiseLevel(noise);
			  } else src.setNoiseLevel(1);

			  src.setDetectionThreshold(thresholdForFitting);
			  src.setHeader(head);
			  src.defineBox(this->itsCube.pars().section(), this->itsFitParams, this->itsCube.header().getWCS()->spec);

			  if (this->itsFlagDoFit) {

                            if (this->itsFlagFitJustDetection) {
			      // get a list of just the detected voxels, with correct fluxes
			      std::vector<PixelInfo::Voxel> voxlist = src.getPixelSet();
			      std::vector<PixelInfo::Voxel>::iterator vox = voxlist.begin();

			      for (; vox < voxlist.end(); vox++) {
				std::vector<PixelInfo::Voxel>::iterator voxcomp = this->itsVoxelList.begin();

				while (voxcomp < this->itsVoxelList.end() && !vox->match(*voxcomp))
				  voxcomp++;

				if (voxcomp == this->itsVoxelList.end())
				  ASKAPLOG_ERROR_STR(logger, "Voxel lists mismatch: source pixel " << *vox << " does not have a match");
				else vox->setF(voxcomp->getF());
			      }

			      src.fitGaussNew(&voxlist, this->itsFitParams);
                            } else {
			      src.fitGauss(&this->itsVoxelList, this->itsFitParams);
                            }

                            src.findAlpha(this->itsCube.pars().getImageFile(), this->itsFlagFindSpectralIndex);
                            src.findBeta(this->itsCube.pars().getImageFile(), this->itsFlagFindSpectralIndex);
			  }
			}
                        this->itsSourceList.push_back(src);
                    }
                }
		else this->calcObjectParams(); // if no edge sources, call this anyway so the workers know what to do...

		if(this->itsFlagDistribFit) this->fitRemaining();

                ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Finished cleaning up edge sources");

                for (src = goodSources.begin(); src < goodSources.end(); src++) {
                    src->setHeader(head);

                    // Need to check that there are no small sources present that violate the minimum size criteria
                    if ((src->hasEnoughChannels(this->itsCube.pars().getMinChannels()))
                            && (src->getSpatialSize() >= this->itsCube.pars().getMinPix()))
                        this->itsSourceList.push_back(*src);
                }

                ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Now have a total of " << this->itsSourceList.size() << " sources.");

		//                ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Starting sort of source list");
                std::multimap<float, int> detlist;

                for (size_t i = 0; i < this->itsSourceList.size(); i++) {
                    float val = this->is2D() ? this->itsSourceList[i].getXcentre() : this->itsSourceList[i].getVel();
                    detlist.insert(std::pair<float, int>(val, int(i)));
                }

                std::vector<sourcefitting::RadioSource> newlist;
                std::map<float, int>::iterator det;

                for (det = detlist.begin(); det != detlist.end(); det++) newlist.push_back(this->itsSourceList[det->second]);

                this->itsSourceList.clear();
                this->itsSourceList = newlist;
                newlist.clear();
                ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Finished sort of source list");
                this->itsCube.clearDetectionList();

                for (src = this->itsSourceList.begin(); src < this->itsSourceList.end(); src++) {
                    src->setID(src - this->itsSourceList.begin() + 1);
                    src->setAtEdge(this->itsCube, this->itsSubimageDef, this->itsRank - 1);

                    if (src->isAtEdge()) src->addToFlagText("E");
                    else src->addToFlagText("-");

                    this->itsCube.addObject(duchamp::Detection(*src));
                }

                ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Finished adding sources to cube. Now have " << this->itsCube.getNumObj() << " objects.");

            }
	    else if(this->isWorker() && this->itsFlagDistribFit) this->fitRemaining();
        }

        //**************************************************************//

      void DuchampParallel::calcObjectParams()
      {
	if(this->isParallel()){
	  if(this->isMaster()) {
	    int16 rank;
	    LOFAR::BlobString bs;

	    int objsize=0;
	    duchamp::Section fullSec=this->itsCube.pars().section();
	    ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Using subsection for box calcs: " << fullSec.getSection());
	    // now send the individual sources to each worker in turn
	    for(int i=0;i<this->itsCube.getNumObj();i++){
	      rank = i % (this->itsNNode - 1);
	      objsize = this->itsCube.getObject(i).getSize();
	      ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Sending source #"<<i+1<<" of size " << objsize << " to worker "<<rank+1);
	      bs.resize(0);
	      LOFAR::BlobOBufString bob(bs);
	      LOFAR::BlobOStream out(bob);
	      out.putStart("paramsrc", 1);
	      out << objsize;
	      sourcefitting::RadioSource src(this->itsCube.getObject(i));
	      src.defineBox(this->itsCube.pars().section(), this->itsFitParams, this->itsCube.header().getWCS()->spec);
	      out << src;
	      out.putEnd();
	      this->itsConnectionSet->write(rank, bs);
	    }
	    // now send a zero size to tell everyone the list has finished.
	    objsize=0;
	    bs.resize(0);
	    LOFAR::BlobOBufString bob(bs);
	    LOFAR::BlobOStream out(bob);
	    out.putStart("paramsrc", 1);
	    out << objsize;
	    out.putEnd();
	    this->itsConnectionSet->writeAll(bs);

	    // now read back the sources from the workers
	    this->itsSourceList.clear();
	    this->itsCube.clearDetectionList();
	    for (int n=0;n<this->itsNNode-1;n++){
	      int numSrc;
	      ASKAPLOG_INFO_STR(logger, "Master about to read from worker #"<< n+1);
	      this->itsConnectionSet->read(n, bs);
	      LOFAR::BlobIBufString bib(bs);
	      LOFAR::BlobIStream in(bib);
	      int version = in.getStart("final");
	      ASKAPASSERT(version == 1);
	      in >> numSrc;
	      for(int i=0;i<numSrc;i++){
		sourcefitting::RadioSource src;
		in >> src;
		this->itsCube.addObject(src);
	      }
	      in.getEnd();
	    }
	  }
	  else if(this->isWorker()){
	    ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Setting up cube in preparation for object calculation");
	    this->itsCube.pars().setSubsection(this->itsBaseSubsection); // take care of any global offsets due to subsectioning
	    casaImageToMetadata(this->itsCube, this->itsSubimageDef, -1);
	    LOFAR::BlobString bs;

	    // now read individual sources
	    int objsize=1;
	    this->itsCube.clearDetectionList();
	    while(objsize>0) {	    
	      this->itsConnectionSet->read(0, bs);
	      LOFAR::BlobIBufString bib(bs);
	      LOFAR::BlobIStream in(bib);
	      int version = in.getStart("paramsrc");
	      ASKAPASSERT(version == 1);
	      in >> objsize;
	      ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Object calcs: Reading object size=" << objsize << " from Master");
	      if(objsize>0){
		ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Object calcs: Got OK to read object");
		sourcefitting::RadioSource src;
		in >> src;
		ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Object calcs: Read object of size " << src.getSize() << " from Master");
		this->itsCube.addObject(src);
	      }
	      in.getEnd();
	    }

            int numVox = this->itsVoxelList.size();
            int numObj = this->itsCube.getNumObj();

	    ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Read all " << numObj << " objects. Now have to get voxels from list of " << numVox);

	    if (numObj > 0) {
	      std::vector<PixelInfo::Voxel> templist[numObj];
                for (int i = 0; i < numObj; i++) {

		  // for each object, make a vector list of voxels that appear in it.
		  std::vector<PixelInfo::Voxel> objVoxList = this->itsCube.getObject(i).getPixelSet();
		  std::vector<PixelInfo::Voxel>::iterator vox;
		  
		  // get the fluxes of each voxel
		  for (vox = objVoxList.begin(); vox < objVoxList.end(); vox++) {
		    int ct = 0;
		    
		    while (ct < numVox && !vox->match(this->itsVoxelList[ct])) {
		      ct++;
		    }
		    
		    if (numVox != 0 && ct == numVox) { // there has been no match -- problem!
		      ASKAPLOG_ERROR_STR(logger, this->workerPrefix() << "Found a voxel ("<< vox->getX() << "," << vox->getY()<< ") in the object lists that doesn't appear in the base list.");
		    } else vox->setF(this->itsVoxelList[ct].getF());
		  }

		  templist[i] = objVoxList;
                }
		
		ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Allocated fluxes to voxel lists. Now calculating parameters");
		
                std::vector< std::vector<PixelInfo::Voxel> > bigVoxSet(templist, templist + numObj);
                this->itsCube.calcObjectWCSparams(bigVoxSet);
	   }

	    ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Done the WCS parameter calculation. About to send back to master");

	    // send sources back to master
	    bs.resize(0);
	    LOFAR::BlobOBufString bob(bs);
	    LOFAR::BlobOStream out(bob);
	    out.putStart("final", 1);
	    out << int(numObj);
	    for(int i=0;i<numObj;i++){
	      sourcefitting::RadioSource src=this->itsCube.getObject(i);
	      src.defineBox(this->itsCube.pars().section(), this->itsFitParams, this->itsCube.header().getWCS()->spec);
	      out << src;
	    }
	    out.putEnd();
	    this->itsConnectionSet->write(0,bs);

	  }
	}

      }


      void DuchampParallel::calcObjectParamsOLD()
        {
            /// @details A function to calculate object parameters
            /// (including WCS parameters), making use of the this->itsVoxelList
            /// set of voxels. The function finds the voxels that appear in
            /// each object in itsCube, making a vector of vectors of
            /// voxels, then passes this vector to
            /// duchamp::Cube::calcObjectWCSparams().
            int numVox = this->itsVoxelList.size();
            int numObj = this->itsCube.getNumObj();

	    ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "About to calculate parameters for all " << numObj << " objects");
            if (numObj > 0) {
                std::vector<PixelInfo::Voxel> templist[numObj];

                for (int i = 0; i < this->itsCube.getNumObj(); i++) {

// ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Setting voxel list for param calc for object at (ra,dec)=("
// 		     << this->itsCube.getObject(i).getRAs()<<","<<this->itsCube.getObject(i).getDecs()
// 		     << ") or (x,y)=(" 
// 		     <<this->itsCube.getObject(i).getXcentre()+this->itsCube.getObject(i).getXOffset()
// 		     <<","<<this->itsCube.getObject(i).getYcentre()+this->itsCube.getObject(i).getYOffset()
// 		     <<") and size " << this->itsCube.getObject(i).getSize());

                    // for each object, make a vector list of voxels that appear in it.
		    std::vector<PixelInfo::Voxel> objVoxList = this->itsCube.getObject(i).getPixelSet();
		    std::vector<PixelInfo::Voxel>::iterator vox;

                    // get the fluxes of each voxel
                    for (vox = objVoxList.begin(); vox < objVoxList.end(); vox++) {
                        int ct = 0;

                        while (ct < numVox && !vox->match(this->itsVoxelList[ct])) {
                            ct++;
                        }

                        if (numVox != 0 && ct == numVox) { // there has been no match -- problem!
                            ASKAPLOG_ERROR_STR(logger, this->workerPrefix() << "Found a voxel ("
					       << vox->getX() << "," << vox->getY() 
					       << ") in the object lists that doesn't appear in the base list.");
                        } else vox->setF(this->itsVoxelList[ct].getF());
                    }

                    templist[i] = objVoxList;
                }

		ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Allocated fluxes to voxel lists. Now calculating parameters");

                std::vector< std::vector<PixelInfo::Voxel> > bigVoxSet(templist, templist + numObj);
                this->itsCube.calcObjectWCSparams(bigVoxSet);

                for (int i = 0; i < this->itsCube.getNumObj(); i++) {

// ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Finished setting voxel list for param calc for object at (ra,dec)=("
// 		     << this->itsCube.getObject(i).getRAs()<<","<<this->itsCube.getObject(i).getDecs()
// 		     << ") or (x,y)=(" 
// 		     <<this->itsCube.getObject(i).getXcentre()+this->itsCube.getObject(i).getXOffset()
// 		     <<","<<this->itsCube.getObject(i).getYcentre()+this->itsCube.getObject(i).getYOffset()
// 		     <<") and size " << this->itsCube.getObject(i).getSize());
		}

            }
        }

        //**************************************************************//

      void DuchampParallel::distributeVoxelList()
      {
	if(this->isParallel()){
	  if(this->isMaster()) {
	    LOFAR::BlobString bs;

	    // first send the voxel list to all workers
	    /// @todo This could be made more efficient, so that we don't send unnecessary voxels to some workers.
	    bs.resize(0);
	    LOFAR::BlobOBufString bob(bs);
	    LOFAR::BlobOStream out(bob);
	    ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Broadcasting voxel list to all workers");
	    out.putStart("voxels", 1);
	    out << int(this->itsVoxelList.size());
	    for(size_t p=0;p<this->itsVoxelList.size();p++) 
	      out << int32(this->itsVoxelList[p].getX()) << int32(this->itsVoxelList[p].getY()) << int32(this->itsVoxelList[p].getZ()) << this->itsVoxelList[p].getF();
	    out.putEnd();
	    this->itsConnectionSet->writeAll(bs);
	  }
	  else if(this->isWorker()){
	    // first read the voxel list
	    this->itsVoxelList.clear();
	    LOFAR::BlobString bs;
	    this->itsConnectionSet->read(0, bs);
	    LOFAR::BlobIBufString bib(bs);
	    LOFAR::BlobIStream in(bib);
	    int version = in.getStart("voxels");
	    ASKAPASSERT(version == 1);
	    int size;
	    in >> size;
	    int32 x,y,z;
	    float f;
	    ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "About to read a list of " << size << " voxels from the master");
	    for(int p=0;p<size;p++){
	      in >> x >> y >> z >> f;
	      this->itsVoxelList.push_back(PixelInfo::Voxel(x,y,z,f));
	    }
	    in.getEnd();
	  }
	}
      }

	//**************************************************************//

      void DuchampParallel::fitRemaining()
      {
	if(this->isParallel()){
	  if(this->isMaster()) {
	    int16 rank;
	    LOFAR::BlobString bs;
	    
	    // now send the individual sources to each worker in turn
	    for(size_t i=0;i<this->itsSourceList.size();i++){
	      ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Preparing source #"<<i+1);
	      this->prepareSourceForFit(this->itsSourceList[i],false);
	      rank = i % (this->itsNNode - 1);
	      ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Sending source #"<<i+1<<" of size " << this->itsSourceList[i].getSize() << " to worker "<<rank+1);
	      bs.resize(0);
	      LOFAR::BlobOBufString bob(bs);
	      LOFAR::BlobOStream out(bob);
	      out.putStart("fitsrc", 1);
	      out << true << this->itsSourceList[i];
	      out.putEnd();
	      this->itsConnectionSet->write(rank, bs);
	    }

	    // now notify all workers that we're finished.
	    LOFAR::BlobOBufString bob(bs);
	    LOFAR::BlobOStream out(bob);
	    bs.resize(0);
	    bob = LOFAR::BlobOBufString(bs);
	    out = LOFAR::BlobOStream(bob);
	    ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Broadcasting 'finished' signal to all workers");
	    out.putStart("fitsrc", 1);
	    out << false;
	    out.putEnd();
	    this->itsConnectionSet->writeAll(bs);

	    // now read back the sources from the workers
	    this->itsSourceList.clear();
	    for (int n=0;n<this->itsNNode-1;n++){
	      int numSrc;
	      ASKAPLOG_INFO_STR(logger, "Master about to read from worker #"<< n+1);
	      this->itsConnectionSet->read(n, bs);
	      LOFAR::BlobIBufString bib(bs);
	      LOFAR::BlobIStream in(bib);
	      int version = in.getStart("final");
	      ASKAPASSERT(version == 1);
	      in >> numSrc;
	      for(int i=0;i<numSrc;i++){
		sourcefitting::RadioSource src;
		in >> src;
		src.setHeader(this->itsCube.getHead());  // make sure we have the right WCS etc information
		this->itsSourceList.push_back(src);
	      }
	      in.getEnd();
	    }
	  }
	  else if(this->isWorker()){
	    ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Setting up cube in preparation for source fitting");
	    this->itsCube.pars().setSubsection("");
	    casaImageToMetadata(this->itsCube, this->itsSubimageDef, -1);
	    LOFAR::BlobString bs;

	    // now read individual sources
	    bool isOK=true;
	    this->itsSourceList.clear();
	    while(isOK) {	    
	      sourcefitting::RadioSource src;
	      this->itsConnectionSet->read(0, bs);
	      LOFAR::BlobIBufString bib(bs);
	      LOFAR::BlobIStream in(bib);
	      int version = in.getStart("fitsrc");
	      ASKAPASSERT(version == 1);
	      in >> isOK;
	      if(isOK){
		in >> src;
		ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "About to fit src at ra="<<src.getRAs()<<", dec="<<src.getDecs());
		src.setHeader(this->itsCube.getHead());  // this doesn't get copied over the blob, so make sure the beam is correct - that's all we need it for.
		this->fitSource(src,false);
		this->itsSourceList.push_back(src);
	      }
	      in.getEnd();
	    }
	    // send sources back to master
	    bs.resize(0);
	    LOFAR::BlobOBufString bob(bs);
	    LOFAR::BlobOStream out(bob);
	    out.putStart("final", 1);
	    out << int(this->itsSourceList.size());
	    for(size_t i=0;i<this->itsSourceList.size();i++) out << this->itsSourceList[i];
	    out.putEnd();
	    this->itsConnectionSet->write(0,bs);

	  }
	}

      }

        //**************************************************************//

        void DuchampParallel::printResults()
        {
            /// @details The final list of detected objects is written to
            /// the terminal and to the results file in the standard Duchamp
            /// manner.
            if (this->isMaster()) {

                std::vector<std::string> outtypes = this->itsFitParams.fitTypes();
                outtypes.push_back("best");
	      
		if(this->itsCube.pars().getFlagNegative()){
		  this->itsCube.reInvert();

		  std::vector<sourcefitting::RadioSource>::iterator src;
		  for (src = this->itsSourceList.begin(); src < this->itsSourceList.end(); src++) {
		    for (size_t t = 0; t < outtypes.size(); t++) {
		      for(int i=0;i<src->numFits(outtypes[t]);i++){
			Double f = src->fitset(outtypes[t])[i].flux();
			src->fitset(outtypes[t])[i].setFlux(f * -1);
		      }
		    }
		  }

		}
		ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Found " << this->itsCube.getNumObj() << " sources.");

                this->itsCube.prepareOutputFile();
                this->itsCube.outputDetectionList();

                if (this->itsCube.pars().getFlagLog() && (this->itsCube.getNumObj() > 0)) {
                    std::ofstream logfile(this->itsCube.pars().getLogFile().c_str(), std::ios::app);
                    logfile << "=-=-=-=-=-=-=-\nCube summary\n=-=-=-=-=-=-=-\n";
                    logfile << this->itsCube;
                    logfile.close();
                }

                if (this->itsCube.pars().getFlagKarma()) {
                    std::ofstream karmafile(this->itsCube.pars().getKarmaFile().c_str());
                    this->itsCube.outputDetectionsKarma(karmafile);
                    karmafile.close();
                }

		if(this->itsCube.pars().getFlagVOT()){
		  std::ofstream votfile(this->itsCube.pars().getVOTFile().c_str());
		  this->itsCube.outputDetectionsVOTable(votfile);
		  votfile.close();
		}


                std::vector<duchamp::Column::Col> columns = this->itsCube.getFullCols();

                for (size_t t = 0; t < outtypes.size(); t++) {
                    std::ofstream summaryFile(sourcefitting::convertSummaryFile(this->itsFitSummaryFile.c_str(), outtypes[t]).c_str());
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
                std::ofstream outfile2;

                if (this->itsFitAnnotationFile != this->itsFitBoxAnnotationFile) {
                    outfile2.open(this->itsFitBoxAnnotationFile.c_str());
                    outfile << "COLOR BLUE\n";
                    outfile << "COORD W\n";
                    outfile << "FONT lucidasans-12\n";
                }

                std::vector<sourcefitting::RadioSource>::iterator src;

                for (src = this->itsSourceList.begin(); src < this->itsSourceList.end(); src++) {
                    if (this->itsFitAnnotationFile != this->itsFitBoxAnnotationFile) {
                        outfile << "# Source " << int(src - this->itsSourceList.begin()) + 1 << ":\n";
                        src->writeFitToAnnotationFile(outfile, true, false);
                        outfile2 << "# Source " << int(src - this->itsSourceList.begin()) + 1 << ":\n";
                        src->writeFitToAnnotationFile(outfile2, false, true);
                    } else {
                        outfile << "# Source " << int(src - this->itsSourceList.begin()) + 1 << ":\n";
                        src->writeFitToAnnotationFile(outfile, true, true);
                    }
                }

                outfile.close();

                if (this->itsFitAnnotationFile != this->itsFitBoxAnnotationFile) outfile2.close();
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
	      if (this->itsFlagDoMedianSearch){
		if(this->itsCube.pars().getFlagUserThreshold())
		  ASKAPLOG_WARN_STR(logger, "Since median searching has been requested, the threshold given ("<<this->itsCube.stats().getThreshold()<<") is changed to a S/N-based one of "<<this->itsCube.pars().getCut()<<" sigma");
		this->itsCube.stats().setThreshold(this->itsCube.pars().getCut());
	      }
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

		    if (!this->itsCube.pars().getFlagUserThreshold()) {
		      this->itsCube.stats().setThresholdSNR(this->itsCube.pars().getCut());
		      this->itsCube.pars().setFlagUserThreshold(true);
		      this->itsCube.pars().setThreshold(this->itsCube.stats().getThreshold());
		    }
		    //                    this->itsCube.pars().setFlagUserThreshold(true);
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
