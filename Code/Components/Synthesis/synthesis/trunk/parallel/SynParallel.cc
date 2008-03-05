/// @file
///
/// @brief Base class for parallel applications
/// @details
/// Supports algorithms by providing methods for initialization
/// of MPI connections, sending and models around.
/// There is assumed to be one master and many workers.
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
/// 
#include <mwcommon/MPIConnection.h>
#include <mwcommon/MPIConnectionSet.h>
#include <mwcommon/MWIos.h>

#include <Blob/BlobString.h>
#include <Blob/BlobIBufString.h>
#include <Blob/BlobOBufString.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>

#include <casa/OS/Timer.h>
#include <casa/Utilities/Regex.h>
#include <casa/BasicSL/String.h>
#include <casa/OS/Path.h>

#include <askap_synthesis.h>
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, ".parallel");

#include <askap/AskapError.h>
#include <askap_synthesis.h>

#include <parallel/SynParallel.h>

#include <sstream>

using namespace std;
using namespace askap;
using namespace askap::cp;
using namespace askap::scimath;

namespace askap
{
  namespace synthesis
  {

    SynParallel::SynParallel(int argc, const char** argv) : AskapParallel(argc, argv)
    {
      itsModel = Params::ShPtr(new Params());
      ASKAPCHECK(itsModel, "Model not defined correctly");

    }

    SynParallel::~SynParallel()
    {
    }

    askap::scimath::Params::ShPtr& SynParallel::params()
    {
      return itsModel;
    }

    // Send the model to all workers
    void SynParallel::broadcastModel()
    {
      if (isParallel()&&isMaster())
      {
        ASKAPCHECK(itsModel, "Model not defined prior to broadcast")
        casa::Timer timer;
        timer.mark();

        LOFAR::BlobString bs;
        bs.resize(0);
        LOFAR::BlobOBufString bob(bs);
        LOFAR::BlobOStream out(bob);
        out.putStart("model", 1);
        out << *itsModel;
        out.putEnd();
        itsConnectionSet->writeAll(bs);
        ASKAPLOG_INFO_STR(logger, "Sent model to the workers via MPI in "<< timer.real()
                           << " seconds ");
      }
    }

    // Receive the model from the master
    void SynParallel::receiveModel()
    {
      if (isParallel()&&isWorker())
      {
        ASKAPCHECK(itsModel, "Model not defined prior to receiving")
        casa::Timer timer;
        timer.mark();
        ASKAPLOG_INFO_STR(logger, "Receiving model from the master via MPI");
        LOFAR::BlobString bs;
        bs.resize(0);
        itsConnectionSet->read(0, bs);
        LOFAR::BlobIBufString bib(bs);
        LOFAR::BlobIStream in(bib);
        int version=in.getStart("model");
        ASKAPASSERT(version==1);
        in >> *itsModel;
        in.getEnd();
        ASKAPLOG_INFO_STR(logger, "Received model from the master via MPI in "<< timer.real()
                           << " seconds ");
      }
    }

  }
}
