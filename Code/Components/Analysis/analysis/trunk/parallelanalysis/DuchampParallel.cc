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

#include <sstream>
#include <algorithm>

#include <duchamp/duchamp.hh>
#include <duchamp/param.hh>
#include <duchamp/Cubes/cubes.hh>
#include <duchamp/Utils/Statistics.hh>
#include <duchamp/Utils/utils.hh>
#include <duchamp/Detection/detection.hh>
#include <duchamp/PixelMap/Voxel.hh>

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
      if(isWorker()) {
        itsImage = substitute(parset.getString("image"));

	itsCube.pars().setImageFile(itsImage);
	CONRADLOG_INFO_STR(logger, "Defined the cube.");
	//CONRADLOG_DEBUG_STR(logger, "Its Param set is :" << itsCube.pars());


// 	ParameterSet subset(parset.makeSubset("param."));
// 	std::string param; // use this for temporary storage of parameters.
// 	param = substitute(parset.getString("");
	

      }
      else{
	itsImage = substitute(parset.getString("masterImage"));
	itsCube.pars().setImageFile(itsImage);
      }

      itsCube.pars().setVerbosity(false);

      string flagRobust = substitute(parset.getString("flagRobust"));
      itsCube.pars().setFlagRobustStats(flagRobust=="true" || flagRobust=="1");
      
      float cutlevel = parset.getFloat("snrCut");
      itsCube.pars().setCut(cutlevel);

    }

    // Read in the data from the image file (on the workers)
    void DuchampParallel::readData()
    {
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

      }

    }
      


    // Find the lists (on the workers)
    void DuchampParallel::findLists()
    {
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
	  //	  out << itsRank << itsCube.getNumObj() << int(itsCube.pObjectList());
	  out << itsRank << num;
	  for(int i=0;i<itsCube.getNumObj();i++){
	    int size = itsCube.getObject(i).getSize();
	    out << size;
	    for(int p=0;p<size;p++){
	      PixelInfo::Voxel vox = itsCube.getObject(i).getPixel(p);
	      int x = vox.getX(), y=vox.getY(), z=vox.getZ();
	      out << x << y << z;
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
      if(isMaster()) {
        // Get the lists from the workers here
        CONRADLOG_INFO_STR(logger,  "Retrieving lists from workers" );

 	LOFAR::BlobString bs;
        int rank, numObj;
	//	std::vector<duchamp::Detection> *workerList;
        for (int i=1; i<itsNNode; i++)
        {
          itsConnectionSet->read(i-1, bs);
          LOFAR::BlobIBufString bib(bs);
          LOFAR::BlobIStream in(bib);
          int version=in.getStart("detW2M");
          CONRADASSERT(version==1);
	  //          in >> rank >> numObj >> workerList;
	  in >> rank >> numObj;
	  for(int obj=0;obj<numObj;obj++){
	    duchamp::Detection object;
	    int objsize,x,y,z;
	    in >> objsize;
	    for(int p=0;p<objsize;p++){
	      in >> x >> y >> z;
	      object.addPixel(x,y,z);
	    }
	    itsCube.addObject(object);
	  }
          in.getEnd();
          CONRADLOG_INFO_STR(logger, "Received list from worker "<< rank);
	  CONRADLOG_INFO_STR(logger, "Now have " << itsCube.getNumObj() << " objects");
	  //	  itsCube.addObjectList(*workerList);
        }

	//	std::cout << itsCube;
// 	std::ofstream logfile("duchampLog.txt");
// 	logfile << "=-=-=-=-=-=-=-\nCube summary\n=-=-=-=-=-=-=-\n";
// 	logfile << itsCube;
// 	logfile.close();

	// Now process the lists
        CONRADLOG_INFO_STR(logger,  "Condensing lists..." );
	if(itsCube.getNumObj()>1) itsCube.ObjectMerger(); 
        CONRADLOG_INFO_STR(logger,  "Condensing lists done" );

       }
      else {
      }
    }

    void DuchampParallel::printResults()
    {
      if(isMaster()) {
	CONRADLOG_INFO_STR(logger, "Found " << itsCube.getNumObj() << " sources.");
	
	itsCube.prepareOutputFile();
	if(itsCube.getNumObj()>0){
 	  itsCube.calcObjectWCSparams();
	  itsCube.setObjectFlags();
	  itsCube.sortDetections();
	}
	itsCube.outputDetectionList();
	//itsCube.logDetectionList();
      }
      else{
      }
    }
    
    void DuchampParallel::findMeans()
    {
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
    
    double findSpread(bool robust, double middle, int size, float *array)
    {
      double spread=0;
      if(robust){
	float *arrayCopy = new float[size];
	for(int i=0;i<size;i++) arrayCopy[i] = fabs(array[i]-middle);
	std::sort(arrayCopy,arrayCopy+size);
	if((size%2)==0) spread = (arrayCopy[size/2-1]+arrayCopy[size/2])/2;
	else spread = arrayCopy[size/2];
	delete [] arrayCopy;
	spread = Statistics::madfmToSigma(spread);
      }
      else{
	for(int i=0;i<size;i++) spread += (array[i]-middle)*(array[i]-middle);
	spread = sqrt(spread / double(size-1));
      }
      return spread;
    }


    void DuchampParallel::findRMSs() 
    {
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
// 	double rms = 0.;
 	int size = itsCube.getSize();
 	float *array = itsCube.getArray();
// 	for(int i=0;i<size;i++) rms += (array[i]-mean)*(array[i]-mean);
// 	rms = sqrt(rms / double(size-1));
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

    // Find the statistics (on the workers)
    void DuchampParallel::findStatistics()
    {
      if(isWorker()) {
        // Get the region specifications from the master
        CONRADLOG_INFO_STR(logger,  "Finding Statistics" );
        // Find the statistics for the local cube
	itsCube.setCubeStats();
	CONRADLOG_INFO_STR(logger, "Mean = " << itsCube.stats().getMean() << ", RMS = " << itsCube.stats().getStddev() );
	CONRADLOG_INFO_STR(logger, "Median = " << itsCube.stats().getMedian() << ", MADFM = " << itsCube.stats().getMadfm() );
	CONRADLOG_INFO_STR(logger, "Noise level = " << itsCube.stats().getMiddle() << ", Noise spread = " << itsCube.stats().getSpread() );

	if(isParallel()) {
	  // Send back the statistics to the master	
	  // copying the structure from MEParallel.cc
	  LOFAR::BlobString bs;
	  bs.resize(0);
	  LOFAR::BlobOBufString bob(bs);
	  LOFAR::BlobOStream out(bob);
	  out.putStart("stat",1);
	  double mean = itsCube.stats().getMean(), rms = itsCube.stats().getStddev();
	  int size = itsCube.getSize();
	  out << itsRank << mean << rms << size;
	  out.putEnd();
	  itsConnectionSet->write(0,bs);
	  CONRADLOG_INFO_STR(logger, "Sent stats to the master via MPI from worker " << itsRank );
	}

      }
      else {
      }
    }

    void DuchampParallel::combineMeans()
    {
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

	CONRADLOG_INFO_STR(logger, "OVERALL RMS = " << rms);

      }
    }

    // Print the statistics (on the master)
    void DuchampParallel::printStatistics()
    {
      if(isMaster()&&isParallel()) {
        // Get the statistics from the workers
        CONRADLOG_INFO_STR(logger,  "Receiving Statistics" );
 
	// copying the structure from MEParallel.cc
	LOFAR::BlobString bs;
        int rank, size=0;
	double av=0,rms=0;
        for (int i=1; i<itsNNode; i++)
        {
          itsConnectionSet->read(i-1, bs);
          LOFAR::BlobIBufString bib(bs);
          LOFAR::BlobIStream in(bib);
          int version=in.getStart("stat");
          CONRADASSERT(version==1);
	  double newav, newrms;
	  int newsize;
          in >> rank >> newav >> newrms >> newsize;
          in.getEnd();
          CONRADLOG_INFO_STR(logger, i-1 << ": Received stats from worker "<< rank);

	  size += newsize;
	  av += newav * newsize;
	  rms += (newrms * newrms * newsize);
        }
	
	if(size>0){
	  av /= double(size);
	  rms = sqrt( rms / double(size) );
	}

        // Print out
	CONRADLOG_INFO_STR(logger, "Aggregated stats: mean = " << av << ", rms = " << rms);

      }
      else {
      }
    }

  }
}
