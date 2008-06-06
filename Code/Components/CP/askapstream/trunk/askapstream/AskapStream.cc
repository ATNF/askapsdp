/// @file
///
/// @brief Base class for Stream applications
/// @details
/// Supports algorithms by providing methods for initialization
/// of MPI connections, sending and models around.
/// There is assumed to be one master and many workers.
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
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

#include <askap_askapstream.h>

#include <askap/AskapLogging.h>

#include <askap/AskapError.h>

#include <askapstream/AskapStream.h>

#include <sstream>

using namespace std;
using namespace askap;
using namespace askap::cp;

namespace askap
{
  namespace cp
  {

    ASKAP_LOGGER(logger, ".askapstream");

    AskapStream::AskapStream(int argc, const char** argv)
    {

      // Now we have to initialize the logger before we use it
      ASKAPLOG_INIT("askap.log_cfg");

      ASKAPLOG_INFO_STR(logger, ASKAP_PACKAGE_VERSION);

    }

    AskapStream::~AskapStream()
    {
    }

  }
}
