/// @file
///
/// @brief Base class for Stream applications
/// @details
/// Supports algorithms by providing methods for initialization
/// of MPI connections, sending and models around.
/// There is assumed to be one master and many workers.
///
/// @copyright (c) 2008 CSIRO
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
