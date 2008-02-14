/// @file
///
/// @brief Base class for parallel applications
/// @details
/// Supports algorithms by providing methods for initialization
/// of MPI connections, sending and models around.
/// There is assumed to be one master and many workers.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
/// 

#include <Common/LofarTypedefs.h>
using namespace LOFAR::TYPES;
#include <Blob/BlobString.h>
#include <Blob/BlobIBufString.h>
#include <Blob/BlobOBufString.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>
#include <APS/Exceptions.h>

#include <casa/OS/Timer.h>
#include <casa/Utilities/Regex.h>
#include <casa/BasicSL/String.h>
#include <casa/OS/Path.h>

// boost includes
#include <boost/shared_ptr.hpp>

#include <conrad_analysis.h>

#include <conrad/ConradLogging.h>
#include <conrad/ConradError.h>

#include <conradparallel/ConradParallel.h>

#include <parallelanalysis/DuchampParallel.h>
#include <analysisutilities/AnalysisUtilities.h>

#include <sstream>
#include <algorithm>

#include <duchamp/duchamp.hh>
#include <duchamp/param.hh>
#include <duchamp/Cubes/cubes.hh>
#include <duchamp/Utils/Statistics.hh>
#include <duchamp/Utils/utils.hh>
#include <duchamp/Detection/detection.hh>
#include <duchamp/PixelMap/Voxel.hh>
#include <duchamp/PixelMap/Object3D.hh>

CONRAD_LOGGER(logger, ".parallelanalysis");

using namespace std;
using namespace conrad;
using namespace conrad::cp;

using namespace duchamp;

namespace conrad
{
  namespace analysis
  {

    DuchampParallel::DuchampParallel(int argc, const char** argv,
        const LOFAR::ACC::APS::ParameterSet& parset)
    : ConradParallel(argc, argv)
    {
      /// @details The constructor reads parameters from the parameter
      /// set parset. This set can include Duchamp parameters, as well
      /// as particular cduchamp parameters such as masterImage and
      /// sectionInfo.


      // First do the setup needed for both workers and master

      itsCube.pars() = parseParset(parset);

      itsCube.pars().setVerbosity(false);
      itsCube.pars().setFlagLog(true);

      // Now read the correct image name according to worker/master state.
      if(isWorker()) {
        itsImage = substitute(parset.getString("image"));
	itsCube.pars().setImageFile(itsImage);
	CONRADLOG_INFO_STR(logger, "Defined the cube.");
	//CONRADLOG_DEBUG_STR(logger, "Its Param set is :" << itsCube.pars());

	itsCube.pars().setFlagRobustStats( parset.getBool("flagRobust",true) );

	itsCube.pars().setLogFile( substitute(parset.getString("logFile", "duchamp-Logfile-%w.txt")) );

	// TODO:
	// 	ParameterSet subset(parset.makeSubset("param."));
	// 	std::string param; // use this for temporary storage of parameters.
	// 	param = substitute(parset.getString("");
	

      }
      else{
	itsImage = substitute(parset.getString("masterImage"));
	itsCube.pars().setImageFile(itsImage);
	
	itsCube.pars().setFlagRobustStats(false);

	itsCube.pars().setLogFile( substitute(parset.getString("logFile", "duchamp-Logfile.txt")) );

	/// The sectionInfo, read by the master, is interpreted by the
	/// function readSectionInfo(). See its description for a
	/// description of the format of the sectionInfo file, and how
	/// the separated data is interpreted.
	string sectionInfo = substitute(parset.getString("sectionInfo"));
	itsSectionList = readSectionInfo(sectionInfo);
	CONRADLOG_INFO_STR(logger, "Read in the sectionInfo.");
	if(itsSectionList.size() == 0){
	  CONRADLOG_ERROR_STR(logger, "No SectionInfo file found. Exiting.");
	  exit(0);
	}
	else if(int(itsSectionList.size()) != (itsNNode-1) )
	  CONRADLOG_ERROR_STR(logger, "Number of sections provided by " 
			      << sectionInfo 
			      << " does not match the number of images being processed.");

	if(itsCube.pars().getFlagLog()){
	  CONRADLOG_INFO_STR(logger, "Setting up logfile " << itsCube.pars().getLogFile() );
	  std::ofstream logfile(itsCube.pars().getLogFile().c_str());
	  logfile << "New run of the Duchamp sourcefinder: ";
	  time_t now = time(NULL);
	  logfile << asctime( localtime(&now) );
	  // Write out the command-line statement
	  logfile << "Executing statement : ";
	  for(int i=0;i<argc;i++) logfile << argv[i] << " ";
	  logfile << std::endl;
	  logfile << itsCube.pars();
	  logfile.close();
	} 
      }

    }

