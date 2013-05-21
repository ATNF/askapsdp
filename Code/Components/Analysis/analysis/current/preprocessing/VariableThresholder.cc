#include <askap_analysis.h>
#include <preprocessing/VariableThresholder.h>
#include <preprocessing/VariableThresholdingHelpers.h>
#include <outputs/ImageWriter.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <Common/ParameterSet.h>
#include <duchamp/Cubes/cubes.hh>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/IPosition.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <images/Images/ImageOpener.h>
#include <images/Images/FITSImage.h>
#include <images/Images/MIRIADImage.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/PagedImage.h>
#include <images/Images/SubImage.h>

#include <casainterface/CasaInterface.h>

#include <string>

///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".varthresh");

namespace askap {

    namespace analysis {

	VariableThresholder::VariableThresholder(const LOFAR::ParameterSet &parset):
	    itsParset(parset)
	{
	    /// @details Initialise from a LOFAR parset. Define all
	    /// parameters save for the input image, the search type
	    /// and the robust stats flag - all of which are set
	    /// according to the duchamp::Cube parameters. If an
	    /// output image name is not provided, it will not be
	    /// written.

	    this->itsBoxSize = parset.getInt16("boxWidth",50);
	    this->itsSNRimageName = parset.getString("SNRimageName", "");
	    this->itsThresholdImageName = parset.getString("ThresholdImageName","");
	    this->itsNoiseImageName = parset.getString("NoiseImageName","");
	    this->itsAverageImageName = parset.getString("AverageImageName","");
	    this->itsBoxSumImageName = parset.getString("BoxSumImageName","");
	    this->itsInputImage="";
	    this->itsSearchType = "spatial";
	    this->itsCube = 0;
	    this->itsFlagRobustStats = true;
	}

	void VariableThresholder::setFilenames(askap::askapparallel::AskapParallel& comms)
	{
	    /// @details Updates the output image names in the case of
	    /// distributed processing. The names will have the worker
	    /// number appended to them (so that instead of something
	    /// like "image_snr" it will become "image_snr_6_9" for
	    /// worker #6 out of 9.
	    if(comms.isParallel()){
		std::stringstream suffix;
		suffix << "_" << comms.rank() << "_"<<comms.nProcs();
		if(this->itsSNRimageName!="") this->itsSNRimageName += suffix.str();
		if(this->itsNoiseImageName!="") this->itsNoiseImageName += suffix.str();
		if(this->itsBoxSumImageName!="") this->itsBoxSumImageName += suffix.str();
		if(this->itsAverageImageName!="") this->itsAverageImageName += suffix.str();
		if(this->itsThresholdImageName!="") this->itsThresholdImageName += suffix.str();
	    }
	}


	VariableThresholder::VariableThresholder(const VariableThresholder& other)
	{
	    this->operator=(other);
	}

