#include <askap_analysis.h>
#include <preprocessing/VariableThresholder.h>
#include <preprocessing/VariableThresholdingHelpers.h>
#include <outputs/ImageWriter.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <Common/ParameterSet.h>
#include <duchamp/Cubes/cubes.hh>
#include <string>

///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".varthresh");

namespace askap {

    namespace analysis {

	VariableThresholder::VariableThresholder(const LOFAR::ParameterSet &parset)
	{
	    this->itsBoxSize = parset.getInt16("boxWidth",50);
	    this->itsFlagWriteSNRimage = parset.getBool("flagWriteSNRimage", false);
	    this->itsSNRimageName = parset.getString("SNRimageName", "");
	    this->itsFlagWriteThresholdImage = parset.getBool("flagWriteThresholdImage",false);
	    this->itsThresholdImageName = parset.getString("ThresholdImageName","");
	    this->itsFlagWriteNoiseImage = parset.getBool("flagWriteNoiseImage",false);
	    this->itsNoiseImageName = parset.getString("NoiseImageName","");
	    this->itsInputImage="";
	    this->itsSearchType = "spatial";
	    this->itsCube = 0;
	    this->itsFlagRobustStats = true;
	}

	VariableThresholder::VariableThresholder(const VariableThresholder& other)
	{
	    this->operator=(other);
	}

	VariableThresholder& VariableThresholder::operator= (const VariableThresholder& other)
	{
	    if(this==&other) return *this;
	    this->itsFlagRobustStats = other.itsFlagRobustStats;
	    this->itsSNRthreshold = other.itsSNRthreshold;
	    this->itsSearchType = other.itsSearchType;
	    this->itsBoxSize = other.itsBoxSize;
	    this->itsInputImage = other.itsInputImage;
	    this->itsFlagWriteSNRimage = other.itsFlagWriteSNRimage;
	    this->itsSNRimageName = other.itsSNRimageName;
	    this->itsFlagWriteThresholdImage = other.itsFlagWriteThresholdImage;
	    this->itsThresholdImageName = other.itsThresholdImageName;
	    this->itsFlagWriteNoiseImage = other.itsFlagWriteNoiseImage;
	    this->itsNoiseImageName = other.itsNoiseImageName;
	    this->itsCube = other.itsCube;
	    return *this;
	}

	
	void VariableThresholder::fixName(std::string name, bool flag, std::string suffix)
	{
	    if(name == "") {
		// if it has not been specified, construct name from input image
		name = this->itsInputImage;
		// trim .fits off name if present
		if(name.substr(name.size()-5,5)==".fits") name=name.substr(0,name.size()-5);
		name = name + "-" + suffix;
		if(flag) ASKAPLOG_DEBUG_STR(logger, "Actually saving " << suffix  << " map to image \"" << name <<"\"");
	    }
	}

	void VariableThresholder::initialise(duchamp::Cube &cube)
	{
	    
	    this->itsCube = &cube;
	    this->itsInputImage = cube.pars().getImageFile();
	    this->itsFlagRobustStats = cube.pars().getFlagRobustStats();
	    this->itsSNRthreshold = cube.pars().getCut();
	    this->itsSearchType = cube.pars().getSearchType();
	    ASKAPCHECK((this->itsSearchType=="spectral")||(this->itsSearchType=="spatial"),
		       "SearchType needs to be either 'spectral' or 'spatial' - you have " << this->itsSearchType);

	    fixName(this->itsSNRimageName, this->itsFlagWriteSNRimage, "SNR");
	    fixName(this->itsThresholdImageName, this->itsFlagWriteThresholdImage, "Threshold");
	    fixName(this->itsNoiseImageName, this->itsFlagWriteNoiseImage, "Noise");
	    
	}


	void VariableThresholder::threshold()
	{
	    ImageWriter imWriter;
	    imWriter.copyMetadata(this->itsCube);
	    const size_t spatsize=this->itsCube->getDimX() * this->itsCube->getDimY();
	    const size_t specsize=this->itsCube->getDimZ();

	    if(this->itsSearchType == "spatial"){

		int specAxis=imWriter.coordsys().spectralAxisNumber();
		casa::IPosition spatshape = imWriter.shape();
		spatshape(specAxis)=1;
		casa::IPosition box(2, this->itsBoxSize, this->itsBoxSize);
		for(size_t z=0; z<specsize;z++){
		    casa::Array<Float> chanMapInput(spatshape,this->itsCube->getArray()+z*spatsize,casa::SHARE);
		    casa::Array<Float> middle(spatshape,0.);
		    casa::Array<Float> spread(spatshape,0.);
		    slidingBoxStats(chanMapInput, middle, spread, box, this->itsFlagRobustStats);
		    
		    casa::IPosition loc(imWriter.shape().size(),0);
		    loc(specAxis) = z;
		    if(this->itsFlagWriteNoiseImage){
			if(z==0) imWriter.create(this->itsNoiseImageName);
			imWriter.write(spread,loc);
		    }
		    if(this->itsFlagWriteThresholdImage){
			casa::Array<Float> thresh = middle + this->itsSNRthreshold * spread;
			if(z==0) imWriter.create(this->itsThresholdImageName);
			imWriter.write(thresh,loc);
		    }
		    casa::Array<Float> snr = calcSNR(chanMapInput,middle,spread);
		    if(this->itsFlagWriteSNRimage){
			if(z==0) imWriter.create(this->itsSNRimageName);
			imWriter.write(snr,loc);
		    }
		    for(size_t i=0;i<spatsize;i++) this->itsCube->getRecon()[i+z*spatsize] = snr.data()[i];
		}

	    }
	    else{

		int lngAxis=imWriter.coordsys().directionAxesNumbers()[0];
		int latAxis=imWriter.coordsys().directionAxesNumbers()[1];
		casa::IPosition specshape=imWriter.shape();
		specshape(lngAxis) = specshape(latAxis) = 1;
		casa::IPosition box(1, this->itsBoxSize);
		for(size_t i=0;i<spatsize;i++){
		    casa::Array<Float> specInput(specshape,this->itsCube->getArray()+i*specsize,casa::SHARE);
		    casa::Array<Float> middle(specshape,0.);
		    casa::Array<Float> spread(specshape,0.);
		    slidingBoxStats(specInput, middle, spread, box, this->itsFlagRobustStats);

		    casa::IPosition loc(imWriter.shape().size(),0);
		    loc(lngAxis) = i%this->itsCube->getDimX();
		    loc(latAxis) = i/this->itsCube->getDimX();
		    if(this->itsFlagWriteNoiseImage){
			if(i==0) imWriter.create(this->itsNoiseImageName);
			imWriter.write(spread,loc);
		    }
		    if(this->itsFlagWriteThresholdImage){
			casa::Array<Float> thresh = middle + this->itsSNRthreshold * spread;
			if(i==0) imWriter.create(this->itsThresholdImageName);
			imWriter.write(thresh,loc);
		    }
		    casa::Array<Float> snr = calcSNR(specInput,middle,spread);
		    if(this->itsFlagWriteSNRimage){
			if(i==0) imWriter.create(this->itsSNRimageName);
			imWriter.write(snr,loc);
		    }
		    for(size_t z=0;z<specsize;z++) this->itsCube->getRecon()[i+z*spatsize] = snr.data()[z];
		}

	    }

	    this->itsCube->setReconFlag(true);
   
	}


	void VariableThresholder::search()
	{
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