    // Read in the data from the image file (on the workers)
    void DuchampParallel::readData()
    {
      /// @details Reads in the data using duchamp functionality and
      /// the image name defined in the constructor.

      if(isWorker()) {

	if(itsCube.getCube()==duchamp::FAILURE){
	  CONRADLOG_ERROR_STR(logger, "Could not read in data from image " << itsImage);
	}
	else {
	  CONRADLOG_INFO_STR(logger,  "#"<<itsRank<<": Read data from image " << itsImage);
	  std::stringstream ss;
	  ss << itsCube.getDimX() << " " << itsCube.getDimY() << " " << itsCube.getDimZ();
	  CONRADLOG_INFO_STR(logger, "#"<<itsRank<<": Dimensions are " << ss.str() );
	}

      }
      else {
	if(itsCube.getCube()==duchamp::FAILURE){
	  CONRADLOG_ERROR_STR(logger, "MASTER: Could not read in data from image " << itsImage << ".");
	}
// 	itsCube.header().defineWCS(itsCube.pars().getImageFile(), itsCube.pars());
// 	itsCube.header().readHeaderInfo(itsCube.pars().getImageFile(), itsCube.pars());

      }

    }
      


    // Find the lists (on the workers)
    void DuchampParallel::findLists()
    {
      ///@details Searches the image/cube for objects, using the
      ///duchamp::Cube::CubicSearch function. The detected objects are
      ///then sent to the master using LOFAR Blobs. This is done
      ///pixel-by-pixel, sending the pixel flux as well.
      ///
      /// This is only done on the workers.

      if(isWorker()) {
        CONRADLOG_INFO_STR(logger,  "Finding lists from image " << itsImage);

	itsCube.setCubeStats();
	if(itsCube.pars().getFlagATrous()){
	  CONRADLOG_INFO_STR(logger,  "Searching with reconstruction first");
	  itsCube.ReconSearch();
	}
	else if(itsCube.pars().getFlagSmooth()) {
	  CONRADLOG_INFO_STR(logger,  "Searching with smoothing first");	  
	  itsCube.SmoothSearch();
	}
	else {
	  CONRADLOG_INFO_STR(logger,  "Searching, no smoothing or reconstruction done.");
	  itsCube.CubicSearch();
	}

	int16 num = itsCube.getNumObj(), rank=this->itsRank;
        CONRADLOG_INFO_STR(logger,  "Found " << num << " objects in worker " << this->itsRank);
	
        // Send the lists to the master here
	if(isParallel()) {
	  LOFAR::BlobString bs;
	  bs.resize(0);
	  LOFAR::BlobOBufString bob(bs);
	  LOFAR::BlobOStream out(bob);
	  out.putStart("detW2M",1);
	  out << rank << num;
	  for(int i=0;i<itsCube.getNumObj();i++){
	    std::vector<PixelInfo::Voxel> voxlist = itsCube.getObject(i).getPixelSet();
	    std::vector<PixelInfo::Voxel>::iterator vox;
	    int32 size = voxlist.size();
	    out << size;
	    for(vox=voxlist.begin();vox<voxlist.end();vox++){
	      int32 x,y,z;  // should be long but problem with Blobs
	      x = vox->getX(); 
	      y = vox->getY(); 
	      z = vox->getZ();
 	      out << x << y << z << itsCube.getPixValue(x,y,z);
	    }
	  }
	  out.putEnd();
	  itsConnectionSet->write(0,bs);
	  //	  itsConnectionSet->write(this->itsRank,bs);
	  CONRADLOG_INFO_STR(logger, "Sent detection list to the master from worker " << this->itsRank );
	}
	
      }
      else {
      }
      
    }
    
