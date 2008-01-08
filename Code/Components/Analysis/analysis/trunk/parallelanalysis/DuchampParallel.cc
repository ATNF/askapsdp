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

#include <conrad_analysis.h>

#include <conrad/ConradLogging.h>
#include <conrad/ConradError.h>

#include <parallelanalysis/DuchampParallel.h>

#include <sstream>

#include <duchamp/duchamp.hh>
#include <duchamp/Cubes/cubes.hh>
#include <duchamp/Utils/Statistics.hh>

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
	itsCube.pars().setVerbosity(false);
	CONRADLOG_INFO_STR(logger, "Defined the cube.");
	//CONRADLOG_DEBUG_STR(logger, "Its Param set is :" << itsCube.pars());

// 	ParameterSet subset(parset.makeSubset("param."));
// 	std::string param; // use this for temporary storage of parameters.
// 	param = subsitute(parset.getString("");
	

      }

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
      }

    }
      


    // Find the lists (on the workers)
    void DuchampParallel::findLists()
    {
      if(isWorker()) {
        CONRADLOG_INFO_STR(logger,  "Finding lists from image " << itsImage);
        // Send the lists to the master here
      }
      else {
      }
      
    }
    
    // Condense the lists (on the master)
    void DuchampParallel::condenseLists() 
    {
      if(isMaster()) {
        // Get the lists from the workers here
        // ....
        // Now process the lists
        CONRADLOG_INFO_STR(logger,  "Condensing lists" );
        // Send the region specifications to the workers
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
          CONRADLOG_INFO_STR(logger, "Received stats from worker "<< rank);

	  size += newsize;
	  av += newav * newsize;
	  rms += newrms * newsize;
        }
	
	if(size>0){
	  av /= double(size);
	  rms /= double(size);
	}

        // Print out
	CONRADLOG_INFO_STR(logger, "Aggregated stats: mean = " << av << ", rms = " << rms);

      }
      else {
      }
    }

  }
}
