/// @file
///
/// @brief Base class for parallel synthesis applications
/// @details
/// Supports parallel algorithms by providing methods for initialization
/// of MPI connections, sending normal equations and models around.
/// There is assumed to be one solver and many prediffers.
///
/// @copyright (c) 2007 CSIRO
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

// Include own header file first
#include <parallel/MEParallel.h>

#include <Blob/BlobString.h>
#include <Blob/BlobIBufString.h>
#include <Blob/BlobOBufString.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>
#include <casa/OS/Timer.h>

#include <askap_synthesis.h>
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <askap_synthesis.h>
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, ".parallel");

#include <mwcommon/AskapParallel.h>
#include <fitting/ImagingNormalEquations.h>

using namespace askap;
using namespace askap::mwcommon;
using namespace askap::scimath;

namespace askap
{
  namespace synthesis
  {

    MEParallel::MEParallel(askap::mwcommon::AskapParallel& comms, const LOFAR::ParameterSet& parset) :
      SynParallel(comms,parset)
    {
      itsSolver = Solver::ShPtr(new Solver);
      itsNe = ImagingNormalEquations::ShPtr(new ImagingNormalEquations(*itsModel));
    }

    MEParallel::~MEParallel()
    {
    }

    // Send the normal equations from this worker to the master as a blob
    void MEParallel::sendNE()
    {
      if (itsComms.isParallel() && itsComms.isWorker())
      {
        casa::Timer timer;
        timer.mark();
        ASKAPLOG_INFO_STR(logger, "Sending normal equations to the solver via MPI" );

        LOFAR::BlobString bs;
        bs.resize(0);
        LOFAR::BlobOBufString bob(bs);
        LOFAR::BlobOStream out(bob);
        out.putStart("ne", 1);
        out << itsComms.rank() << *itsNe;
        out.putEnd();
        itsComms.connectionSet()->write(0, bs);
        ASKAPLOG_INFO_STR(logger, "Sent normal equations to the solver via MPI in "
                           << timer.real()<< " seconds ");
      }
    }

    // Receive the normal equations as a blob
    void MEParallel::receiveNE()
    {
      ASKAPCHECK(itsSolver, "Solver not yet defined");
      if (itsComms.isParallel() && itsComms.isMaster())
      {
        ASKAPLOG_INFO_STR(logger, "Initialising solver");
        itsSolver->init();

        ASKAPLOG_INFO_STR(logger, "Waiting to receive normal equations");
        casa::Timer timer;
        timer.mark();

        LOFAR::BlobString bs;
        int rank;

        for (int i=1; i<itsComms.nNodes(); i++)
        {
          itsComms.connectionSet()->read(i-1, bs);
          LOFAR::BlobIBufString bib(bs);
          LOFAR::BlobIStream in(bib);
          int version=in.getStart("ne");
          ASKAPASSERT(version==1);
          in >> rank >> *itsNe;
          in.getEnd();
          itsSolver->addNormalEquations(*itsNe);
          ASKAPLOG_INFO_STR(logger, "Received normal equations from prediffer "<< rank
                             << " after "<< timer.real() << " seconds");
        }
        ASKAPLOG_INFO_STR(logger, "Received normal equations from all prediffers via MPI in "
            << timer.real() << " seconds");
      }
    }

    void MEParallel::writeModel(const std::string &)
    {
    }

  }
}