    // Condense the lists (on the master)
    void DuchampParallel::condenseLists() 
    {
      /// @details Done only on the master. Receive the list of
      /// detected pixels from the workers and store the objects in
      /// itsCube's objectList. Also store the detected voxels in
      /// itsVoxelList so that the fluxes are available for later use.
      ///
      /// Once all the objects are read, merge the object list to
      /// combine adjacent/overlapping objects and remove unacceptable
      /// ones.

      if(isMaster()) {
        // Get the lists from the workers here
        CONRADLOG_INFO_STR(logger,  "MASTER: Retrieving lists from workers" );

	if(isParallel()) {
	  LOFAR::BlobString bs;
	  int16 rank, numObj;
	  int ct=0;
	  for (int i=1; i<itsNNode; i++)
	    {
	      itsConnectionSet->read(i-1, bs);
	      LOFAR::BlobIBufString bib(bs);
	      LOFAR::BlobIStream in(bib);
	      int version=in.getStart("detW2M");
	      CONRADASSERT(version==1);
	      in >> rank >> numObj;
	      CONRADLOG_INFO_STR(logger, "MASTER: Starting to read " 
				 << numObj << " objects from worker #"<< rank);
	      for(int obj=0;obj<numObj;obj++){
		duchamp::Detection *object = new duchamp::Detection;
		int32 objsize;
		in >> objsize;
		for(int p=0;p<objsize;p++){
		  int32 x,y,z; 
		  float flux;
		  in >> x >> y >> z >> flux;
		  x += itsSectionList[i-1].getStart(0);
		  y += itsSectionList[i-1].getStart(1);
		  z += itsSectionList[i-1].getStart(2);
		  object->addPixel(x,y,z);
		  PixelInfo::Voxel vox(x,y,z,flux);
		  itsVoxelList.push_back(vox);
		  ct++;
		}
		itsCube.addObject(*object);
		delete object;
	      }
	      in.getEnd();

	      CONRADLOG_INFO_STR(logger, "MASTER: Received list of size " 
				 << numObj << " from worker #"<< rank);
	      CONRADLOG_INFO_STR(logger, "MASTER: Now have " 
				 << itsCube.getNumObj() << " objects");
	    }
	}
	// Now process the lists
        CONRADLOG_INFO_STR(logger,  "MASTER: Condensing lists..." );
	if(itsCube.getNumObj()>1) itsCube.ObjectMerger(); 
        CONRADLOG_INFO_STR(logger,  "MASTER: Condensing lists done" );

     }
      else {
      }
    }

    void DuchampParallel::calcFluxes()
    {
      /// @details Done on the master. Calculate the fluxes for each
      /// of the objects by constructing voxelLists. The WCS
      /// parameters for each object are also calculated here, so the
      /// WCS header information stored by the master must be
      /// correct. The objects are then ordered by velocity. 
      ///
      /// In the parallel case, we do not set any object flags, as we
      /// don't have all the flux information. This may be possible if
      /// we pass all surrounding pixels as well as detected pixels,
      /// but at the moment, no flags are set. In the serial case,
      /// they are.

      if(isMaster()){

	if(isParallel()) {

	  int numVox = itsVoxelList.size();
	  int numObj = itsCube.getNumObj();
	  std::vector<PixelInfo::Voxel> templist[numObj];

	  CONRADLOG_INFO_STR(logger, "MASTER: Calculating fluxes of " 
			     << numObj << " detected objects.");

	  for(int i=0;i<itsCube.getNumObj();i++){ // for each object
	  
	    std::vector<PixelInfo::Voxel> 
	      objVoxList=itsCube.getObject(i).getPixelSet();
	    std::vector<PixelInfo::Voxel>::iterator vox;
	    // get the fluxes of each voxel
	    for(vox=objVoxList.begin();vox<objVoxList.end();vox++){
	      int ct=0;
	      while(ct<numVox && !vox->match(itsVoxelList[ct])){
		ct++;
	      }
	      if(numVox!=0 && ct==numVox){ // there has been no match -- problem!
		CONRADLOG_ERROR(logger, "MASTER: Found a voxel in the object lists that doesn't appear in the base list.");
	      }
	      else vox->setF( itsVoxelList[ct].getF() );
	    }

	    templist[i] = objVoxList;

	  }
	  std::vector< std::vector<PixelInfo::Voxel> > 
	    bigVoxSet (templist, templist + numObj);

	  itsCube.prepareOutputFile();
	  if(itsCube.getNumObj()>0){
	    // no flag-setting, as it's hard to do when we don't have
	    // all the pixels. Particularly the negative flux flags
	    itsCube.calcObjectWCSparams(bigVoxSet);
	    itsCube.sortDetections();
	  }

	}
	else {
	  
	  itsCube.prepareOutputFile();
	  if(itsCube.getNumObj()>0){
	    itsCube.calcObjectWCSparams();
	    itsCube.setObjectFlags();
	    itsCube.sortDetections();
	  }
	}
      }      
    }

