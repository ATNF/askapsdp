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
#include <images/Images/FITSImage.h>
#include <images/Images/MIRIADImage.h>
#include <images/Images/SubImage.h>
#include <casa/aipstype.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayPartMath.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Slicer.h>

// boost includes
#include <boost/shared_ptr.hpp>

#include <askap_analysis.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <askapparallel/AskapParallel.h>
#include <imageaccess/CasaImageAccess.h>

#include <parallelanalysis/DuchampParallel.h>
#include <extraction/SourceSpectrumExtractor.h>
#include <extraction/SpectralBoxExtractor.h>
#include <extraction/NoiseSpectrumExtractor.h>
#include <analysisutilities/AnalysisUtilities.h>
#include <analysisutilities/CasaImageUtil.h>
#include <analysisutilities/SubimageDef.h>
#include <sourcefitting/RadioSource.h>
#include <sourcefitting/FittingParameters.h>
#include <analysisutilities/NewArrayMath.h>
#include <analysisutilities/NewArrayPartMath.h>
#include <parametrisation/OptimisedGrower.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <time.h>
#include <vector>
#include <map>

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
using namespace askap::askapparallel;

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
//D1.1.13   long *dim = this->itsCube.getDimArray();
            size_t *dim = this->itsCube.getDimArray();

            for (int i = 0; i < this->itsCube.getNumDim(); i++) if (dim[i] > 1) numDim++;

            return numDim <= 2;
        }

        //**************************************************************//


        DuchampParallel::DuchampParallel(askap::askapparallel::AskapParallel& comms)
                : itsComms(comms)
        {
            this->itsFitParams = sourcefitting::FittingParameters(LOFAR::ParameterSet());
	    this->itsWeighter = new Weighter(itsComms);
        }
        //**************************************************************//

        DuchampParallel::DuchampParallel(askap::askapparallel::AskapParallel& comms,
                                         const LOFAR::ParameterSet& parset)
	  : itsComms(comms), itsParset(parset)
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
	    else ASKAPLOG_DEBUG_STR(logger, "Requested subsection " << this->itsBaseSubsection);
	    if(this->itsBaseSubsection=="") this->itsBaseSubsection = duchamp::nullSection( getCASAdimensions(this->itsCube.pars().getImageFile()).size() );

	    bool flagStatSubsection = parset.getBool("flagStatSec",false);
	    this->itsBaseStatSubsection = parset.getString("statSec","");
	    if(!flagStatSubsection) this->itsBaseStatSubsection = "";
            this->itsWeightImage = parset.getString("weightsimage", "");

	    this->itsFlagThresholdPerWorker = parset.getBool("thresholdPerWorker",false);
	    
            if (this->itsWeightImage != "" ){
	      this->itsWeighter = new Weighter(itsComms);
	      if(!itsComms.isParallel() || itsComms.isMaster()) // only log for the master process.
		ASKAPLOG_INFO_STR(logger, "Using weights image: " << this->itsWeightImage);
	    }

            this->itsFlagDoMedianSearch = parset.getBool("doMedianSearch", false);
            this->itsMedianBoxWidth = parset.getInt16("medianBoxWidth", 50);
	    this->itsFlagWriteSNRimage = parset.getBool("flagWriteSNRimage", false);
	    this->itsSNRimageName = parset.getString("SNRimageName", "");
	    this->itsFlagWriteThresholdImage = parset.getBool("flagWriteThresholdImage",false);
	    this->itsThresholdImageName = parset.getString("ThresholdImageName","");

	    this->itsFlagOptimiseMask = parset.getBool("optimiseMask",false);
	    if(this->itsCube.pars().getFlagGrowth() && this->itsFlagOptimiseMask){
	      ASKAPLOG_WARN_STR(logger, "flagGrowth is set to true, so setting optimiseMask to false");
	      this->itsFlagOptimiseMask = false;
	    }

            this->itsFlagDoFit = parset.getBool("doFit", false);
	    this->itsFlagDistribFit = parset.getBool("distribFit",true);
            this->itsFlagFitJustDetection = parset.getBool("fitJustDetection", false);
            this->itsFlagFindSpectralIndex = parset.getBool("findSpectralIndex", false);

	    this->itsFlagExtractSpectra = parset.getBool("extractSpectra",false);
	    if(this->itsFlagExtractSpectra){
	      ASKAPCHECK(parset.isDefined("spectralCube"), "Source cube not defined for extracting spectra. Please use the \"spectralCube\" parameter.");
	      ASKAPLOG_INFO_STR(logger, "Extracting spectra for detected sources from " << parset.getString("spectralCube",""));
	    }
// 	    if(parset.isDefined("summaryFile")){
// 	      this->itsFitSummaryFile = parset.getString("summaryFile", "duchamp-fitResults.txt");
// 	      ASKAPLOG_WARN_STR(logger, "We've changed the name of the 'summaryFile' parameter to 'fitResultsFile'. Using your parameter " << this->itsFitSummaryFile << " for now, but please change your parset!");
// 	    }
//             this->itsFitSummaryFile = parset.getString("fitResultsFile", "duchamp-fitResults.txt");
//             this->itsFitAnnotationFile = parset.getString("fitAnnotationFile", "duchamp-fitResults.ann");
//             this->itsFitBoxAnnotationFile = parset.getString("fitBoxAnnotationFile", this->itsFitAnnotationFile);
	    if( parset.isDefined("logFile") || 
		((!itsComms.isParallel() || itsComms.isMaster()) && 
		 (parset.isDefined("subimageAnnotationFile") ||
		  parset.isDefined("summaryFile") || 
		  parset.isDefined("fitResultsFile") || 
		  parset.isDefined("fitAnnotationFile") || 
		  parset.isDefined("fitBoxAnnotationFile"))
		 )
		)
	      ASKAPLOG_WARN_STR(logger, "Output filenames have been fixed to certain default values:");

	    this->itsFitSummaryFile = "duchamp-fitResults.txt";
	    this->itsFitAnnotationFile = "duchamp-fitResults.ann";
	    this->itsFitBoxAnnotationFile = "duchamp-fitResults.boxes.ann";

	    if(!itsComms.isParallel() || itsComms.isMaster()){
	      if(parset.isDefined("summaryFile") || parset.isDefined("fitResultsFile") ) ASKAPLOG_WARN_STR(logger, "fitResultsFile = 'duchamp-fitResults.txt'  (= old parameter summaryFile)");
	      if(parset.isDefined("fitAnnotationFile")) ASKAPLOG_WARN_STR(logger, "fitAnnotationFile = 'duchamp-fitResults.ann'");
	      if(parset.isDefined("fitBoxAnnotationFile")) ASKAPLOG_WARN_STR(logger, "fitBoxAnnotationFile = 'duchamp-fitResults.boxes.ann'");
	    }

            LOFAR::ParameterSet fitParset = parset.makeSubset("Fitter.");
            this->itsFitParams = sourcefitting::FittingParameters(fitParset);
	    this->itsFitParams.useBoxFlux(!this->itsFlagFitJustDetection);

            if (this->itsFitParams.numFitTypes() == 0 && this->itsFlagDoFit){
                ASKAPLOG_WARN_STR(logger, "No valid fit types given, so setting doFit flag to false.");
		this->itsFlagDoFit = false;
	    }

//             this->itsSubimageAnnotationFile = parset.getString("subimageAnnotationFile", "");
	    this->itsSubimageAnnotationFile = "duchamp-SubimageLocations.ann";	       
	    if((!itsComms.isParallel() || itsComms.isMaster()) && parset.isDefined("subimageAnnotationFile")) 
	      ASKAPLOG_WARN_STR(logger, "subimageAnnotationFile = 'duchamp-SubimageLocations.ann'");


	    if(parset.isDefined("logFile")){
	      if(itsComms.isParallel() && itsComms.isMaster())
		ASKAPLOG_WARN_STR(logger, "logFile = 'duchamp-Logfile-Master.txt'");
	      if(itsComms.isWorker())
		ASKAPLOG_WARN_STR(logger, "logFile = '"<<itsComms.substitute("duchamp-Logfile-%w.txt")<<"'");
	    }

            if (itsComms.isParallel()) {
	      this->itsSubimageDef = SubimageDef(parset);
	      bool overlapsChanged=false;
	      // Need the overlap to be at least the boxPadSize used by the Fitting
	      if(this->itsFlagDoFit){
		overlapsChanged=true;
		this->itsSubimageDef.setOverlapX(std::max(this->itsSubimageDef.overlapx(), this->itsFitParams.boxPadSize()));
		this->itsSubimageDef.setOverlapY(std::max(this->itsSubimageDef.overlapy(), this->itsFitParams.boxPadSize()));
		this->itsSubimageDef.setOverlapZ(std::max(this->itsSubimageDef.overlapz(), this->itsFitParams.boxPadSize()));
	      }
	      
	      if(this->itsFlagDoMedianSearch){
		overlapsChanged=true;
		this->itsSubimageDef.setOverlapX(std::max(this->itsSubimageDef.overlapx(), 2*this->itsMedianBoxWidth));
		this->itsSubimageDef.setOverlapY(std::max(this->itsSubimageDef.overlapy(), 2*this->itsMedianBoxWidth));
		this->itsSubimageDef.setOverlapZ(std::max(this->itsSubimageDef.overlapz(), 2*this->itsMedianBoxWidth));
	      }
	      if(overlapsChanged)
		ASKAPLOG_DEBUG_STR(logger, "Changed Subimage overlaps to " << this->itsSubimageDef.overlapx() << ","
				   << this->itsSubimageDef.overlapy() << ","<< this->itsSubimageDef.overlapz());

	      if (itsComms.isMaster()) {
		this->itsCube.pars().setLogFile("duchamp-Logfile-Master.txt");
	      }
            }
	    
            if (itsComms.isWorker())
	      this->itsCube.pars().setLogFile(itsComms.substitute("duchamp-Logfile-%w.txt"));

        }


        //**************************************************************//

        int DuchampParallel::getMetadata()
        {
            /// @details Provides a simple front-end to the correct
            /// metadata-reading function, according to whether the image is
            /// FITS data or a CASA image
            /// @return The return value of the function that was used:
            /// either duchamp::SUCCESS or duchamp::FAILURE
	    int returnCode;
            if (this->itsIsFITSFile){
                   this->itsSubimageDef.defineFITS(this->itsCube.pars().getImageFile());
                    this->itsSubimageDef.setImageDim(getFITSdimensions(this->itsCube.pars().getImageFile()));

                    if (!this->itsCube.pars().getFlagSubsection() || this->itsCube.pars().getSubsection() == "") {
                        this->itsCube.pars().setFlagSubsection(true);
                        this->itsCube.pars().setSubsection(nullSection(this->itsSubimageDef.getImageDim().size()));
                    }

                    if (this->itsCube.pars().verifySubsection() == duchamp::FAILURE)
                        ASKAPTHROW(AskapError, this->workerPrefix() << "Cannot parse the subsection string " << this->itsCube.pars().getSubsection());

                    returnCode = this->itsCube.getMetadata();
		    if(returnCode == duchamp::FAILURE) {
		      ASKAPTHROW(AskapError, this->workerPrefix() << "Something went wrong with itsCube.getMetadata()");
		    }
		    else{

//D1.1.13	      long *dimarr=this->itsCube.getDimArray();
		      size_t *dimarr=this->itsCube.getDimArray();
		      ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Dimensions ("<<this->itsCube.getNumDim()<<" of them) before correction are "
					<< dimarr[0] << "x"<<dimarr[1] << "x" << dimarr[2]);
		      int lng=this->itsCube.header().WCS().lng, lat=this->itsCube.header().WCS().lat, spec=this->itsCube.header().WCS().spec;
		      dimarr[0] = this->itsCube.pars().section().getDim(lng);
		      dimarr[1] = this->itsCube.pars().section().getDim(lat);
		      dimarr[2] = this->itsCube.pars().section().getDim(spec);
		      ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Dimensions after correction are "
					<< dimarr[0] << "x"<<dimarr[1] << "x" << dimarr[2]);

		      this->itsCube.header().WCS().crpix[lng] -= this->itsCube.pars().section().getStart(lng);
		      this->itsCube.header().WCS().crpix[lat] -= this->itsCube.pars().section().getStart(lat);
		      this->itsCube.header().WCS().crpix[spec] -= this->itsCube.pars().section().getStart(spec);

		      ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Dimensions are "
					<< this->itsCube.getDimX() << " " << this->itsCube.getDimY() << " " << this->itsCube.getDimZ());
		    }
		    
                    // check the true dimensionality and set the 2D flag in the cube header.
                    int numDim = 0;
