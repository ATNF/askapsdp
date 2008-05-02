/// @file
///
/// Provides general utility functions to support moving objects around with LOFAR Blobs
///
/// (c) 2007 ASKAP, All Rights Reserved.
/// @author Matthew Whiting <matthew.whiting@csiro.au>
///
#ifndef ASKAP_ANALYSIS_BLOBRELATED_H_
#define ASKAP_ANALYSIS_BLOBRELATED_H_

#include <string>
#include <vector>

#include <sourcefitting/RadioSource.h>

#include <duchamp/Detection/detection.hh>

#include <Common/LofarTypedefs.h>
using namespace LOFAR::TYPES;
#include <Blob/BlobString.h>
#include <Blob/BlobIBufString.h>
#include <Blob/BlobOBufString.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>
#include <APS/Exceptions.h>

namespace askap
{
  namespace analysis
  {

    /// @brief Send a RadioSource over a Blob Stream
    /// @ingroup blobrelated
/*     LOFAR::BlobOStream& operator<<(LOFAR::BlobOStream& blob, sourcefitting::RadioSource& src); */

    /// @brief Receive a duchamp::Detection from a Blob Stream
    /// @ingroup blobrelated
/*     duchamp::Detection receiveDetection(LOFAR::BlobIStream blob); */
  }

}


#endif