    void DuchampParallel::printResults()
    {
      /// @details The final list of detected objects is written to
      /// the terminal and to the results file in the standard Duchamp
      /// manner.

      if(isMaster()) {
	CONRADLOG_INFO_STR(logger, "MASTER: Found " << itsCube.getNumObj() << " sources.");
	
	itsCube.outputDetectionList();

	if(itsCube.pars().getFlagKarma()){
	  std::ofstream karmafile(itsCube.pars().getKarmaFile().c_str());
	  itsCube.outputDetectionsKarma(karmafile);
	  karmafile.close();
	}

      }
      else{
      }
    }
    
    
    void DuchampParallel::gatherStats()
    {
      /// @details A front-end function that calls all the statistics
      /// functions. Net effect is to find the mean/median and
      /// rms/MADFM for the entire dataset and store these values in
      /// the master's itsCube statsContainer.

      if(!itsCube.pars().getFlagUserThreshold()){
	findMeans();
	combineMeans();
	broadcastMean();
	findRMSs();
	combineRMSs();
      }
      else itsCube.stats().setThreshold( itsCube.pars().getThreshold() );
    }


    void DuchampParallel::findMeans()
    {
      /// @details Find the mean or median (according to the
      /// flagRobustStats parameter) of the worker's image/cube, then
      /// send to the master via LOFAR Blobs.

      if(isWorker()) {
	CONRADLOG_INFO_STR(logger, "Finding mean: worker " << this->itsRank);

	if(itsCube.pars().getFlagATrous()) itsCube.ReconCube();
	else if(itsCube.pars().getFlagSmooth()) itsCube.SmoothCube();

	int32 size = itsCube.getSize();
	double mean;
	float *array;
	if(itsCube.pars().getFlagATrous())       array = itsCube.getArray();
	else if (itsCube.pars().getFlagSmooth()) array = itsCube.getRecon();
	else                                     array = itsCube.getArray();

	if(itsCube.pars().getFlagRobustStats()) mean = findMedian(array,size);
	else                                    mean = findMean(array,size);

	CONRADLOG_INFO_STR(logger, "#" << this->itsRank << ": Mean = " << mean );
	
	if(isParallel()) {
	  LOFAR::BlobString bs;
	  bs.resize(0);
	  LOFAR::BlobOBufString bob(bs);
	  LOFAR::BlobOStream out(bob);
	  out.putStart("meanW2M",1);
	  int16 rank = this->itsRank;
	  out << rank << mean << size;
	  out.putEnd();
	  itsConnectionSet->write(0,bs);
	  //	  itsConnectionSet->write(this->itsRank,bs);
	  CONRADLOG_INFO_STR(logger, "Sent mean to the master from worker " << this->itsRank );
	}
	else {
	  // serial case
	  if(itsCube.pars().getFlagRobustStats()) itsCube.stats().setMedian(mean);
	  else itsCube.stats().setMean(mean);
	  itsCube.stats().setRobust(itsCube.pars().getFlagRobustStats());
	}

      }
      else {
      }
    }

