#include <askap_analysis.h>
#include <preprocessing/VariableThresholder.h>
#include <preprocessing/VariableThresholdingHelpers.h>
#include <outputs/ImageWriter.h>
#include <analysisparallel/SubimageDef.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <Common/ParameterSet.h>
#include <Blob/BlobString.h>
#include <Blob/BlobIBufString.h>
#include <Blob/BlobOBufString.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>
#include <duchamp/Cubes/cubes.hh>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Slicer.h>
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

	VariableThresholder::VariableThresholder(askap::askapparallel::AskapParallel& comms, const LOFAR::ParameterSet &parset):
	    itsComms(&comms),itsParset(parset)
	{
	    /// @details Initialise from a LOFAR parset. Define all
	    /// parameters save for the input image, the search type
	    /// and the robust stats flag - all of which are set
	    /// according to the duchamp::Cube parameters. If an
	    /// output image name is not provided, it will not be
	    /// written.

	    this->itsBoxSize = parset.getInt16("boxSize",50);
	    this->itsSNRimageName = parset.getString("SNRimageName", "");
	    this->itsThresholdImageName = parset.getString("ThresholdImageName","");
	    this->itsNoiseImageName = parset.getString("NoiseImageName","");
	    this->itsAverageImageName = parset.getString("AverageImageName","");
	    this->itsBoxSumImageName = parset.getString("BoxSumImageName","");
	    this->doWriteImages = this->itsSNRimageName!="" || this->itsThresholdImageName!="" || this->itsNoiseImageName!="" || this->itsAverageImageName!="" || this->itsBoxSumImageName!="";
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
	    this->itsComms = other.itsComms;
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

	
	void VariableThresholder::initialise(duchamp::Cube &cube, analysisutilities::SubimageDef &subdef)
	{
	    /// @details Initialise the class with information from
	    /// the duchamp::Cube. This is done to avoid replicating
	    /// parameters and preserving the parameter
	    /// hierarchy. Once the input image is known, the output
	    /// image names can be set with fixName() (if they have
	    /// not been defined via the parset).

	    this->itsCube = &cube;
	    this->itsSubimageDef = subdef;
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
	    this->itsSlicer = analysisutilities::subsectionToSlicer(cube.pars().section());
	    analysisutilities::fixSlicer(this->itsSlicer, cube.header().getWCS());

	    const SubImage<Float> *sub = new SubImage<Float>(*imagePtr, this->itsSlicer);
	    this->itsInputCoordSys = sub->coordinates();
	    this->itsInputShape = sub->shape();

	    duchamp::Section sec = this->itsSubimageDef.section(this->itsComms->rank()-1);
	    sec.parse(this->itsInputShape.asStdVector());
	    duchamp::Section secMaster = this->itsSubimageDef.section(-1);
	    secMaster.parse(this->itsInputShape.asStdVector());
	    this->itsLocation = casa::IPosition(sec.getStartList()) - casa::IPosition(secMaster.getStartList());
	    ASKAPLOG_DEBUG_STR(logger, "Reference location for rank " << this->itsComms->rank() << " is " << this->itsLocation << " since local subsection = " << sec.getSection() << " and input shape = " << this->itsInputShape);
	    ASKAPLOG_DEBUG_STR(logger, "Rank " << this->itsComms->rank() << " has loc " << casa::IPosition(sec.getStartList()) << " while the master has loc " << casa::IPosition(secMaster.getStartList()));
	    

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

	    ASKAPLOG_INFO_STR(logger, "Will calculate box-wise signal-to-noise in image of shape " << this->itsInputShape << " using  '"<<this->itsSearchType<<"' mode with chunks of shape " << chunkshape << " and a box of shape " << box);

	    for(size_t ctr=0;ctr<maxCtr;ctr++){
		if(maxCtr>1) ASKAPLOG_DEBUG_STR(logger, "Iteration " << ctr << " of " << maxCtr);
		bool isStart=(ctr==0);
		casa::Array<Float> inputChunk(chunkshape,0.);
		defineChunk(inputChunk,ctr);
		casa::Array<Float> middle(chunkshape,0.);
		casa::Array<Float> spread(chunkshape,0.);
		casa::Array<Float> snr(chunkshape,0.);
		casa::Array<Float> boxsum(chunkshape,0.);

		casa::IPosition loc(this->itsLocation.size(),0);
		if(this->itsSearchType == "spatial"){
		    if(specAxis>=0) loc(specAxis) = ctr;
		}
		else{
		    if(lngAxis>=0) loc(lngAxis) = ctr%this->itsCube->getDimX();
		    if(latAxis>=0) loc(latAxis) = ctr/this->itsCube->getDimX();
		}
		// loc = loc + this->itsSlicer.start();
		loc = loc + this->itsLocation;
		
		if(this->itsComms->isWorker()){
		    slidingBoxStats(inputChunk, middle, spread, box, this->itsFlagRobustStats);
		    snr = calcSNR(inputChunk,middle,spread);
		    if(this->itsBoxSumImageName!=""){
			boxsum = slidingArrayMath(inputChunk, box, SumFunc<Float>());
		    }
		}

		if(this->doWriteImages) this->writeImages(middle,spread,snr,boxsum,loc,isStart);

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

	void VariableThresholder::writeImages(casa::Array<Float> &middle, casa::Array<Float> &spread, casa::Array<Float> &snr, casa::Array<Float> &boxsum, casa::IPosition &loc, bool doCreate)
	{
	    /// @details Write all 

	    ImageWriter noiseWriter(this->itsCube),averageWriter(this->itsCube),threshWriter(this->itsCube),snrWriter(this->itsCube),boxWriter(this->itsCube);


	    if(!this->itsComms->isParallel() || this->itsComms->isMaster()){
		// If serial mode, or we're on the master node, create the images as needed, but only when requested via doCreate.
		if(doCreate){
		    noiseWriter.create(this->itsNoiseImageName);
		    averageWriter.create(this->itsAverageImageName);
		    threshWriter.create(this->itsThresholdImageName);
		    snrWriter.create(this->itsSNRimageName);
		    boxWriter.create(this->itsBoxSumImageName);
		}
	    }

	    if(this->itsComms->isParallel()){

		bool OK;
		LOFAR::BlobString bs;

		if(this->itsComms->isMaster()){
		    for (int i = 1; i < this->itsComms->nProcs(); i++) {
			// First send the node number
			ASKAPLOG_DEBUG_STR(logger, "MASTER: Sending 'go' to worker#" << i);
			bs.resize(0);
			LOFAR::BlobOBufString bob(bs);
			LOFAR::BlobOStream out(bob);
			out.putStart("goWrite", 1);
			out << i ;
			out.putEnd();
			this->itsComms->sendBlob(bs, i);
			ASKAPLOG_DEBUG_STR(logger, "MASTER: Sent. Now waiting for reply from worker#"<<i);
			// Then wait for the OK from that node
			bs.resize(0);
			ASKAPLOG_DEBUG_STR(logger, "MASTER: Reading from connection "<< i-1);
			this->itsComms->receiveBlob(bs, i);
			LOFAR::BlobIBufString bib(bs);
			LOFAR::BlobIStream in(bib);
			int version = in.getStart("writeDone");
			ASKAPASSERT(version == 1);
			in >> OK;
			in.getEnd();			
			ASKAPLOG_DEBUG_STR(logger, "MASTER: Received. Worker#"<<i<<" done.");
			if (!OK) ASKAPTHROW(AskapError, "Staged writing of image failed.");
		    }

		} else if (this->itsComms->isWorker()) {
		    
		    OK = true;
		    int rank;
		    int version;

		    if (this->itsComms->isParallel()) {
			do {
			    bs.resize(0);
			    this->itsComms->receiveBlob(bs, 0);
			    LOFAR::BlobIBufString bib(bs);
			    LOFAR::BlobIStream in(bib);
			    version = in.getStart("goWrite");
			    ASKAPASSERT(version == 1);
			    in >> rank;
			    in.getEnd();
			    OK = (rank == this->itsComms->rank());
			} while (!OK);
		    }

		    if (OK) {
			ASKAPLOG_INFO_STR(logger,  "Worker #" << this->itsComms->rank() << ": About to write data to image ");

			this->writeArray(noiseWriter,spread,loc);
			this->writeArray(averageWriter,middle,loc);
			this->writeArray(snrWriter,snr,loc);
			this->writeArray(boxWriter,boxsum,loc);
			casa::Array<Float> thresh = middle + this->itsSNRthreshold * spread;
			this->writeArray(threshWriter,spread,loc);

			// Return the OK to the master to say that we've written to the image
			if (this->itsComms->isParallel()) {
			    bs.resize(0);
			    LOFAR::BlobOBufString bob(bs);
			    LOFAR::BlobOStream out(bob);
			    ASKAPLOG_DEBUG_STR(logger, "Worker #" << this->itsComms->rank() << ": Sending done message to Master.");
			    out.putStart("writeDone", 1);
			    out << OK;
			    out.putEnd();
			    this->itsComms->sendBlob(bs, 0);
			    ASKAPLOG_DEBUG_STR(logger, "Worker #" << this->itsComms->rank() << ": All done.");

			}
		    }

		}

	    }
	    else {
		this->writeArray(noiseWriter,spread,loc);
		this->writeArray(averageWriter,middle,loc);
		this->writeArray(snrWriter,snr,loc);
		this->writeArray(boxWriter,boxsum,loc);
		casa::Array<Float> thresh = middle + this->itsSNRthreshold * spread;
		this->writeArray(threshWriter,spread,loc);
		
	    }

	}

	void VariableThresholder::writeArray(ImageWriter &writer, casa::Array<casa::Float> &array, casa::IPosition &loc)
	{
	    if(writer.imagename()!=""){
		ASKAPLOG_DEBUG_STR(logger, "Writing array of shape " << array.shape() << " to " << writer.imagename() << " at location " << loc);
		writer.write(array,loc);
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