	VariableThresholder& VariableThresholder::operator= (const VariableThresholder& other)
	{
	    if(this==&other) return *this;
	    this->itsParset = other.itsParset;
	    this->itsFlagRobustStats = other.itsFlagRobustStats;
	    this->itsSNRthreshold = other.itsSNRthreshold;
	    this->itsSearchType = other.itsSearchType;
	    this->itsBoxSize = other.itsBoxSize;
	    this->itsInputImage = other.itsInputImage;
	    this->itsSNRimageName = other.itsSNRimageName;
	    this->itsThresholdImageName = other.itsThresholdImageName;
	    this->itsNoiseImageName = other.itsNoiseImageName;
	    this->itsAverageImageName = other.itsAverageImageName;
	    this->itsBoxSumImageName = other.itsAverageImageName;
	    this->itsCube = other.itsCube;
	    this->itsInputShape = other.itsInputShape;
	    this->itsInputCoordSys = other.itsInputCoordSys;
	    return *this;
	}

	
	void VariableThresholder::initialise(duchamp::Cube &cube)
	{
	    /// @details Initialise the class with information from
	    /// the duchamp::Cube. This is done to avoid replicating
	    /// parameters and preserving the parameter
	    /// hierarchy. Once the input image is known, the output
	    /// image names can be set with fixName() (if they have
	    /// not been defined via the parset).

	    this->itsCube = &cube;
	    this->itsInputImage = cube.pars().getImageFile();
	    this->itsFlagRobustStats = cube.pars().getFlagRobustStats();
	    this->itsSNRthreshold = cube.pars().getCut();
	    this->itsSearchType = cube.pars().getSearchType();
	    ASKAPCHECK((this->itsSearchType=="spectral")||(this->itsSearchType=="spatial"),
		       "SearchType needs to be either 'spectral' or 'spatial' - you have " << this->itsSearchType);

	    ImageOpener::registerOpenImageFunction(ImageOpener::FITS, FITSImage::openFITSImage);
	    ImageOpener::registerOpenImageFunction(ImageOpener::MIRIAD, MIRIADImage::openMIRIADImage);
	    const LatticeBase* lattPtr = ImageOpener::openImage(cube.pars().getImageFile());
	    if (lattPtr == 0)
		ASKAPTHROW(AskapError, "Requested image \"" << cube.pars().getImageFile() << "\" does not exist or could not be opened.");
	    const ImageInterface<Float>* imagePtr = dynamic_cast<const ImageInterface<Float>*>(lattPtr);
	    casa::Slicer slicer = analysisutilities::subsectionToSlicer(cube.pars().section());
	    analysisutilities::fixSlicer(slicer, cube.header().getWCS());

	    const SubImage<Float> *sub = new SubImage<Float>(*imagePtr, slicer);
	    this->itsInputCoordSys = sub->coordinates();
	    this->itsInputShape = sub->shape();

	}


	void VariableThresholder::calculate()
	{
	    /// @details Calculate the signal-to-noise at each
	    /// pixel. The cube (if it is a cube) is broken up into a
	    /// series of lower dimensional data sets - the search
	    /// type parameter defines whether this is done as a
	    /// series of 2D images or 1D spectra. For each subset,
	    /// the "middle" (mean or median) and "spread" (standard
	    /// deviation or median absolute deviation from the
	    /// median) for each pixel are calculated, and the
	    /// signal-to-noise map is formed. At each stage, any
	    /// outputs are made, with the subset being written to the
	    /// appropriate image at the appropriate location. At the
	    /// end, the signal-to-noise map is written to the Cube's
	    /// reconstructed array, from where the detections can be
	    /// made.	    

	    ASKAPLOG_INFO_STR(logger, "Will calculate the pixel-by-pixel signal-to-noise map");
	    if(this->itsSNRimageName!="") ASKAPLOG_INFO_STR(logger, "Will write the SNR map to " << this->itsSNRimageName);
	    if(this->itsBoxSumImageName!="") ASKAPLOG_INFO_STR(logger, "Will write the box sum map to " << this->itsBoxSumImageName);
	    if(this->itsNoiseImageName!="") ASKAPLOG_INFO_STR(logger, "Will write the noise map to " << this->itsNoiseImageName);
	    if(this->itsAverageImageName!="") ASKAPLOG_INFO_STR(logger, "Will write the average background map to " << this->itsAverageImageName);
	    if(this->itsThresholdImageName!="") ASKAPLOG_INFO_STR(logger, "Will write the flux threshold map to " << this->itsThresholdImageName);

	    int specAxis=this->itsInputCoordSys.spectralAxisNumber();
	    int lngAxis=this->itsInputCoordSys.directionAxesNumbers()[0];
	    int latAxis=this->itsInputCoordSys.directionAxesNumbers()[1];
	    size_t spatsize=this->itsInputShape(lngAxis) * this->itsInputShape(latAxis);
	    size_t specsize=(specAxis>=0) ? this->itsInputShape(specAxis) : 1;
	    if(specsize<1) specsize=1;
	    casa::IPosition chunkshape=this->itsInputShape;
	    ASKAPLOG_DEBUG_STR(logger, "input shape = " << this->itsInputShape);
	    casa::IPosition box;
	    size_t maxCtr;
	    if(this->itsSearchType == "spatial"){
	    	if(specAxis>=0) chunkshape(specAxis)=1;
	    	box=casa::IPosition(2, this->itsBoxSize, this->itsBoxSize);
		maxCtr=specsize;
	    }
	    else{
		if(lngAxis>=0) chunkshape(lngAxis) = 1;
		if(latAxis>=0) chunkshape(latAxis) = 1;
		box=casa::IPosition(1, this->itsBoxSize);
		maxCtr=spatsize;
	    }

	    ASKAPLOG_INFO_STR(logger, "Will calculate box-wise signal-to-noise in '"<<this->itsSearchType<<"' mode with chunks of shape " << chunkshape << " and a box of shape " << box);

	    for(size_t ctr=0;ctr<maxCtr;ctr++){
		if(maxCtr>1) ASKAPLOG_DEBUG_STR(logger, "Iteration " << ctr << " of " << maxCtr);
		bool isStart=(ctr==0);
		casa::Array<Float> inputChunk(chunkshape,0.);
		defineChunk(inputChunk,ctr);
		casa::Array<Float> middle(chunkshape,0.);
		casa::Array<Float> spread(chunkshape,0.);
		slidingBoxStats(inputChunk, middle, spread, box, this->itsFlagRobustStats);

		casa::IPosition loc(this->itsInputShape.size(),0);
		if(this->itsSearchType == "spatial"){
	    	    if(specAxis>=0) loc(specAxis) = ctr;
		}
		else{
		    if(lngAxis>=0) loc(lngAxis) = ctr%this->itsCube->getDimX();
		    if(latAxis>=0) loc(latAxis) = ctr/this->itsCube->getDimX();
		}
		casa::Array<Float> snr = calcSNR(inputChunk,middle,spread);

		this->writeImages(middle,spread,snr,loc,isStart);
		this->doBoxSum(inputChunk,box,loc,isStart);

		ASKAPLOG_DEBUG_STR(logger, "About to store the SNR map to the cube for iteration " << ctr << " of " << maxCtr);
		this->saveSNRtoCube(snr,ctr);
	    }

	    this->itsCube->setReconFlag(true);
   
	}