    void DuchampParallel::findRMSs() 
    {
      /// @details Find the rms or the median absolute deviation from
      /// the median (MADFM) (dictated by the flagRobustStats
      /// parameter) of the worker's image/cube, then send to the
      /// master via LOFAR Blobs. To calculate the rms/MADFM, the mean
      /// of the full dataset must be read from the master (again
      /// passed via LOFAR Blobs). The calculation uses the
      /// findSpread() function.

      if(isWorker()) {
	CONRADLOG_INFO_STR(logger, "About to calculate rms on worker " << this->itsRank);

	// first read in the overall mean for the cube
	double mean=0;
	if(isParallel()) {
	  LOFAR::BlobString bs1;
	  itsConnectionSet->read(0, bs1);
	  LOFAR::BlobIBufString bib(bs1);
	  LOFAR::BlobIStream in(bib);
	  int version=in.getStart("meanM2W");
	  CONRADASSERT(version==1);
	  in >> mean;
	  in.getEnd();
	}
	else{
	  mean = itsCube.stats().getMiddle();
	}
	// use it to calculate the rms for this section
 	int32 size = itsCube.getSize();
	float *array;
	if(itsCube.pars().getFlagATrous()){
	  array = new float[size];
	  for(int i=0;i<size;i++) array[i] = itsCube.getPixValue(i) - itsCube.getReconValue(i);
	}
	else if (itsCube.pars().getFlagSmooth()) array = itsCube.getRecon();
	else array = itsCube.getArray();
	double rms = findSpread(itsCube.pars().getFlagRobustStats(),mean,size,array);
	CONRADLOG_INFO_STR(logger, "#" << this->itsRank << ": rms = " << rms );

	// return it to the master
	if(isParallel()) {
	  LOFAR::BlobString bs2;
	  bs2.resize(0);
	  LOFAR::BlobOBufString bob(bs2);
	  LOFAR::BlobOStream out(bob);
	  out.putStart("rmsW2M",1);
	  int16 rank = this->itsRank;
	  out << rank << rms << size;
	  out.putEnd();
	  itsConnectionSet->write(0,bs2);
	  //	  itsConnectionSet->write(this->itsRank,bs2);
	  CONRADLOG_INFO_STR(logger, "Sent local rms to the master from worker " << this->itsRank );
	}
	else{
	  //serial case
	  if(itsCube.pars().getFlagRobustStats()) itsCube.stats().setMadfm(rms);
	  else itsCube.stats().setStddev(rms);
	}

      }
      else {
      }

    }

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

      if(isMaster()&&isParallel()) {
	// get the means from the workers
        CONRADLOG_INFO_STR(logger,  "MASTER: Receiving Means and combining" );

	LOFAR::BlobString bs1;
        int size=0;
	double av=0;
        for (int i=1; i<itsNNode; i++)
        {
          itsConnectionSet->read(i-1, bs1);
          LOFAR::BlobIBufString bib(bs1);
          LOFAR::BlobIStream in(bib);
          int version=in.getStart("meanW2M");
          CONRADASSERT(version==1);
	  double newav;
	  int32 newsize;
	  int16 rank;
          in >> rank >> newav >> newsize;
          in.getEnd();
          CONRADLOG_INFO_STR(logger, "MASTER: Received mean from worker "<< rank);

	  size += newsize;
	  av += newav * newsize;
        }

	if(size>0){
	  av /= double(size);
	}

	CONRADLOG_INFO_STR(logger, "MASTER: OVERALL SIZE = " << size);
	CONRADLOG_INFO_STR(logger, "MASTER: OVERALL MEAN = " << av);
	itsCube.stats().setMean(av);
      }
      else {
      }
    }
	