//D1.1.13           long *dim = this->itsCube.getDimArray();
                    size_t *dim = this->itsCube.getDimArray();

                    for (int i = 0; i < this->itsCube.getNumDim(); i++) if (dim[i] > 1) numDim++;

                    this->itsCube.header().set2D(numDim <= 2);

                    // set up the various flux units
//D1.1.13           if (this->itsCube.header().getWCS()->spec >= 0) this->itsCube.header().fixUnits(this->itsCube.pars());
                    if (this->itsCube.header().getWCS()->spec >= 0) this->itsCube.header().fixSpectralUnits(this->itsCube.pars().getSpectralUnits());

	    }
            // else return casaImageToMetadata(this->itsCube, this->itsSubimageDef, itsComms);
            else returnCode = this->getCASA(METADATA,false);

	    return returnCode;
        }

        //**************************************************************//

        std::vector<float> DuchampParallel::getBeamInfo()
        {
            /// @details Returns a vector containing the beam parameters:
            /// major axis [deg], minor axis [deg], position angle [deg].
            std::vector<float> beam(3);
	    beam[0] = this->itsCube.header().beam().maj();
	    beam[1] = this->itsCube.header().beam().min();
	    beam[2] = this->itsCube.header().beam().pa();
// 	      beam[0] = this->itsCube.header().getBmajKeyword();
// 	      beam[1] = this->itsCube.header().getBminKeyword();
// 	      beam[2] = this->itsCube.header().getBpaKeyword();
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
            if (itsComms.isParallel() && itsComms.isMaster()) {
                
                ASKAPLOG_INFO_STR(logger,  this->workerPrefix() << "About to read metadata from image " << this->itsCube.pars().getImageFile());

		int result=this->getMetadata();

                ASKAPLOG_INFO_STR(logger, "Annotation file for subimages is \"" << this->itsSubimageAnnotationFile << "\".");

                if (this->itsSubimageAnnotationFile != "") {
                    ASKAPLOG_INFO_STR(logger, "Writing annotation file showing subimages to " << this->itsSubimageAnnotationFile);
		    //                    this->itsSubimageDef.writeAnnotationFile(this->itsSubimageAnnotationFile, this->itsCube.pars().section(),  this->itsCube.header(), this->itsCube.pars().getImageFile(), itsComms);
                    this->itsSubimageDef.writeAnnotationFile(this->itsSubimageAnnotationFile, this->itsCube.header(), this->itsCube.pars().getImageFile(), itsComms);
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

            } else if (itsComms.isWorker()) {

//                 ASKAPLOG_INFO_STR(logger,  this->workerPrefix() << "About to read data from image " << this->itsCube.pars().getFullImageFile());

//                 if (this->itsIsFITSFile) ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Reading with FITS code");
//                 else                     ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Reading with CASA code");

                int result;

                if (this->itsIsFITSFile) {
                    this->itsSubimageDef.defineFITS(this->itsCube.pars().getImageFile());
                    this->itsSubimageDef.setImageDim(getFITSdimensions(this->itsCube.pars().getImageFile()));

                    if (!this->itsCube.pars().getFlagSubsection() || this->itsCube.pars().getSubsection() == "") {
                        this->itsCube.pars().setFlagSubsection(true);
                        this->itsCube.pars().setSubsection(nullSection(this->itsSubimageDef.getImageDim().size()));
                    }

                    if (itsComms.isParallel()) {
		      //		      duchamp::Section subsection = this->itsSubimageDef.section(itsComms.rank()-1, this->itsCube.pars().getSubsection());
		      //		      this->itsSubimageDef.setInputSubsection(this->itsCube.pars().getSubsection());
		      this->itsSubimageDef.setInputSubsection(this->itsBaseSubsection);
		      duchamp::Section subsection = this->itsSubimageDef.section(itsComms.rank()-1);
		      ASKAPLOG_DEBUG_STR(logger, this->workerPrefix()<<"Starting with base section = |"<<this->itsBaseSubsection<<"| and node #"<<itsComms.rank()-1<<" we get section "<<subsection.getSection());
		      //		      this->itsCube.pars().setSubsection(subsection.getSection());
		      //		      this->itsCube.pars().parseSubsections(this->itsCube.getDimArray(), this->itsCube.getNumDim());
		      this->itsCube.pars().setFlagSubsection(true);
		      this->itsCube.pars().section()=subsection;
		      ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Subsection = " << this->itsCube.pars().section().getSection());
		      if(this->itsCube.pars().getFlagStatSec()){
			if(this->itsCube.pars().statsec().isValid())
			  ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Statistics section = " << this->itsCube.pars().statsec().getSection());
			else
			  ASKAPLOG_INFO_STR(logger, this->workerPrefix() << " does not contribute to the statistics section");
		      }
                    }
		    else{
		      this->itsCube.pars().setSubsection(this->itsBaseSubsection);
		      ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Subsection = " << this->itsCube.pars().section().getSection());
		    }

                    if (this->itsCube.pars().verifySubsection() == duchamp::FAILURE)
                        ASKAPTHROW(AskapError, this->workerPrefix() << "Cannot parse the subsection string " << this->itsCube.pars().getSubsection());

                    ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Using subsection " << this->itsCube.pars().getSubsection());
                    ASKAPLOG_INFO_STR(logger,  this->workerPrefix()
                                          << "About to read data from image " << this->itsCube.pars().getFullImageFile());
                    result = this->itsCube.getCube();
                } else { // if it's a CASA image
		  //                    result = casaImageToCube(this->itsCube, this->itsSubimageDef, itsComms);
		  result = getCASA(IMAGE);
                }

                if (result == duchamp::FAILURE) {
                    ASKAPLOG_ERROR_STR(logger, this->workerPrefix() << "Could not read in data from image " << this->itsCube.pars().getImageFile());
                    ASKAPTHROW(AskapError, this->workerPrefix() << "Unable to read image " << this->itsCube.pars().getImageFile());
                } else {
//                     ASKAPLOG_INFO_STR(logger,  this->workerPrefix() << "Read data from image " << this->itsCube.pars().getImageFile());
                    ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Dimensions are "
                                          << this->itsCube.getDimX() << " " << this->itsCube.getDimY() << " " << this->itsCube.getDimZ());

                    if (this->itsCube.getDimZ() == 1) this->itsCube.pars().setMinChannels(0);
                }

		if(this->itsCube.pars().getFlagNegative()){
		  ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Inverting cube");
		  this->itsCube.invert();
		}

                if (this->itsCube.pars().getFlagATrous()) {
		  ASKAPLOG_INFO_STR(logger,  this->workerPrefix() << "Reconstructing with dimension " << this->itsCube.pars().getReconDim());
		    
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
	  if(itsComms.isParallel() && itsComms.isMaster()){
	    if(this->itsFlagDoMedianSearch){
	      this->medianSearch(); // doesn't search, just creates image if need be
	    }
	    else if(this->itsWeightImage != ""){
	      this->itsWeighter->initialise(this->itsWeightImage, this->itsCube.pars().section(), !(itsComms.isParallel()&&itsComms.isMaster()));
	    }
	  }	      
            if (itsComms.isWorker()) {
                // remove mininum size criteria, so we don't miss anything on the borders.
                int minpix = this->itsCube.pars().getMinPix();
                int minchan = this->itsCube.pars().getMinChannels();
		int minvox = this->itsCube.pars().getMinVoxels();

                if (itsComms.isParallel()) {
                    this->itsCube.pars().setMinPix(1);
                    this->itsCube.pars().setMinChannels(1);
		    this->itsCube.pars().setMinVoxels(1);
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

                ASKAPLOG_INFO_STR(logger,  this->workerPrefix() << "Intermediate list has " << this->itsCube.getNumObj() << " objects.");
                // merge the objects, and grow them if necessary.
                this->itsCube.ObjectMerger();

		if(this->itsFlagOptimiseMask){
		  // Use the mask optimisation routine provided by WALLABY
		  OptimisedGrower grower(this->itsParset.makeSubset("optimiseMask"));
		  grower.define(&this->itsCube);
		  double pix[3],wld[3];
		  for(size_t o=0;o<this->itsCube.getNumObj();o++){
		    Detection *det=this->itsCube.pObject(o);
		    wld[0]=det->getRA(); wld[1]=det->getDec(); wld[2]=this->itsCube.header().velToSpec(det->getV50Max()); 
		    this->itsCube.header().wcsToPix(wld,pix);
		    int zmax=int(pix[2]);
		    wld[0]=det->getRA(); wld[1]=det->getDec(); wld[2]=this->itsCube.header().velToSpec(det->getV50Min()); 
		    this->itsCube.header().wcsToPix(wld,pix);
		    int zmin=int(pix[2]);
		    grower.setMaxMinZ(zmax,zmin);
		    grower.grow(det);
		  }
		  grower.updateDetectMap(this->itsCube.getDetectMap());
		  this->itsCube.ObjectMerger(); // do a second merging to clean up any objects that have joined together.
		}

                if (itsComms.isParallel()) {
                    this->itsCube.pars().setMinPix(minpix);
                    this->itsCube.pars().setMinChannels(minchan);
		    this->itsCube.pars().setMinVoxels(minvox);
                }

		// Remove non-edge sources that are smaller than originally requested, as these won't be grown any further.
		std::vector<duchamp::Detection> edgelist,goodlist;
//D1.1.13 	for(int i=0; i<this->itsCube.getNumObj();i++){
		for(size_t i=0; i<this->itsCube.getNumObj();i++){
		  sourcefitting::RadioSource src(this->itsCube.getObject(i));
		  src.setAtEdge(this->itsCube, this->itsSubimageDef, itsComms.rank() - 1);
		  if(src.isAtEdge()) edgelist.push_back(this->itsCube.getObject(i));
		  else goodlist.push_back(this->itsCube.getObject(i));
		}
		duchamp::finaliseList(goodlist,this->itsCube.pars());
		size_t ngood=goodlist.size(),nedge=edgelist.size();
		this->itsCube.clearDetectionList();
		for(size_t i=0;i<edgelist.size();i++) goodlist.push_back(edgelist[i]);
		this->itsCube.ObjectList() = goodlist;
		//-------

		ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Calculating WCS params");
                this->itsCube.calcObjectWCSparams();
		if(this->itsFlagDoMedianSearch){
//D1.1.13 	  for(int i=0; i<this->itsCube.getNumObj();i++){
		  for(size_t i=0; i<this->itsCube.getNumObj();i++){
		    std::vector<Voxel> voxlist = this->itsCube.getObject(i).getPixelSet();
		    for(size_t v=0;v<voxlist.size();v++){
		      float snr=this->itsCube.getReconValue(voxlist[v].getX(),voxlist[v].getY(),voxlist[v].getZ());
		      if(v==0 || snr>this->itsCube.getObject(i).getPeakSNR()) this->itsCube.pObject(i)->setPeakSNR(snr);
		    }
		  }
		}
                ASKAPLOG_INFO_STR(logger,  this->workerPrefix() << "Found " << this->itsCube.getNumObj() << " objects, of which "
				  << nedge << " are on the boundary and " << ngood << " are good.");

            }
	}

        //**************************************************************//
       void DuchampParallel::weightSearch()
      {
	
	ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Allocating weighted image");
	float *snrAll = new float[this->itsCube.getSize()];
	ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Defining weighted image");
	for(size_t i=0; i<size_t(this->itsCube.getSize());i++){
	  snrAll[i] = this->itsCube.getPixValue(i)*this->itsWeighter->weight(i);
	}
	ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Saving weighted image");
	this->itsCube.saveRecon(snrAll, this->itsCube.getSize());
	this->itsCube.setReconFlag(true);
		
	ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Searching weighted image to threshold " << this->itsCube.stats().getThreshold());
	this->itsCube.ObjectList() = searchReconArray(this->itsCube.getDimArray(),this->itsCube.getArray(),this->itsCube.getRecon(),this->itsCube.pars(),this->itsCube.stats());
	this->itsCube.updateDetectMap();
	if(this->itsCube.pars().getFlagLog())
	  this->itsCube.logDetectionList();
	
	delete [] snrAll;
	delete this->itsWeighter; // don't need it anymore.
      }

      
      void findVariableThreshold(float *input, float *output, float SNRthreshold, casa::IPosition shape, casa::IPosition box, size_t loc, size_t spatSize, size_t specSize, bool isSpatial, bool useRobust)
      {
	casa::Array<Float> base(shape, input, casa::COPY);
	casa::Array<Float> middle, spread;
	if(useRobust){
	  ASKAPLOG_DEBUG_STR(logger, "findVariableThreshold: Finding 'middle' array'");
	  middle = slidingArrayMath(base, box, MedianFunc<Float>());
	  ASKAPLOG_DEBUG_STR(logger, "findVariableThreshold: Finding 'spread' array'");
	  spread = slidingArrayMath(base, box, MadfmFunc<Float>()) / Statistics::correctionFactor;
	}
	else{
	  middle = slidingArrayMath(base, box, MeanFunc<Float>());
	  spread = slidingArrayMath(base, box, StddevFunc<Float>());
	}
	ASKAPLOG_DEBUG_STR(logger, "findVariableThreshold: Calculating threshold array using SNRthreshold="<<SNRthreshold);
	casa::Array<Float> threshold = middle + SNRthreshold * spread;
	casa::Array<Float>::iterator threshEnd(threshold.end()),iterThresh(threshold.begin());
	int pos=0,i=0;
	ASKAPLOG_DEBUG_STR(logger, "findVariableThreshold: Preparing to return");
	ASKAPLOG_DEBUG_STR(logger, "spatSize="<<spatSize <<", loc="<<loc);
	while (iterThresh != threshEnd){
	  pos = isSpatial ?  i+loc*spatSize :  loc+i*spatSize;
	  output[pos] = *iterThresh;
	  if(i%100==0) ASKAPLOG_DEBUG_STR(logger, i<< " " << pos << " " << output[pos]);
	  i++;
	  iterThresh++;
	}
	ASKAPLOG_DEBUG_STR(logger, "findVariableThreshold: All done");
	
	
      }

      void findSNR(float *input, float *output, casa::IPosition shape, casa::IPosition box, size_t loc, size_t spatSize, size_t specSize, bool isSpatial, bool useRobust)
      {
	casa::Array<Float> base(shape, input, casa::COPY);
	casa::Array<Float> middle, spread;
	if(useRobust){
	  middle = slidingArrayMath(base, box, MedianFunc<Float>());
	  spread = slidingArrayMath(base, box, MadfmFunc<Float>()) / Statistics::correctionFactor;
	}
	else{
	  middle = slidingArrayMath(base, box, MeanFunc<Float>());
	  spread = slidingArrayMath(base, box, StddevFunc<Float>());
	}
	casa::Array<Float> snr = (base - middle);
	
	// Make sure we don't divide by the zeros around the edge of madfm. Need to set those values to S/N=0.
	Array<Float>::iterator baseEnd(base.end());
	Array<Float>::iterator iterBase(base.begin()),iterMiddle(middle.begin()),iterSpread(spread.begin()),iterSnr(snr.begin());
	int i=0,pos=0;
	while(iterBase != baseEnd){
	  pos = isSpatial ?  i+loc*spatSize :  loc+i*spatSize;
	  output[pos] = (*iterSpread > 0) ? (*iterSnr)/(*iterSpread) : 0.;
	  i++;
	  iterBase++;
	  iterMiddle++;
	  iterSpread++;
	  iterSnr++;
	}
	
	
      }


      void DuchampParallel::writeImage(std::string imageName, Float* data)
      {
	const LatticeBase* lattPtr = ImageOpener::openImage(this->itsCube.pars().getImageFile());
	if (lattPtr == 0)
	  ASKAPTHROW(AskapError, "Requested image \"" << this->itsCube.pars().getImageFile() << "\" does not exist or could not be opened.");
	const ImageInterface<Float>* imagePtr = dynamic_cast<const ImageInterface<Float>*>(lattPtr);
	Slicer slice = subsectionToSlicer(this->itsCube.pars().section());
	fixSlicer(slice, this->itsCube.header().getWCS());
	ASKAPLOG_DEBUG_STR(logger, "Writing into subimage defined by slicer " << slice << " which comes from "<< this->itsCube.pars().section().getSection());
	const SubImage<Float> *sub = new SubImage<Float>(*imagePtr, slice);
	accessors::CasaImageAccess ia;
	
	ASKAPLOG_DEBUG_STR(logger, "Creating image " << imageName);
	ia.create(imageName, sub->shape(), sub->coordinates());
	ASKAPLOG_DEBUG_STR(logger, "Writing data to image");
	ia.write(imageName, casa::Array<float>(sub->shape(), data) );

	delete imagePtr;
	
      }

      void DuchampParallel::writeImage(std::string imageName, casa::Array<Float> data)
      {
	const LatticeBase* lattPtr = ImageOpener::openImage(this->itsCube.pars().getImageFile());
	if (lattPtr == 0)
	  ASKAPTHROW(AskapError, "Requested image \"" << this->itsCube.pars().getImageFile() << "\" does not exist or could not be opened.");
	const ImageInterface<Float>* imagePtr = dynamic_cast<const ImageInterface<Float>*>(lattPtr);
	Slicer slice = subsectionToSlicer(this->itsCube.pars().section());
	fixSlicer(slice, this->itsCube.header().getWCS());
	ASKAPLOG_DEBUG_STR(logger, "Writing into subimage defined by slicer " << slice << " which comes from "<< this->itsCube.pars().section().getSection());
	const SubImage<Float> *sub = new SubImage<Float>(*imagePtr, slice);
	accessors::CasaImageAccess ia;
	
	ia.create(imageName, sub->shape(), sub->coordinates());
	ia.write(imageName, data );
	
	delete imagePtr;
	
      }


      void DuchampParallel::medianSearch()
      {
	if(this->itsFlagWriteSNRimage || this->itsFlagWriteThresholdImage){
	  ImageOpener::registerOpenImageFunction(ImageOpener::FITS, FITSImage::openFITSImage);
	  ImageOpener::registerOpenImageFunction(ImageOpener::MIRIAD, MIRIADImage::openMIRIADImage);
	  if(this->itsFlagWriteSNRimage && this->itsSNRimageName==""){
	    // if it has not been specified, construct name from input image
	    this->itsSNRimageName=this->itsCube.pars().getImageFile();
	    // trim .fits off name if present
	    if(this->itsSNRimageName.substr(this->itsSNRimageName.size()-5,5)==".fits") 
	      this->itsSNRimageName=this->itsSNRimageName.substr(0,this->itsSNRimageName.size()-5);
	    this->itsSNRimageName += "-SNR";
	    ASKAPLOG_DEBUG_STR(logger, "Actually saving SNR map to image \"" << this->itsSNRimageName <<"\"");
	  }
	  if(this->itsFlagWriteThresholdImage && this->itsThresholdImageName==""){
	    // if it has not been specified, construct name from input image
	    this->itsThresholdImageName=this->itsCube.pars().getImageFile();
	    // trim .fits off name if present
	    if(this->itsThresholdImageName.substr(this->itsThresholdImageName.size()-5,5)==".fits") 
	      this->itsThresholdImageName=this->itsThresholdImageName.substr(0,this->itsThresholdImageName.size()-5);
	    this->itsThresholdImageName += "-Threshold";
	    ASKAPLOG_DEBUG_STR(logger, "Actually saving Threshold map to image \"" << this->itsThresholdImageName <<"\"");
	  }
	}

	if(itsComms.isWorker()){

	  ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "About to find median & MADFM arrays, and use these to search");
	  size_t spatSize = this->itsCube.getDimX() * this->itsCube.getDimY();
	  size_t specSize = this->itsCube.getDimZ();
 	  float *snrAll = new float[this->itsCube.getSize()];
//D1.1.13 long *imdim = new long[2];
	  size_t *imdim = new size_t[2];

	  if(this->itsCube.pars().getSearchType()=="spatial"){
	    casa::IPosition box(2, this->itsMedianBoxWidth, this->itsMedianBoxWidth);
	    casa::IPosition shape(2, this->itsCube.getDimX(), this->itsCube.getDimY());
	    imdim[0] = this->itsCube.getDimX(); imdim[1] = this->itsCube.getDimY();
	    duchamp::Image *chanIm = new duchamp::Image(imdim);
	    for (size_t z = 0; z < specSize; z++) {
	      chanIm->extractImage(this->itsCube, z);
	      findSNR(chanIm->getArray(),snrAll,shape,box,z,spatSize,specSize,true,this->itsCube.pars().getFlagRobustStats());
	    }
	    delete chanIm;
	  }
	  else if(this->itsCube.pars().getSearchType()=="spectral"){
	    casa::IPosition box(1,this->itsMedianBoxWidth);
	    casa::IPosition shape(1,this->itsCube.getDimZ());
	    imdim[0] = this->itsCube.getDimZ(); imdim[1] = 1;
	    duchamp::Image *chanIm = new duchamp::Image(imdim);
	    ASKAPLOG_DEBUG_STR(logger, "Finding SNR map in spectral mode, with shape="<<shape<<" and box="<<box);
	    for (size_t i = 0; i < spatSize; i++) {
	      chanIm->extractSpectrum(this->itsCube, i);
 	      findSNR(chanIm->getArray(),snrAll,shape,box,i,spatSize,specSize,false,this->itsCube.pars().getFlagRobustStats());
	    }
	    delete chanIm;
	  }
	  ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Saving SNR map");
	  this->itsCube.saveRecon(snrAll, this->itsCube.getSize());
	  this->itsCube.setReconFlag(true);
	
	  if (!this->itsCube.pars().getFlagUserThreshold()) {
	    ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Setting user threshold to " << this->itsCube.pars().getCut());
	    this->itsCube.pars().setThreshold(this->itsCube.pars().getCut());
	    this->itsCube.pars().setFlagUserThreshold(true);
	    if(this->itsCube.pars().getFlagGrowth()){
	      ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Setting user growth threshold to " << this->itsCube.pars().getGrowthCut());
	      this->itsCube.pars().setGrowthThreshold(this->itsCube.pars().getGrowthCut());
	      this->itsCube.pars().setFlagUserGrowthThreshold(true);
	    }
	  }
	
	  ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Searching SNR map");
	  this->itsCube.ObjectList() = searchReconArray(this->itsCube.getDimArray(),this->itsCube.getArray(),this->itsCube.getRecon(),this->itsCube.pars(),this->itsCube.stats());
	  ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Number of sources found = " << this->itsCube.getNumObj());
	  this->itsCube.updateDetectMap();
	  if(this->itsCube.pars().getFlagLog())
	    this->itsCube.logDetectionList();

	  if(this->itsFlagWriteSNRimage){

	    ASKAPLOG_DEBUG_STR(logger, "Saving SNR map to image \"" << this->itsSNRimageName <<"\"");
	    std::stringstream addition;
	    addition << "-" << itsComms.rank();
	    this->itsSNRimageName += addition.str();
	    ASKAPLOG_DEBUG_STR(logger, "SNR image name is now " << this->itsSNRimageName);
	    
	    this->writeImage(this->itsSNRimageName,this->itsCube.getRecon());
	  }
	
	  if(this->itsFlagWriteThresholdImage){

	    ASKAPLOG_DEBUG_STR(logger, "Saving Threshold map to image \"" << this->itsThresholdImageName <<"\"");
	    std::stringstream addition;
	    addition << "-" << itsComms.rank();
	    this->itsSNRimageName += addition.str();
	    ASKAPLOG_DEBUG_STR(logger, "Threshold image name is now " << this->itsSNRimageName);
	    
	    // Find the flux threshold map, purely for the purposes of writing it out to an image.
	    // Re-use the temporary storage we used for the SNR map prior to saving to the Cube's recon array
	    if(this->itsCube.pars().getSearchType()=="spatial"){
	      casa::IPosition box(2, this->itsMedianBoxWidth, this->itsMedianBoxWidth);
	      casa::IPosition shape(2, this->itsCube.getDimX(), this->itsCube.getDimY());
	      imdim[0] = this->itsCube.getDimX(); imdim[1] = this->itsCube.getDimY();
	      duchamp::Image *chanIm = new duchamp::Image(imdim);
	      for (size_t z = 0; z < specSize; z++) {
		chanIm->extractImage(this->itsCube, z);
		ASKAPLOG_DEBUG_STR(logger, "Finding threshold map for z="<<z);
		findVariableThreshold(chanIm->getArray(),snrAll,this->itsCube.stats().getThreshold(),shape,box,z,spatSize,specSize,true,this->itsCube.pars().getFlagRobustStats());
	      }
	      delete chanIm;
	    }
	    else if(this->itsCube.pars().getSearchType()=="spectral"){
	      casa::IPosition box(1,this->itsMedianBoxWidth);
	      casa::IPosition shape(1,this->itsCube.getDimZ());
	      imdim[0] = this->itsCube.getDimZ(); imdim[1] = 1;
	      duchamp::Image *chanIm = new duchamp::Image(imdim);
	      ASKAPLOG_DEBUG_STR(logger, "Finding SNR map in spectral mode, with shape="<<shape<<" and box="<<box);
	      for (size_t i = 0; i < spatSize; i++) {
		chanIm->extractSpectrum(this->itsCube, i);
		findVariableThreshold(chanIm->getArray(),snrAll,this->itsCube.stats().getThreshold(),shape,box,i,spatSize,specSize,false,this->itsCube.pars().getFlagRobustStats());
	      }
	      delete chanIm;
	    }
	    ASKAPLOG_DEBUG_STR(logger, "About to write the threshold map to image " << this->itsThresholdImageName);
	    this->writeImage(this->itsThresholdImageName,snrAll);
	    ASKAPLOG_DEBUG_STR(logger, "Done");
	  }
	
	  delete [] snrAll;
	  delete [] imdim;
	}
      }

      


        void DuchampParallel::medianSearch2D()
        {

            ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "About to find median & MADFM arrays, and use these to search");
            casa::IPosition box(2, this->itsMedianBoxWidth, this->itsMedianBoxWidth);
            casa::IPosition shape(2, this->itsCube.getDimX(), this->itsCube.getDimY());

            int spatSize = this->itsCube.getDimX() * this->itsCube.getDimY();
            float *snrAll = new float[this->itsCube.getSize()];
//D1.1.13   long *imdim = new long[2];
            size_t *imdim = new size_t[2];
            imdim[0] = this->itsCube.getDimX(); imdim[1] = this->itsCube.getDimY();
            duchamp::Image *chanIm = new duchamp::Image(imdim);

//D1.1.13   for (int z = 0; z < this->itsCube.getDimZ(); z++) {
            for (size_t z = 0; z < this->itsCube.getDimZ(); z++) {

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
            if (itsComms.isWorker()) {
                // don't do fit if we have a spectral axis.
                bool flagIs2D = !this->itsCube.header().canUseThirdAxis() || this->is2D();
                this->itsFlagDoFit = this->itsFlagDoFit && flagIs2D;

                if (this->itsFlagDoFit)
                    ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Fitting source profiles.");

 //D1.1.13      for (int i = 0; i < this->itsCube.getNumObj(); i++) {
                for (size_t i = 0; i < this->itsCube.getNumObj(); i++) {
                    if (this->itsFlagDoFit)
                        ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Setting up source #" << i + 1 << " / " << this->itsCube.getNumObj() 
					  << ", size " << this->itsCube.getObject(i).getSize() 
					  << ", peaking at (x,y)=("<<this->itsCube.getObject(i).getXPeak()+this->itsCube.getObject(i).getXOffset()
					  << "," << this->itsCube.getObject(i).getYPeak()+this->itsCube.getObject(i).getYOffset() << ")");

                    sourcefitting::RadioSource src(this->itsCube.getObject(i));

		    this->prepareSourceForFit(src,true);
		    // Only do fit if object is not next to boundary
		    src.setAtEdge(this->itsCube, this->itsSubimageDef, itsComms.rank() - 1);
		    
		    if (itsComms.nProcs() == 1) src.setAtEdge(false);

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

	if(useArray) src.setDetectionThreshold(this->itsCube, this->itsFlagDoMedianSearch);
	else src.setDetectionThreshold(this->itsVoxelList, this->itsSNRVoxelList, this->itsFlagDoMedianSearch);

	src.setHeader(this->itsCube.getHead());
	src.setOffsets(this->itsCube.pars());
	if(!this->itsFlagDoFit) this->itsFitParams.setBoxPadSize(1);
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

	for(int t=1;t<=2;t++)
	  src.findSpectralTerm(this->itsCube.pars().getImageFile(), t, this->itsFlagFindSpectralIndex);

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
            if (itsComms.isWorker()) {
                int32 num = this->itsCube.getNumObj();
                int16 rank = itsComms.rank();

                if (itsComms.isParallel()) {
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
//D1.1.13                   xmax = std::min(this->itsCube.getDimX() - 1, src->boxXmax());
                            xmax = std::min(int(this->itsCube.getDimX()) - 1, int(src->boxXmax()));
                            ymin = std::max(0 , int(src->boxYmin()));
//D1.1.13                   ymax = std::min(this->itsCube.getDimY() - 1, src->boxYmax());
                            ymax = std::min(int(this->itsCube.getDimY()) - 1, int(src->boxYmax()));

                            if (this->is2D()) {
                                zmin = zmax = 0;
                            } else {
                                zmin = std::max(0 , int(src->boxZmin()));
//D1.1.13                       zmax = std::min(this->itsCube.getDimZ() - 1, src->boxZmax());
                                zmax = std::min(int(this->itsCube.getDimZ()) - 1, int(src->boxZmax()));
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
                    itsComms.sendBlob(bs, 0);
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
	if (!itsComms.isParallel() || itsComms.isMaster()) {
	  ASKAPLOG_INFO_STR(logger,  this->workerPrefix() << "Retrieving lists from workers");

	  if (itsComms.isParallel()) {
	    LOFAR::BlobString bs;
	    int16 rank;
	    int32 numObj;

	    // don't do fit if we have a spectral axis.
	    bool flagIs2D = !this->itsCube.header().canUseThirdAxis() || this->is2D();
	    this->itsFlagDoFit = this->itsFlagDoFit && flagIs2D;

	    // list of fit types, for use in correcting positions of fitted components
	    std::vector<std::string>::iterator fittype;
	    std::vector<std::string> fittypelist = sourcefitting::availableFitTypes;
	    fittypelist.push_back("best");
	    fittypelist.push_back("guess");

	    for (int i = 1; i < itsComms.nProcs(); i++) {
	      ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "In loop #"<<i<<" of reading from workers");
	      itsComms.receiveBlob(bs, i);
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

		// Correct the offsets for the fitted components
		for (fittype = fittypelist.begin(); fittype < fittypelist.end(); fittype++) {
		  if(this->itsFitParams.hasType(*fittype) || *fittype=="best" || *fittype=="guess"){
		    for (unsigned int f = 0; f < src.fitset(*fittype).size(); f++) {
		      src.fitset(*fittype)[f].setXcenter(src.fitset(*fittype)[f].xCenter() + src.getXOffset());
		      src.fitset(*fittype)[f].setYcenter(src.fitset(*fittype)[f].yCenter() + src.getYOffset());
		    }
		  }
		}

		// And now set offsets to those of the full image as we are in the master cube
		src.setOffsets(this->itsCube.pars());
		src.defineBox(this->itsCube.pars().section(), this->itsFitParams, this->itsCube.header().getWCS()->spec);
		src.fitparams() = this->itsFitParams;
		if(src.isAtEdge()) this->itsEdgeSourceList.push_back(src);
		else this->itsSourceList.push_back(src);

		if (src.isAtEdge()) {
		  int numVox;
		  bool haveSNRvalues;
		  in >> numVox >> haveSNRvalues;
		  //		  ASKAPLOG_DEBUG_STR(logger,this->workerPrefix() << "Edge source from worker #"<<i<<": (x,y,z) = ("<<src.getXcentre()<<","<<src.getYcentre()<<","<<src.getZcentre()<<") in ["<<src.getXmin()<<":"<<src.getXmax()<<","<<src.getYmin()<<":"<<src.getYmax()<<","<<src.getZmin()<<":"<<src.getZmax()<<"], numVox="<<numVox);

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

	      if(this->itsFlagDoMedianSearch)
		ASKAPASSERT(this->itsVoxelList.size()==this->itsSNRVoxelList.size());

	      ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Received list of size "
				<< numObj << " from worker #" << rank);
	      ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Now have "
				<< this->itsSourceList.size() << " good objects and " << this->itsEdgeSourceList.size() << " edge objects");
	      in.getEnd();
	    }
	  }
	}

	if(itsComms.isParallel() && itsComms.isMaster()) 
	  ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Distributing full voxel list, of size " << this->itsVoxelList.size() << " to the workers.");
	else if(itsComms.isParallel() && itsComms.isWorker())
	  ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "About to receive the voxel list from the Master");
	this->distributeVoxelList();
	if(itsComms.isParallel() && itsComms.isMaster()) 
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
	  if(itsComms.isParallel() && itsComms.isWorker()){
	    // need to call calcObjectParams only, so that the distributed calculation works
	    ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Calculating the parameters in a distributed manner via calcObjectParams()");
	    this->calcObjectParams();
	  }


            if (!itsComms.isParallel() || itsComms.isMaster()) {
                ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Beginning the cleanup");

                duchamp::FitsHeader head = this->itsCube.getHead();
                std::vector<sourcefitting::RadioSource>::iterator src;

		ASKAPLOG_DEBUG_STR(logger, "Refining non-edge source list of size " << this->itsSourceList.size());
                for (src = this->itsSourceList.begin(); src < this->itsSourceList.end(); src++) {
                    src->setHeader(head);

                    // Need to check that there are no small sources present that violate the minimum size criteria
                    if (!(src->hasEnoughChannels(this->itsCube.pars().getMinChannels()))
			|| (src->getSpatialSize() < this->itsCube.pars().getMinPix()))
                        this->itsSourceList.erase(src);
                }
		ASKAPLOG_DEBUG_STR(logger, "Now have " << this->itsSourceList.size() << " non-edge sources");

		ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "num edge sources in cube = " << this->itsEdgeSourceList.size());

		this->itsCube.clearDetectionList();

                float threshold;

                if (this->itsCube.pars().getFlagUserThreshold()) threshold = this->itsCube.pars().getThreshold();
                else threshold = this->itsCube.stats().getThreshold();

                if (this->itsEdgeSourceList.size() > 0) { // if there are edge sources
                    for (src = this->itsEdgeSourceList.begin(); src < this->itsEdgeSourceList.end(); src++) this->itsCube.addObject(*src);
                    ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "num edge sources in cube = " << this->itsCube.getNumObj());
                    bool growthflag = this->itsCube.pars().getFlagGrowth();
                    this->itsCube.pars().setFlagGrowth(false);  // can't grow as don't have flux array in itsCube
                    ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Merging edge sources");
                    this->itsCube.ObjectMerger();
                    ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "num edge sources in cube after merging = " << this->itsCube.getNumObj());
                    this->itsCube.pars().setFlagGrowth(growthflag);
		    this->calcObjectParams();
                    ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "num edge sources in cube after param calcs = " << this->itsCube.getNumObj());

		    this->itsEdgeSourceList.clear();
//D1.1.13           for (long i = 0; i < this->itsCube.getNumObj(); i++) {
                    for (size_t i = 0; i < this->itsCube.getNumObj(); i++) {
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

			    for(int t=1;t<=2;t++)
			      src.findSpectralTerm(this->itsCube.pars().getImageFile(), t, this->itsFlagFindSpectralIndex);
			  }
			}
                        this->itsEdgeSourceList.push_back(src);
                    }
                }
		else this->calcObjectParams(); // if no edge sources, call this anyway so the workers know what to do...

		if(this->itsFlagDistribFit && this->itsFlagDoFit) this->fitRemaining();

                ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Finished cleaning up " << this->itsEdgeSourceList.size() <<" edge sources");

		for(src=this->itsEdgeSourceList.begin(); src<this->itsEdgeSourceList.end();src++){
		  this->itsSourceList.push_back(*src);
		}
		this->itsEdgeSourceList.clear();

                ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Now have a total of " << this->itsSourceList.size() << " sources.");

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
		SortDetections(this->itsSourceList, this->itsCube.pars().getSortingParam());
		newlist.clear();
                ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Finished sort of source list");
                this->itsCube.clearDetectionList();

                for (src = this->itsSourceList.begin(); src < this->itsSourceList.end(); src++) {
                    src->setID(src - this->itsSourceList.begin() + 1);
                    src->setAtEdge(this->itsCube, this->itsSubimageDef, itsComms.rank() - 1);

                    if (src->isAtEdge()) src->addToFlagText("E");
                    else src->addToFlagText("-");

                    this->itsCube.addObject(duchamp::Detection(*src));
                }

                ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Finished adding sources to cube. Now have " << this->itsCube.getNumObj() << " objects.");

            }
	    else if(itsComms.isWorker() && this->itsFlagDistribFit && this->itsFlagDoFit) this->fitRemaining();
        }

        //**************************************************************//

      void DuchampParallel::calcObjectParams()
      {
	if(itsComms.isParallel()){
	  if(itsComms.isMaster()) {
	    int16 rank;
	    LOFAR::BlobString bs;

	    int objsize=0;
	    duchamp::Section fullSec=this->itsCube.pars().section();
	    //	    ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Using subsection for box calcs: " << fullSec.getSection());
	    // now send the individual sources to each worker in turn
//D1.1.13   for(int i=0;i<this->itsCube.getNumObj();i++){
	    for(size_t i=0;i<this->itsCube.getNumObj();i++){
	      rank = i % (itsComms.nProcs() - 1);
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
	      itsComms.sendBlob(bs, rank + 1); // First worker is rank 1
	    }
	    // now send a zero size to tell everyone the list has finished.
	    objsize=0;
	    bs.resize(0);
	    LOFAR::BlobOBufString bob(bs);
	    LOFAR::BlobOStream out(bob);
	    out.putStart("paramsrc", 1);
	    out << objsize;
	    out.putEnd();
	    for (int i = 1; i < itsComms.nProcs(); ++i) {
	      itsComms.sendBlob(bs, i);
	    }
	    // now read back the sources from the workers
	    // Only need to do this if we actually sent something in the first place.
	    if(this->itsCube.getNumObj()>0){
	      this->itsEdgeSourceList.clear();
//D1.1.13     int numObj=this->itsCube.getNumObj();
	      size_t numObj=this->itsCube.getNumObj();
	      this->itsCube.clearDetectionList();
	      for (int n=0;n<itsComms.nProcs()-1 && this->itsCube.getNumObj()<numObj;n++){
		int numSrc;
		ASKAPLOG_DEBUG_STR(logger, "Master about to read from worker #"<< n+1);
		itsComms.receiveBlob(bs, n + 1);
		LOFAR::BlobIBufString bib(bs);
		LOFAR::BlobIStream in(bib);
		int version = in.getStart("final");
		ASKAPASSERT(version == 1);
		in >> numSrc;
		for(int i=0;i<numSrc;i++){
		  sourcefitting::RadioSource src;
		  in >> src;
		  this->itsEdgeSourceList.push_back(src);
		  this->itsCube.addObject(src);
		}
		in.getEnd();
	      }
	    }
	  }
	  else if(itsComms.isWorker()){
	    ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Setting up cube in preparation for object calculation");
	    this->itsCube.pars().setSubsection(this->itsBaseSubsection); // take care of any global offsets due to subsectioning
	    this->getMetadata();

	    // now read individual sources
	    LOFAR::BlobString bs;
	    int objsize=1;
	    this->itsCube.clearDetectionList();
	    while(objsize>0) {	    
	      itsComms.receiveBlob(bs, 0);
	      LOFAR::BlobIBufString bib(bs);
	      LOFAR::BlobIStream in(bib);
	      int version = in.getStart("paramsrc");
	      ASKAPASSERT(version == 1);
	      in >> objsize;
	      //	      ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Object calcs: Reading object size=" << objsize << " from Master");
	      if(objsize>0){
		//		ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Object calcs: Got OK to read object");
		sourcefitting::RadioSource src;
		in >> src;
		ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Object calcs: Read object of size " << src.getSize() << " from Master");
		this->itsCube.addObject(src);
	      }
	      in.getEnd();
	    }

	    if(this->itsCube.getNumObj() > 0){
	      int numVox = this->itsVoxelList.size();
	      int numObj = this->itsCube.getNumObj();

	      ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Read all " << numObj << " objects. Now have to get voxels from list of " << numVox);

	      if (numObj > 0) {
		// 	      std::vector<PixelInfo::Voxel> templist[numObj];
                for (int i = 0; i < numObj; i++) {

		  // for each object, make a vector list of voxels that appear in it.
		  std::vector<PixelInfo::Voxel> objVoxList = this->itsCube.getObject(i).getPixelSet();
		  std::vector<PixelInfo::Voxel>::iterator vox;
		  
		  // get the fluxes of each voxel
		  // 		  for (vox = objVoxList.begin(); vox < objVoxList.end(); vox++) {
		  // 		    int ct = 0;
		    
		  // 		    while (ct < numVox && !vox->match(this->itsVoxelList[ct])) {
		  // 		      ct++;
		  // 		    }
		    
		  // 		    if (numVox != 0 && ct == numVox) { // there has been no match -- problem!
		  // 		      ASKAPLOG_ERROR_STR(logger, this->workerPrefix() << "Found a voxel ("<< vox->getX() << "," << vox->getY()<< ") in the object lists that doesn't appear in the base list.");
		  // 		    } else vox->setF(this->itsVoxelList[ct].getF());
		  // 		  }
		  
		  // 		  for (vox = objVoxList.begin(); vox < objVoxList.end(); vox++) {
		  for(size_t v=0;v<objVoxList.size();v++){
		    // 		    if(v%1000 == 0 ) ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "finding flux for point #"<<v);
		    if(this->itsVoxelMap.find(objVoxList[v])==this->itsVoxelMap.end()){
		      ASKAPLOG_ERROR_STR(logger, this->workerPrefix() << "found a voxel ("
					 << objVoxList[v].getX() << "," << objVoxList[v].getY()
					 << ") in the object lists that doesn't appear in the base list.");
		    }
		    else objVoxList[v].setF(this->itsVoxelMap[objVoxList[v]]);
		  }

		  // 		  templist[i] = objVoxList;
                }
		
		ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Allocated fluxes to voxel lists. Now calculating parameters");
		
		//                 std::vector< std::vector<PixelInfo::Voxel> > bigVoxSet(templist, templist + numObj);
		//                 this->itsCube.calcObjectWCSparams(bigVoxSet);
		this->itsCube.calcObjectWCSparams(this->itsVoxelMap);
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
	      itsComms.sendBlob(bs, 0);

	    }
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
//D1.1.13   int numVox = this->itsVoxelList.size();
//D1.1.13   int numObj = this->itsCube.getNumObj();
            size_t numVox = this->itsVoxelList.size();
            size_t numObj = this->itsCube.getNumObj();

	    ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "About to calculate parameters for all " << numObj << " objects");
            if (numObj > 0) {
                std::vector<PixelInfo::Voxel> templist[numObj];

//D1.1.13       for (int i = 0; i < this->itsCube.getNumObj(); i++) {
                for (size_t i = 0; i < this->itsCube.getNumObj(); i++) {

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
//D1.1.13             int ct = 0;
		      size_t ct = 0;

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

//D1.1.13       for (int i = 0; i < this->itsCube.getNumObj(); i++) {
                for (size_t i = 0; i < this->itsCube.getNumObj(); i++) {

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
	if(itsComms.isParallel()){
	  if(itsComms.isMaster()) {
	    LOFAR::BlobString bs;

	    // first send the voxel list to all workers
	    /// @todo This could be made more efficient, so that we don't send unnecessary voxels to some workers.
	    bs.resize(0);
	    LOFAR::BlobOBufString bob(bs);
	    LOFAR::BlobOStream out(bob);
	    ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Broadcasting voxel list of size " << this->itsVoxelList.size() << " to all workers");
	    out.putStart("voxels", 1);
	    out << int(this->itsVoxelList.size());
	    for(size_t p=0;p<this->itsVoxelList.size();p++) 
	      out << int32(this->itsVoxelList[p].getX()) << int32(this->itsVoxelList[p].getY()) << int32(this->itsVoxelList[p].getZ()) << this->itsVoxelList[p].getF();
	    out.putEnd();
	    itsComms.broadcastBlob(bs, 0);
	  }
	  else if(itsComms.isWorker()){
	    // first read the voxel list
	    this->itsVoxelList.clear();
	    // 	    bool(*fn_pt)(PixelInfo::Voxel,PixelInfo::Voxel)=voxComp;
	    // 	    this->itsVoxelMap = std::map<PixelInfo::Voxel,float>(fn_pt);
	    LOFAR::BlobString bs;
	    itsComms.broadcastBlob(bs, 0);
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
	      PixelInfo::Voxel vox(x,y,z,f);
	      this->itsVoxelList.push_back(vox);
	      vox.setF(0.);
	      this->itsVoxelMap[vox] = f;
	      // 	      this->itsVoxelMap.insert(std::pair(PixelInfo::Voxel(x,y,z,0),f));
	      //	      this->itsVoxelMap[PixelInfo::Voxel(x,y,z,0)] = f;
	    }
	    in.getEnd();
	  }
	}
      }
      
      //**************************************************************//

      void DuchampParallel::fitRemaining()
      {
	if(itsComms.isParallel()){
	  if(itsComms.isMaster()) {
	    int16 rank;
	    LOFAR::BlobString bs;
	      
	    ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Distributing " << this->itsEdgeSourceList.size()<<" edge sources to be fit by workers");

	    // now send the individual sources to each worker in turn
	    for(size_t i=0;i<this->itsEdgeSourceList.size();i++){
	      //	      ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Preparing source #"<<i+1);
	      this->prepareSourceForFit(this->itsEdgeSourceList[i],false);
	      rank = i % (itsComms.nProcs() - 1);
	      ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Sending source #"<<i+1<<" of size " << this->itsEdgeSourceList[i].getSize() << " to worker "<<rank+1);
	      bs.resize(0);
	      LOFAR::BlobOBufString bob(bs);
	      LOFAR::BlobOStream out(bob);
	      out.putStart("fitsrc", 1);
	      out << true << this->itsEdgeSourceList[i];
	      out.putEnd();
	      itsComms.sendBlob(bs, rank + 1);
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
	    for (int i = 1; i < itsComms.nProcs(); ++i) {
	      itsComms.sendBlob(bs, i);
	    }

	    // now read back the sources from the workers
	    this->itsEdgeSourceList.clear();
	    for (int n=0;n<itsComms.nProcs()-1;n++){
	      int numSrc;
	      ASKAPLOG_INFO_STR(logger, "Master about to read from worker #"<< n+1);
	      itsComms.receiveBlob(bs, n + 1);
	      LOFAR::BlobIBufString bib(bs);
	      LOFAR::BlobIStream in(bib);
	      int version = in.getStart("final");
	      ASKAPASSERT(version == 1);
	      in >> numSrc;
	      for(int i=0;i<numSrc;i++){
		sourcefitting::RadioSource src;
		in >> src;
		src.setHeader(this->itsCube.getHead());  // make sure we have the right WCS etc information
		this->itsEdgeSourceList.push_back(src);
	      }
	      in.getEnd();
	    }
	      
	    ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Finished fitting of edge sources");

	  }
	  else if(itsComms.isWorker()){
	    ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Fitting edge sources passed from master.");
	      
	    ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Setting up cube in preparation for source fitting");
	    this->itsCube.pars().setSubsection("");
	    //	    casaImageToMetadata(this->itsCube, this->itsSubimageDef, -1);
	    this->getCASA(METADATA,false);
	    LOFAR::BlobString bs;
	      
	    // now read individual sources
	    bool isOK=true;
	    this->itsSourceList.clear();
	    while(isOK) {	    
	      sourcefitting::RadioSource src;
	      itsComms.receiveBlob(bs, 0);
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
	    ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Have fitted to " << this->itsSourceList.size() << " edge sources. Returning results to master.");
	    bs.resize(0);
	    LOFAR::BlobOBufString bob(bs);
	    LOFAR::BlobOStream out(bob);
	    out.putStart("final", 1);
	    out << int(this->itsSourceList.size());
	    for(size_t i=0;i<this->itsSourceList.size();i++) out << this->itsSourceList[i];
	    out.putEnd();
	    itsComms.sendBlob(bs, 0);
	      
	  }
	}

      }

        //**************************************************************//

        void DuchampParallel::printResults()
        {
            /// @details The final list of detected objects is written to
            /// the terminal and to the results file in the standard Duchamp
            /// manner.
            if (itsComms.isMaster()) {

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


		if(this->itsFlagDoFit){

		  std::vector<duchamp::Column::Col> columns = this->itsCube.getFullCols();
		  
		  for (size_t t = 0; t < outtypes.size(); t++) {
                    std::ofstream summaryFile(sourcefitting::convertSummaryFile(this->itsFitSummaryFile.c_str(), outtypes[t]).c_str());
                    std::vector<sourcefitting::RadioSource>::iterator src = this->itsSourceList.begin();
		    
                    for (; src < this->itsSourceList.end(); src++)
		      src->printSummary(summaryFile, columns, outtypes[t], src == this->itsSourceList.begin());
		    
                    summaryFile.close();
		  }
		  
		  if (this->itsFlagDoFit) this->writeFitAnnotation();

		}

            } else {
            }
        }

        //**************************************************************//

      void DuchampParallel::extractSpectra()
      {
	if(this->itsFlagExtractSpectra && this->itsComms.isMaster()){

	  std::vector<sourcefitting::RadioSource>::iterator src;
	  for (src = this->itsSourceList.begin(); src < this->itsSourceList.end(); src++) {
	    SourceSpectrumExtractor extractor(this->itsParset);
	    extractor.setSource(&*src);
	    extractor.extract();
 	    extractor.writeImage();
	  }

	}
	
      }

      void DuchampParallel::saveMask()
      {
	
	if(!this->itsComms.isParallel()){
	  
	  if(this->itsCube.pars().getFlagOutputMask()){
	    ASKAPLOG_INFO_STR(logger, "Saving mask cube to "<< this->itsCube.pars().outputMaskFile());
	    if(this->itsCube.saveMaskCube() == FAILURE)
	      ASKAPLOG_ERROR_STR(logger, "Unable to save the mask cube to " << this->itsCube.pars().outputMaskFile());
	  }
 
	}

      }


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
                outfile << "#FONT lucidasans-12\n";
                std::ofstream outfile2;

                if (this->itsFitAnnotationFile != this->itsFitBoxAnnotationFile) {
                    outfile2.open(this->itsFitBoxAnnotationFile.c_str());
                    outfile << "COLOR BLUE\n";
                    outfile << "COORD W\n";
                    outfile << "#FONT lucidasans-12\n";
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
	/// stddev/MADFM for the entire dataset and store these values in
	/// the master's itsCube statsContainer.
	if (this->itsFlagDoMedianSearch){
	  if(this->itsCube.pars().getFlagUserThreshold())
	    ASKAPLOG_WARN_STR(logger, "Since median searching has been requested, the threshold given ("
			      <<this->itsCube.pars().getThreshold()<<") is changed to a S/N-based one of "<<this->itsCube.pars().getCut()<<" sigma");
	  ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Setting user threshold to " << this->itsCube.pars().getCut());
	  this->itsCube.pars().setThreshold(this->itsCube.pars().getCut());
	  this->itsCube.pars().setFlagUserThreshold(true);
	  if(this->itsCube.pars().getFlagGrowth()){
	    ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Setting user growth threshold to " << this->itsCube.pars().getGrowthCut());
	    this->itsCube.pars().setGrowthThreshold(this->itsCube.pars().getGrowthCut());
	    this->itsCube.pars().setFlagUserGrowthThreshold(true);
	  }
	  this->itsCube.stats().setThreshold(this->itsCube.pars().getCut());
	  
	}
	else if(!this->itsComms.isParallel() || this->itsFlagThresholdPerWorker){
	  if(this->itsComms.isWorker()){
	    if(this->itsComms.isParallel())
	      ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Calculating stats for each worker individually");
	    else
	      ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Calculating stats");
	    this->itsCube.setCubeStats();
	    ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Stats are as follows:");
	    std::cout << this->itsCube.stats();
	  }
	  if(this->itsComms.isParallel() && this->itsComms.isMaster()){
	    this->itsCube.stats().setThreshold(this->itsCube.pars().getCut());
	    this->itsCube.pars().setThreshold(this->itsCube.pars().getCut());
	  }
	  else                          this->itsCube.pars().setThreshold(this->itsCube.stats().getThreshold());
	  this->itsCube.pars().setFlagUserThreshold(true);
	  ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Threshold = " << this->itsCube.stats().getThreshold());
	}    
	else if (!this->itsFlagDoMedianSearch &&
		 (!this->itsCube.pars().getFlagUserThreshold() ||
		  (this->itsCube.pars().getFlagGrowth() && !this->itsCube.pars().getFlagUserGrowthThreshold()))) {
	  findMeans();
	  combineMeans();
	  broadcastMean();
	  findStddevs();
	  combineStddevs();
	} else{
	  this->itsCube.stats().setThreshold(this->itsCube.pars().getThreshold());
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
            if (itsComms.isWorker()) {
                if (itsComms.isParallel()) {
//                     ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Finding mean");

                    if (this->itsCube.pars().getFlagATrous()) this->itsCube.ReconCube();
                    else if (this->itsCube.pars().getFlagSmooth()) this->itsCube.SmoothCube();

                    int32 size = 0;
                    float mean = 0., stddev;
		    if(!this->itsCube.pars().getFlagStatSec() || this->itsCube.pars().statsec().isValid()) {
		      float *array;
		      // make a mask in case there are blank pixels.
		      bool *mask = this->itsCube.pars().makeStatMask(this->itsCube.getArray(), this->itsCube.getDimArray());
//D1.1.13 	      for(int i=0;i<this->itsCube.getSize();i++) if(mask[i]) size++;		      
		      for(size_t i=0;i<this->itsCube.getSize();i++) if(mask[i]) size++;		      

		      if (size > 0) {
                        if (this->itsCube.pars().getFlagATrous())       array = this->itsCube.getArray();
                        else if (this->itsCube.pars().getFlagSmooth()) array = this->itsCube.getRecon();
                        else                                     array = this->itsCube.getArray();
			
                        // calculate both mean & stddev, but ignore stddev for the moment.
                        if (this->itsCube.pars().getFlagRobustStats()) 
			  findMedianStats(array, this->itsCube.getSize(), mask, mean, stddev);
                        else
			  findNormalStats(array, this->itsCube.getSize(), mask, mean, stddev);
		      }
		      ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Mean = " << mean);
		    }
		    else {
		      // No good points in the stats section
		      mean = 0.;
// 		      ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "No Mean calculated as no pixels in stat sec");
		    }
		    double dmean = mean;
		    LOFAR::BlobString bs;
		    bs.resize(0);
		    LOFAR::BlobOBufString bob(bs);
		    LOFAR::BlobOStream out(bob);
		    out.putStart("meanW2M", 1);
		    int16 rank = itsComms.rank();
		    out << rank << dmean << size;
		    out.putEnd();
		    itsComms.sendBlob(bs, 0);
		      //                     ASKAPLOG_DEBUG_STR(logger, "Sent mean to the master from worker " << itsComms.rank());

                } else {
                    // serial case -- can just calculate all stats at once.
//                     ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Calculating stats");
//                     this->itsCube.setCubeStats();
//                     ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Stats are as follows:");
//                     std::cout << this->itsCube.stats();
                }
            } else {
            }
        }

        //**************************************************************//

        void DuchampParallel::findStddevs()
        {
            /// @details In the parallel case, this finds the stddev or the
            /// median absolute deviation from the median (MADFM) (dictated
            /// by the flagRobustStats parameter) of the worker's
            /// image/cube, then sends that value to the master via LOFAR
            /// Blobs. To calculate the stddev/MADFM, the mean of the full
            /// dataset must be read from the master (again passed via LOFAR
            /// Blobs). The calculation uses the findSpread() function.
            ///
            /// In the serial case, nothing is done, as we have already
            /// calculated the stddev in the findMeans() function.
            if (itsComms.isWorker() && itsComms.isParallel()) {
//                 ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "About to calculate stddev");
                // first read in the overall mean for the cube
                double mean = 0;

                if (itsComms.isParallel()) {
                    LOFAR::BlobString bs1;
                    itsComms.receiveBlob(bs1, 0);
                    LOFAR::BlobIBufString bib(bs1);
                    LOFAR::BlobIStream in(bib);
                    int version = in.getStart("meanM2W");
                    ASKAPASSERT(version == 1);
                    in >> mean;
                    in.getEnd();
                } else {
                    mean = this->itsCube.stats().getMiddle();
                }

                // use it to calculate the stddev for this section
                int32 size = 0;
                double stddev = 0.;
		if(!this->itsCube.pars().getFlagStatSec() || this->itsCube.pars().statsec().isValid()) {
		  float *array;
		  
		  if (this->itsCube.pars().getFlagATrous()) {
		    array = new float[this->itsCube.getSize()];
		    
//D1.1.13 	    for (int i = 0; i < this->itsCube.getSize(); i++) array[i] = this->itsCube.getPixValue(i) - this->itsCube.getReconValue(i);
		    for (size_t i = 0; i < this->itsCube.getSize(); i++) array[i] = this->itsCube.getPixValue(i) - this->itsCube.getReconValue(i);
		  } else if (this->itsCube.pars().getFlagSmooth()) array = this->itsCube.getRecon();
		  else array = this->itsCube.getArray();
		  
		  bool *mask = this->itsCube.pars().makeStatMask(array, this->itsCube.getDimArray());
//D1.1.13 	  for(int i=0;i<this->itsCube.getSize();i++) if(mask[i]) size++;		      
		  for(size_t i=0;i<this->itsCube.getSize();i++) if(mask[i]) size++;		      

		  if(size>0)
		    stddev = findSpread(this->itsCube.pars().getFlagRobustStats(), mean, this->itsCube.getSize(), array, mask);
		  
		  if (this->itsCube.pars().getFlagATrous()) delete [] array;
		  ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "StdDev = " << stddev);
		}
		else {
		  // Only way to get here is for flagStatSec=true but statsec is invalid (ie. has no pixels in this worker)
		  stddev = 0.;
// 		  ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "No pixels to calculate StdDev");
		}

                // return it to the master
                LOFAR::BlobString bs2;
                bs2.resize(0);
                LOFAR::BlobOBufString bob(bs2);
                LOFAR::BlobOStream out(bob);
                out.putStart("stddevW2M", 1);
                int16 rank = itsComms.rank();
                out << rank << stddev << size;
                out.putEnd();
                itsComms.sendBlob(bs2, 0);
//                 ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Sent local stddev to the master");
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
            if (itsComms.isMaster() && itsComms.isParallel() && !this->itsFlagThresholdPerWorker) {
                // get the means from the workers
//                 ASKAPLOG_DEBUG_STR(logger,  this->workerPrefix() << "Receiving Means and combining");
                LOFAR::BlobString bs1;
                int64 size = 0;
                double av = 0;

                for (int i = 1; i < itsComms.nProcs(); i++) {
                    itsComms.receiveBlob(bs1, i);
                    LOFAR::BlobIBufString bib(bs1);
                    LOFAR::BlobIStream in(bib);
                    int version = in.getStart("meanW2M");
                    ASKAPASSERT(version == 1);
                    double newav;
                    int32 newsize;
                    int16 rank;
                    in >> rank >> newav >> newsize;
                    in.getEnd();
//                     ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Received mean from worker " << rank);
                    size += newsize;
                    av += newav * newsize;
                }

                if (size > 0) {
                    av /= double(size);
                }

                ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Overall size = " << size);
                ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Overall mean = " << av);
                this->itsCube.stats().setMean(av);
            } else {
            }
        }

        //**************************************************************//

        void DuchampParallel::broadcastMean()
        {
            /// @details The mean/median value of the full dataset is sent
            /// via LOFAR Blobs to the workers.
            if (itsComms.isMaster() && itsComms.isParallel() && !this->itsFlagThresholdPerWorker) {
                // now send the overall mean to the workers so they can calculate the stddev
                double av = this->itsCube.stats().getMean();
                LOFAR::BlobString bs2;
                bs2.resize(0);
                LOFAR::BlobOBufString bob(bs2);
                LOFAR::BlobOStream out(bob);
                out.putStart("meanM2W", 1);
                out << av;
                out.putEnd();
                for (int i = 1; i < itsComms.nProcs(); ++i) {
                    itsComms.sendBlob(bs2, i);
                }
//                 ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Broadcast overall mean from master to workers.");
            } else {
            }
        }

        //**************************************************************//

        void DuchampParallel::combineStddevs()
        {
            /// @details The master reads the stddev/MADFM values from each of
            /// the workers, and combines them to produce an estimate of the
            /// stddev for the full cube. Again, if MADFM values have been
            /// calculated on the workers, they are treated as estimates of
            /// the stddev and are combined as if they are stddev values. The
            /// overall value is stored in the StatsContainer in itsCube.
            if (itsComms.isMaster() && itsComms.isParallel() && !this->itsFlagThresholdPerWorker) {
                // get the means from the workers
//                 ASKAPLOG_DEBUG_STR(logger,  this->workerPrefix() << "Receiving STDDEV values and combining");
                LOFAR::BlobString bs;
                int64 size = 0;
                double stddev = 0;

                for (int i = 1; i < itsComms.nProcs(); i++) {
                    itsComms.receiveBlob(bs, i);
                    LOFAR::BlobIBufString bib(bs);
                    LOFAR::BlobIStream in(bib);
                    int version = in.getStart("stddevW2M");
                    ASKAPASSERT(version == 1);
                    double newstddev;
                    int32 newsize;
                    int16 rank;
                    in >> rank >> newstddev >> newsize;
                    in.getEnd();
//                     ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Received stddev from worker " << rank);
                    size += newsize;
		    if(newsize>0)
		      stddev += (newstddev * newstddev * (newsize - 1));
                }

                if (size > 0) {
                    stddev = sqrt(stddev / double(size - 1));
                }

                this->itsCube.stats().setStddev(stddev);
                this->itsCube.stats().setRobust(false);

                if (!this->itsCube.pars().getFlagUserThreshold()) {
                    this->itsCube.stats().setThresholdSNR(this->itsCube.pars().getCut());
                    this->itsCube.pars().setFlagUserThreshold(true);
                    this->itsCube.pars().setThreshold(this->itsCube.stats().getThreshold());
                }

                ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Overall StdDev = " << stddev);
            }
        }

        //**************************************************************//

        void DuchampParallel::broadcastThreshold()
        {
            /// @details The detection threshold value (which has been
            /// already calculated) is sent to the workers via LOFAR Blobs.
            if (itsComms.isMaster() && itsComms.isParallel() && !this->itsFlagThresholdPerWorker) {
                // now send the overall mean to the workers so they can calculate the stddev
                LOFAR::BlobString bs;
                bs.resize(0);
                LOFAR::BlobOBufString bob(bs);
                LOFAR::BlobOStream out(bob);
                out.putStart("threshM2W", 1);
                double threshold = this->itsCube.stats().getThreshold();
                double mean = this->itsCube.stats().getMiddle();
                double stddev = this->itsCube.stats().getSpread();
                out << threshold << mean << stddev;
                out.putEnd();
                itsComms.broadcastBlob(bs, 0);
		ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Threshold = " << this->itsCube.stats().getThreshold());
//                 ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Sent threshold ("
//                                       << this->itsCube.stats().getThreshold() << ") from the master");
            } else {
            }
        }


        //**************************************************************//

        void DuchampParallel::receiveThreshold()
        {
            /// @details The workers read the detection threshold sent via LOFAR Blobs from the master.
            if (itsComms.isWorker() && !this->itsFlagThresholdPerWorker) {
                double threshold, mean, stddev;

                if (itsComms.isParallel()) {
                    LOFAR::BlobString bs;
                    itsComms.broadcastBlob(bs, 0);
                    LOFAR::BlobIBufString bib(bs);
                    LOFAR::BlobIStream in(bib);
                    int version = in.getStart("threshM2W");
                    ASKAPASSERT(version == 1);
                    in >> threshold >> mean >> stddev;
                    in.getEnd();
                    this->itsCube.stats().setRobust(false);
                    this->itsCube.stats().setMean(mean);
                    this->itsCube.stats().setStddev(stddev);

//                     if (!this->itsCube.pars().getFlagUserThreshold() && !this->itsFlagDoMedianSearch)
//                         ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Setting mean to be " << mean << " and stddev " << stddev);

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


      //**************************************************************//

      long *getDim(const ImageInterface<Float>* imagePtr)
      {
	IPosition shape = imagePtr->shape();
	long *dim = new long[shape.size()];

	for (size_t i = 0; i < shape.size(); i++) {
	  dim[i] = shape(i);
	  ASKAPCHECK(dim[i] > 0, "Negative dimension: dim[" << i << "]=" << dim[i]);
	}
	
	return dim;
      }

	
      void reportDim(long *dim, size_t size)
      {

	std::stringstream ss;
	for (size_t i = 0; i < size; i++) {
	  ss << dim[i];
	  if(i < size-1) ss << " x ";
	}
	
	ASKAPLOG_INFO_STR(logger, "Dimensions of input image = " << ss.str());
	
      }

      //**************************************************************//

      duchamp::OUTCOME DuchampParallel::getCASA(DATATYPE typeOfData, bool useSubimageInfo)
      {

	/// @details This is the front-end to the image-access
	/// functionality for CASA images. It replicates (kinda) the
	/// behaviour of duchamp::Cube::getCube(). First the image is
	/// opened, then we get the metadata for the image via
	/// getCasaMetadata. Then the subimage that we want is defined
	/// (including the parsing of any subsections given in the
	/// parset), then, if we request IMAGE data, the actual pixel
	/// values are read from the image and stored in the itsCube
	/// object.
	/// @param typeOfData What sort of data are we after? Image
	/// data or just Metadata?
	/// @param useSubimageInfo Whether to use the information of
	/// the distributed nature of the data to determine the
	/// subimage shape, or whether just to get the whole image
	/// dimensions.
	/// @return duchamp::SUCCESS if successfull, duchamp::FAILURE otherwise.

	ImageOpener::registerOpenImageFunction(ImageOpener::FITS, FITSImage::openFITSImage);
	ImageOpener::registerOpenImageFunction(ImageOpener::MIRIAD, MIRIADImage::openMIRIADImage);
	const LatticeBase* lattPtr = ImageOpener::openImage(this->itsCube.pars().getImageFile());
	if (lattPtr == 0)
	  ASKAPTHROW(AskapError, "Requested image \"" << this->itsCube.pars().getImageFile() << "\" does not exist or could not be opened.");
	const ImageInterface<Float>* imagePtr = dynamic_cast<const ImageInterface<Float>*>(lattPtr);


	// Define the subimage - need to be done before metadata, as the latter needs the subsection & offsets
	const SubImage<Float> *sub = this->getSubimage(imagePtr, useSubimageInfo);

	if(this->getCasaMetadata(sub, typeOfData) == duchamp::FAILURE) return duchamp::FAILURE;

	ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Have subimage with shape " << sub->shape() << " and subsection " << this->itsCube.pars().section().getSection());

	if(typeOfData == IMAGE){
	  
	  ASKAPLOG_INFO_STR(logger, "Reading data from image " << this->itsCube.pars().getImageFile());

	  long *dim = getDim(sub);
	  this->itsCube.initialiseCube(dim);
	  if(this->itsCube.getDimZ()==1){
	    this->itsCube.pars().setMinChannels(0);
	  }
	  casa::Array<Float> array = sub->get();
	  this->itsCube.saveArray(array.data(), array.size());
	  
	}

	return duchamp::SUCCESS;

      }

      //**************************************************************//

      const SubImage<Float>* DuchampParallel::getSubimage(const ImageInterface<Float>* imagePtr, bool useSubimageInfo)
      {

	/// @details Define the shape/size of the subimage being used,
	/// and return a pointer that can be used to extract the image
	/// data. The subimage is defined by the itsSubimageDef
	/// object, which, when useSubimageInfo=true, takes into
	/// account how the image is distributed amongst workers. If
	/// useSubimageInfo=false, the whole image is considered. The
	/// image subsection and the statistics subsection are both
	/// parsed and tested for validity.
	/// @param imagePtr Pointer to the image - needs to be opened
	/// and valid
	/// @param useSubimageInfo Whether to use infomation about the
	/// distribution of the image amongst workers.
	/// @return A casa::SubImage pointer to the desired sub-image.

	wcsprm *wcs = casaImageToWCS(imagePtr);
	this->itsSubimageDef.define(wcs);
	this->itsSubimageDef.setImage(this->itsCube.pars().getImageFile());
	//	this->itsSubimageDef.setInputSubsection(this->itsCube.pars().getSubsection());
	this->itsSubimageDef.setInputSubsection(this->itsBaseSubsection);
	long *dim = getDim(imagePtr);
	reportDim(dim,imagePtr->ndim());
	this->itsSubimageDef.setImageDim(dim, imagePtr->ndim());
	
	//      ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Cube's param subsection = " << this->itsCube.pars().getSubsection() << " or " << this->itsCube.pars().section().getSection());
	
// 	if(useSubimageInfo){
// 	  //	  this->itsCube.pars().section() = this->itsSubimageDef.section(this->itsComms.rank()-1, this->itsCube.pars().getSubsection());
// // 	  if(this->itsComms.isMaster() && this->itsComms.isParallel()) this->itsCube.pars().setSubsection(nullSection(this->itsSubimageDef.getImageDim().size()));
// // 	  else this->itsCube.pars().section() = this->itsSubimageDef.section(this->itsComms.rank()-1);
// 	  if(!this->itsComms.isParallel() || this->itsComms.isWorker()) 
	if(useSubimageInfo && (!this->itsComms.isParallel() || this->itsComms.isWorker())){
	    this->itsCube.pars().section() = this->itsSubimageDef.section(this->itsComms.rank()-1);
	}
	else if (!this->itsCube.pars().getFlagSubsection() || this->itsCube.pars().getSubsection() == "") {
	  this->itsCube.pars().setSubsection(nullSection(this->itsSubimageDef.getImageDim().size()));
	}
	this->itsCube.pars().setFlagSubsection(true);

	//	ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Cube's param subsection = " << this->itsCube.pars().getSubsection() << " or " << this->itsCube.pars().section().getSection());
	// Now parse the sections to get them properly set up
	if(this->itsCube.pars().parseSubsections(dim, imagePtr->ndim()) == duchamp::FAILURE){
	  // if here, something went wrong
	  if (this->itsCube.pars().section().parse(dim, imagePtr->ndim()) == duchamp::FAILURE)
	    ASKAPTHROW(AskapError, "Cannot parse the subsection string " << this->itsCube.pars().section().getSection());
	  if (this->itsCube.pars().statsec().parse(dim, imagePtr->ndim()) == duchamp::FAILURE)
	    ASKAPTHROW(AskapError, "Cannot parse the statistics subsection string " << this->itsCube.pars().statsec().getSection());
	  if(!this->itsCube.pars().section().isValid())
	    ASKAPTHROW(AskapError, "Pixel subsection " << this->itsBaseSubsection << " has no pixels");
	  if(!this->itsCube.pars().statsec().isValid())
	    ASKAPTHROW(AskapError, "Statistics subsection " << this->itsBaseStatSubsection << " has no pixels in common with the image or the pixel subsection requested");
	}
	
	if(this->itsComms.isMaster() & this->itsCube.pars().getFlagStatSec() && !this->itsCube.pars().statsec().isValid())
	  ASKAPTHROW(AskapError, "Statistics subsection has no valid pixels");
	
	ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Using subsection " << this->itsCube.pars().section().getSection());
	if(this->itsCube.pars().getFlagStatSec() && this->itsCube.pars().statsec().isValid())
	  ASKAPLOG_INFO_STR(logger, this->workerPrefix() << "Using stat-subsection " << this->itsCube.pars().statsec().getSection());
	

	Slicer slice = subsectionToSlicer(this->itsCube.pars().section());
	fixSlicer(slice, wcs);
	//	ASKAPLOG_DEBUG_STR(logger, "Slicer used to make subimage is " << slice);

	const SubImage<Float> *sub = new SubImage<Float>(*imagePtr, slice);
	
	return sub;
      }

      //**************************************************************//

      duchamp::OUTCOME DuchampParallel::getCasaMetadata(const ImageInterface<Float>*  imagePtr, DATATYPE typeOfData)
      {

	/// @details Read some basic metadata from the image, storing
	/// the WCS information, beam information, flux units, setting
	/// the is2D flag and fixing spectral units if need be. If we
	/// want METADATA, then the cube is initialised without
	/// allocation (ie. the dimension array is set and some
	/// parameter flags are checked). Otherwise, initialisation is
	/// saved till later.
	/// @param imagePtr The image, already opened
	/// @param typeOfData Either IMAGE or METADATA

	long *dim = getDim(imagePtr);
	wcsprm *wcs = casaImageToWCS(imagePtr);
//D1.1.13 storeWCStoHeader(this->itsCube.header(), this->itsCube.pars(), wcs);
	ASKAPLOG_DEBUG_STR(logger, this->workerPrefix() << "Defining WCS and putting into type \""<<this->itsCube.pars().getSpectralType()<<"\"");
	this->itsCube.header().defineWCS(wcs,1,dim,this->itsCube.pars());
	this->itsCube.pars().setOffsets(wcs);
	//	ASKAPLOG_DEBUG_STR(logger, this->workerPrefix()<<"Pixelsec = " << this->itsCube.pars().section().getSection() <<" offsets = " << this->itsCube.pars().getXOffset() <<" " << this->itsCube.pars().getYOffset() <<" " << this->itsCube.pars().getZOffset() << " flagSubsection = " << this->itsCube.pars().getFlagSubsection());
	readBeamInfo(imagePtr, this->itsCube.header(), this->itsCube.pars());
	this->itsCube.header().setFluxUnits(imagePtr->units().getName());
	
	// check the true dimensionality and set the 2D flag in the cube header.
	this->itsCube.header().set2D(imagePtr->shape().nonDegenerate().size() <= 2);
	
	// set up the various flux units
//D1.1.13 if (wcs->spec >= 0) this->itsCube.header().fixUnits(this->itsCube.pars());
	if (wcs->spec >= 0) this->itsCube.header().fixSpectralUnits(this->itsCube.pars().getSpectralUnits());

	if(typeOfData == METADATA) this->itsCube.initialiseCube(dim, false);
	delete [] dim;
	return duchamp::SUCCESS;


      }


      //**************************************************************//




    }
}