	void VariableThresholder::defineChunk(casa::Array<Float> &chunk, size_t ctr)
	{
	    casa::Array<Float>::iterator iter(chunk.begin());
	    int lngAxis=this->itsInputCoordSys.directionAxesNumbers()[0];
	    int latAxis=this->itsInputCoordSys.directionAxesNumbers()[1];
	    size_t spatsize=this->itsInputShape(lngAxis) * this->itsInputShape(latAxis);
	    if(this->itsSearchType == "spatial"){
		for(size_t i=0;iter!=chunk.end();iter++,i++) *iter = this->itsCube->getArray()[i+ctr*spatsize];
	    }
	    else{
		for(size_t z=0;iter!=chunk.end();iter++,z++) *iter = this->itsCube->getArray()[ctr+z*spatsize];
	    }
	}

	void VariableThresholder::saveSNRtoCube(casa::Array<Float> &snr, size_t ctr)
	{
	    if(this->itsCube->getRecon()==0)
		ASKAPLOG_ERROR_STR(logger, "The Cube's recon array not defined - cannot save SNR map");
	    else{
		casa::Array<Float>::iterator iter(snr.begin());
		int lngAxis=this->itsInputCoordSys.directionAxesNumbers()[0];
		int latAxis=this->itsInputCoordSys.directionAxesNumbers()[1];
		size_t spatsize=this->itsInputShape(lngAxis) * this->itsInputShape(latAxis);
		if(this->itsSearchType == "spatial"){
		    for(size_t i=0;iter!=snr.end();iter++,i++) this->itsCube->getRecon()[i+ctr*spatsize] = *iter;
		}
		else{
		    for(size_t z=0;iter!=snr.end();iter++,z++) this->itsCube->getRecon()[ctr+z*spatsize] = *iter;
		}
	    }
	    
	}