    void DuchampParallel::broadcastMean() 
    {
      /// @details The mean/median value of the full dataset is sent
      /// via LOFAR Blobs to the workers.

      if(isMaster()&&isParallel()) {
	// now send the overall mean to the workers so they can calculate the rms
	double av = itsCube.stats().getMean();
	LOFAR::BlobString bs2;
	bs2.resize(0);
	LOFAR::BlobOBufString bob(bs2);
	LOFAR::BlobOStream out(bob);
	out.putStart("meanM2W",1);
	out << av;
	out.putEnd();
	itsConnectionSet->writeAll(bs2);
	CONRADLOG_INFO_STR(logger, "MASTER: Broadcast overal mean from master to workers." );
      }
      else {
      }
    }

    void DuchampParallel::combineRMSs()
    {
      /// @details The master reads the rms/MADFM values from each of
      /// the workers, and combines them to produce an estimate of the
      /// rms for the full cube. Again, if MADFM values have been
      /// calculated on the workers, they are treated as estimates of
      /// the rms and are combined as if they are rms values. The
      /// overall value is stored in the StatsContainer in itsCube.

      if(isMaster()&&isParallel()) {
	// get the means from the workers
        CONRADLOG_INFO_STR(logger,  "MASTER: Receiving RMS values and combining" );

	LOFAR::BlobString bs;
        int size=0;
	double rms=0;
        for (int i=1; i<itsNNode; i++)
        {
          itsConnectionSet->read(i-1, bs);
          LOFAR::BlobIBufString bib(bs);
          LOFAR::BlobIStream in(bib);
          int version=in.getStart("rmsW2M");
          CONRADASSERT(version==1);
	  double newrms;
	  int32 newsize;
	  int16 rank;
          in >> rank >> newrms >> newsize;
          in.getEnd();
          CONRADLOG_INFO_STR(logger, "MASTER: Received RMS from worker "<< rank);

	  size += newsize;
	  rms += (newrms * newrms * (newsize-1));
        }

	if(size>0){
	  rms = sqrt( rms / double(size-1) );
	}
	itsCube.stats().setStddev(rms);

	double av = itsCube.stats().getMean();
	double threshold = av + rms * itsCube.pars().getCut();
	itsCube.stats().setThreshold(threshold);
	itsCube.pars().setFlagUserThreshold(true);
	itsCube.pars().setThreshold(threshold);

	CONRADLOG_INFO_STR(logger, "MASTER: OVERALL RMS = " << rms);

      }
    }

    void DuchampParallel::broadcastThreshold() 
    {
      /// @details The detection threshold value (which has been
      /// already calculated) is sent to the workers via LOFAR Blobs.

      if(isMaster()&&isParallel()) {
	// now send the overall mean to the workers so they can calculate the rms
	LOFAR::BlobString bs;
	bs.resize(0);
	LOFAR::BlobOBufString bob(bs);
	LOFAR::BlobOStream out(bob);
	out.putStart("threshM2W",1);
	double threshold = itsCube.stats().getThreshold();
	out << threshold;
	out.putEnd();
	itsConnectionSet->writeAll(bs);
	CONRADLOG_INFO_STR(logger, "MASTER: Sent threshold (" 
			   << itsCube.stats().getThreshold() << ") from the master" );
      }
      else {
      }
    }


    void DuchampParallel::receiveThreshold()
    {
      /// @details The workers read the detection threshold sent via LOFAR Blobs from the master.
      
     if(isWorker()) {

	double threshold;
	if(isParallel()) {
	  LOFAR::BlobString bs;
	  itsConnectionSet->read(0, bs);
	  LOFAR::BlobIBufString bib(bs);
	  LOFAR::BlobIStream in(bib);
	  int version=in.getStart("threshM2W");
	  CONRADASSERT(version==1);
	  in >> threshold;
	  in.getEnd();
	}
	else{
	  threshold = itsCube.stats().getMiddle() + itsCube.stats().getSpread()*itsCube.pars().getCut();
	}

	CONRADLOG_INFO_STR(logger, "Setting threshold on worker " << itsRank << " to be " << threshold);

	itsCube.pars().setThreshold(threshold);
	itsCube.pars().setFlagUserThreshold(true);
     }
    }


  }
}
