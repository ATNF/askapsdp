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
      itsCube.pars().setVerbosity(false);
      itsCube.pars().setFlagLog(true);

      string pixelcentre = parset.getString("pixelCentre", "centroid");
      itsCube.pars().setPixelCentre(pixelcentre);
      
      float cutlevel = parset.getFloat("snrCut", 4.);
      itsCube.pars().setCut(cutlevel);

      itsCube.pars().setFlagUserThreshold(true);

      // Now read the correct image name according to worker/master state.
      if(isWorker()) {
        itsImage = substitute(parset.getString("image"));
	itsCube.pars().setImageFile(itsImage);
	CONRADLOG_INFO_STR(logger, "Defined the cube.");
	//CONRADLOG_DEBUG_STR(logger, "Its Param set is :" << itsCube.pars());

	bool flagRobust = parset.getBool("flagRobust",true);
	itsCube.pars().setFlagRobustStats(flagRobust);

// 	ParameterSet subset(parset.makeSubset("param."));
// 	std::string param; // use this for temporary storage of parameters.
// 	param = substitute(parset.getString("");
	

      }
      else{
	itsImage = substitute(parset.getString("masterImage"));
	itsCube.pars().setImageFile(itsImage);
	
	itsCube.pars().setFlagRobustStats(false);

	string sectionInfo = substitute(parset.getString("sectionInfo"));
	itsSectionList = readSectionInfo(sectionInfo);
	if(itsSectionList.size() != (itsNNode-1) )
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
	  CONRADLOG_INFO_STR(logger,  "Read data from image " << itsImage);
	}

      }
      else {
	if(itsCube.getCube()==duchamp::FAILURE){
	  CONRADLOG_ERROR_STR(logger, "Master: Could not read in data from image " << itsImage << ".");
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
	itsCube.CubicSearch();
	int num = itsCube.getNumObj();
        CONRADLOG_INFO_STR(logger,  "Found " << num << " objects in worker " << itsRank);
	
        // Send the lists to the master here
	if(isParallel()) {
	  LOFAR::BlobString bs;
	  bs.resize(0);
	  LOFAR::BlobOBufString bob(bs);
	  LOFAR::BlobOStream out(bob);
	  out.putStart("detW2M",1);
	  out << itsRank << num;
	  for(int i=0;i<itsCube.getNumObj();i++){
	    std::vector<PixelInfo::Voxel> voxlist = itsCube.getObject(i).getPixelSet();
	    out << int(voxlist.size());
	    for(int p=0;p<voxlist.size();p++){
	      int *pix=new int[3];  // should be long but problem with Blobs
	      pix[0] = voxlist[p].getX(); 
	      pix[1] = voxlist[p].getY(); 
	      pix[2] = voxlist[p].getZ();
 	      out << pix[0] << pix[1] << pix[2] 
		  << itsCube.getPixValue(pix[0],pix[1],pix[2]);
	      delete [] pix;
	    }
	  }
	  out.putEnd();
	  itsConnectionSet->write(0,bs);
	  CONRADLOG_INFO_STR(logger, "Sent detection list to the master from worker " << itsRank );
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
        CONRADLOG_INFO_STR(logger,  "Retrieving lists from workers" );

 	LOFAR::BlobString bs;
        int rank, numObj;
	//	std::vector<duchamp::Detection> *workerList;
	int ct=0,numbad=0;
        for (int i=1; i<itsNNode; i++)
        {
          itsConnectionSet->read(i-1, bs);
          LOFAR::BlobIBufString bib(bs);
          LOFAR::BlobIStream in(bib);
          int version=in.getStart("detW2M");
          CONRADASSERT(version==1);
	  in >> rank >> numObj;
	  for(int obj=0;obj<numObj;obj++){
	    duchamp::Detection *object = new duchamp::Detection;
	    int objsize;
	    in >> objsize;
	    for(int p=0;p<objsize;p++){
	      int *pix=new int[3]; // should be long but problem with Blobs
	      float flux;
 	      in >> pix[0] >> pix[1] >> pix[2] >> flux;
	      for(int j=0;j<3;j++){
		pix[j] += itsSectionList[i-1].getStart(j);
	      }
	      object->addPixel(pix[0],pix[1],pix[2]);
	      PixelInfo::Voxel vox(pix[0],pix[1],pix[2],flux);
	      itsVoxelList.push_back(vox);
	      delete [] pix;
	      ct++;
	    }
	    itsCube.addObject(*object);
	    delete object;
	  }
          in.getEnd();
          CONRADLOG_INFO_STR(logger, "Received list from worker "<< rank);
	  CONRADLOG_INFO_STR(logger, "Now have " << itsCube.getNumObj() << " objects");
        }

	// Now process the lists
	itsCube.logDetectionList();
        CONRADLOG_INFO_STR(logger,  "Condensing lists..." );
	if(itsCube.getNumObj()>1) itsCube.ObjectMerger(); 
        CONRADLOG_INFO_STR(logger,  "Condensing lists done" );

	std::ofstream logfile("/Users/whi550/pixels1.dat");
	logfile << "=-=-=-=-=-=-=-\nCube summary\n=-=-=-=-=-=-=-\n";
	logfile << itsCube;
	logfile.close();
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

      if(isMaster()){
	int numVox = itsVoxelList.size();
	int numObj = itsCube.getNumObj();
	std::vector<PixelInfo::Voxel> templist[numObj];

	for(int i=0;i<itsCube.getNumObj();i++){ // for each object
	  
	  std::vector<PixelInfo::Voxel> 
	    objVoxList=itsCube.getObject(i).getPixelSet();

	  // get the fluxes of each voxel
	  for(int v=0;v<objVoxList.size();v++){
	    int ct=0;
	    while(ct<numVox && !objVoxList[v].match(itsVoxelList[ct])){
	      ct++;
	    }
	    if(ct==numVox){ // there has been no match -- problem!
	      CONRADLOG_ERROR(logger, "Found a voxel in the object lists that doesn't appear in the base list: ");// << objVoxList[v]);
	    }
	    else objVoxList[v].setF( itsVoxelList[ct].getF() );
	  }

	  templist[i] = objVoxList;

	}
	std::vector< std::vector<PixelInfo::Voxel> > 
	  bigVoxSet (templist, templist + numObj);

	itsCube.prepareOutputFile();
	if(itsCube.getNumObj()>0){
	  itsCube.calcObjectWCSparams(bigVoxSet);
	  itsCube.setObjectFlags();
	  itsCube.sortDetections();
	}

      }
    }

    void DuchampParallel::printResults()
    {
      /// @details The final list of detected objects is written to
      /// the terminal and to the results file in the standard Duchamp
      /// manner.

      if(isMaster()) {
	CONRADLOG_INFO_STR(logger, "Found " << itsCube.getNumObj() << " sources.");
	
	itsCube.outputDetectionList();
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

      readData();
      findMeans();
      combineMeans();
      broadcastMean();
      findRMSs();
      combineRMSs();
    }


    void DuchampParallel::findMeans()
    {
      /// @details Find the mean or median (according to the
      /// flagRobustStats parameter) of the worker's image/cube, then
      /// send to the master via LOFAR Blobs.

      if(isWorker()) {
	CONRADLOG_INFO_STR(logger, "Finding mean: worker " << itsRank);
	int size = itsCube.getSize();
	double mean;
	if(itsCube.pars().getFlagRobustStats())
	  mean = findMedian(itsCube.getArray(),size);
	else
	  mean = findMean(itsCube.getArray(),size);
	CONRADLOG_INFO_STR(logger, "#" << itsRank << ": Mean = " << mean );
	
	if(isParallel()) {
	  LOFAR::BlobString bs;
	  bs.resize(0);
	  LOFAR::BlobOBufString bob(bs);
	  LOFAR::BlobOStream out(bob);
	  out.putStart("meanW2M",1);
	  out << itsRank << mean << size;
	  out.putEnd();
	  itsConnectionSet->write(0,bs);
	  CONRADLOG_INFO_STR(logger, "Sent mean to the master from worker " << itsRank );
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
	CONRADLOG_INFO_STR(logger, "About to calculate rms on worker " << itsRank);

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
	// use it to calculate the rms for this section
 	int size = itsCube.getSize();
 	float *array = itsCube.getArray();
	double rms = findSpread(itsCube.pars().getFlagRobustStats(),mean,size,array);
	CONRADLOG_INFO_STR(logger, "#" << itsRank << ": rms = " << rms );

	// return it to the master
	if(isParallel()) {
	  LOFAR::BlobString bs2;
	  bs2.resize(0);
	  LOFAR::BlobOBufString bob(bs2);
	  LOFAR::BlobOStream out(bob);
	  out.putStart("rmsW2M",1);
	  out << itsRank << rms << size;
	  out.putEnd();
	  itsConnectionSet->write(0,bs2);
	  CONRADLOG_INFO_STR(logger, "Sent local rms to the master from worker " << itsRank );
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
        CONRADLOG_INFO_STR(logger,  "Receiving Means and combining" );

	LOFAR::BlobString bs1;
        int rank, size=0;
	double av=0;
        for (int i=1; i<itsNNode; i++)
        {
          itsConnectionSet->read(i-1, bs1);
          LOFAR::BlobIBufString bib(bs1);
          LOFAR::BlobIStream in(bib);
          int version=in.getStart("meanW2M");
          CONRADASSERT(version==1);
	  double newav;
	  int newsize;
          in >> rank >> newav >> newsize;
          in.getEnd();
          CONRADLOG_INFO_STR(logger, "Received mean from worker "<< rank);

	  size += newsize;
	  av += newav * newsize;
        }

	if(size>0){
	  av /= double(size);
	}

	CONRADLOG_INFO_STR(logger, "OVERALL SIZE = " << size);
	CONRADLOG_INFO_STR(logger, "OVERALL MEAN = " << av);
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
	CONRADLOG_INFO_STR(logger, 
			   "Sent local rms to the master from worker " << itsRank );
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
        CONRADLOG_INFO_STR(logger,  "Receiving RMS values and combining" );

	LOFAR::BlobString bs;
        int rank, size=0;
	double rms=0;
        for (int i=1; i<itsNNode; i++)
        {
          itsConnectionSet->read(i-1, bs);
          LOFAR::BlobIBufString bib(bs);
          LOFAR::BlobIStream in(bib);
          int version=in.getStart("rmsW2M");
          CONRADASSERT(version==1);
	  double newrms;
	  int newsize;
          in >> rank >> newrms >> newsize;
          in.getEnd();
          CONRADLOG_INFO_STR(logger, "Received RMS from worker "<< rank);

	  size += newsize;
	  rms += (newrms * newrms * (newsize-1));
        }

	if(size>0){
	  rms = sqrt( rms / double(size-1) );
	}
	itsCube.stats().setStddev(rms);

	CONRADLOG_INFO_STR(logger, "OVERALL RMS = " << rms);

      }
    }

    void DuchampParallel::broadcastThreshold() 
    {
      /// @details The master values of the mean and rms are used,
      /// along with the snrCut parameter to generate a detection
      /// threshold. This threshold value is then sent to the workers
      /// via LOFAR Blobs.

      if(isMaster()&&isParallel()) {
	// now send the overall mean to the workers so they can calculate the rms
	double av = itsCube.stats().getMean();
	double rms = itsCube.stats().getStddev();
	double threshold = av + rms * itsCube.pars().getCut();
	itsCube.stats().setThreshold(threshold);
	LOFAR::BlobString bs;
	bs.resize(0);
	LOFAR::BlobOBufString bob(bs);
	LOFAR::BlobOStream out(bob);
	out.putStart("threshM2W",1);
	out << threshold;
	out.putEnd();
	itsConnectionSet->writeAll(bs);
	CONRADLOG_INFO_STR(logger, "Sent threshold (" << threshold << ") from the master" );
	itsCube.pars().setThreshold(threshold);
      }
      else {
      }
    }


    void DuchampParallel::receiveThreshold()
    {
      /// @details The workers read the detection threshold sent via LOFAR Blobs from the master.
      
     if(isWorker()) {
	CONRADLOG_INFO_STR(logger, "Setting threshold on worker " << itsRank);

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
	itsCube.pars().setThreshold(threshold);
     }
    }


  }
}