	void VariableThresholder::writeImages(casa::Array<Float> &middle, casa::Array<Float> &spread, casa::Array<Float> &snr, casa::IPosition &loc, bool doCreate)
	{
	    /// @details Write all 

	    if(this->itsNoiseImageName!=""){
		ImageWriter imWriter(this->itsCube);
		if(doCreate) imWriter.create(this->itsNoiseImageName);
		imWriter.write(spread,loc);
	    }
	    if(this->itsAverageImageName!=""){
		ImageWriter imWriter(this->itsCube);
		if(doCreate) imWriter.create(this->itsAverageImageName);
		imWriter.write(middle,loc);
	    }
	    if(this->itsThresholdImageName!=""){
		casa::Array<Float> thresh = middle + this->itsSNRthreshold * spread;
		ImageWriter imWriter(this->itsCube);
		if(doCreate) imWriter.create(this->itsThresholdImageName);
		imWriter.write(thresh,loc);
	    }
	    if(this->itsSNRimageName!=""){
		ImageWriter imWriter(this->itsCube);
		if(doCreate) imWriter.create(this->itsSNRimageName);
		imWriter.write(snr,loc);
	    }

	}

	void VariableThresholder::doBoxSum(casa::Array<Float> &input, casa::IPosition &box, casa::IPosition &loc, bool doCreate)
	{
	    /// @details A function to calculate, for a given chunk,
	    /// the box-wise sum, and write it to an image. The
	    /// box-wise sum is, for each pixel, the sum of all pixels
	    /// within the current box centred on that pixel. This is
	    /// not considered part of the typical usage of the
	    /// variable thresholder, but is included as an optional
	    /// extra.

	    if(this->itsBoxSumImageName!=""){
		if(doCreate) ASKAPLOG_INFO_STR(logger, "Will calculate and write out the box sum image, saving to " << this->itsBoxSumImageName);
		casa::Array<Float> boxsum = slidingArrayMath(input, box, SumFunc<Float>());
		ImageWriter imWriter(this->itsCube);
		if(doCreate) imWriter.create(this->itsBoxSumImageName);
		imWriter.write(boxsum,loc);
	    }
	}



	void VariableThresholder::search()
	{
	    /// @details Once the signal-to-noise array is defined, we
	    /// extract objects from it based on the signal-to-noise
	    /// threshold. The resulting object list is put directly
	    /// into the duchamp::Cube object, where it can be
	    /// accessed from elsewhere. The detection map is updated
	    /// and the Duchamp log file can be written to (if
	    /// required).

	    if(this->itsCube->getRecon()==0)
		ASKAPLOG_ERROR_STR(logger, "The Cube's recon array not defined - cannot search for sources.");
	    else{
		if (!this->itsCube->pars().getFlagUserThreshold()) {
		    ASKAPLOG_DEBUG_STR(logger, "Setting user threshold to " << this->itsCube->pars().getCut() << " sigma");
		    this->itsCube->pars().setThreshold(this->itsCube->pars().getCut());
		    this->itsCube->pars().setFlagUserThreshold(true);
		    if(this->itsCube->pars().getFlagGrowth()){
			ASKAPLOG_DEBUG_STR(logger, "Setting user growth threshold to " << this->itsCube->pars().getGrowthCut() << " sigma");
			this->itsCube->pars().setGrowthThreshold(this->itsCube->pars().getGrowthCut());
			this->itsCube->pars().setFlagUserGrowthThreshold(true);
		    }
		}
	
		ASKAPLOG_DEBUG_STR(logger, "Searching SNR map");
		this->itsCube->ObjectList() = searchReconArray(this->itsCube->getDimArray(),this->itsCube->getArray(),this->itsCube->getRecon(),this->itsCube->pars(),this->itsCube->stats());
		ASKAPLOG_DEBUG_STR(logger, "Number of sources found = " << this->itsCube->getNumObj());
		this->itsCube->updateDetectMap();
		if(this->itsCube->pars().getFlagLog()) 
		    this->itsCube->logDetectionList();
	    }
	}
	

    }

}
