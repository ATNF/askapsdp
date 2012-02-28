/// @file AskapParallel.h
///
/// Provides generic methods for parallel algorithms
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
#ifndef ASKAP_ASKAPPARALLEL_ASKAPPARALLEL_H
#define ASKAP_ASKAPPARALLEL_ASKAPPARALLEL_H

// System includes
#include <string>

// AskapSoft includes
#include "Blob/BlobString.h"

// Local package includes
#include "askapparallel/MPIComms.h"

namespace askap {
namespace askapparallel {

/// @brief Support for parallel algorithms
///
/// @details Support for parallel applications in the area.
/// An application is derived from this abstract base. The model used is that the
/// application has many workers and one master, running in separate MPI processes
/// or in one single thread. The master is the master so the number of processes
/// is one more than the number of workers.
///
/// If the number of nodes is 1 then everything occurs in the same process with
/// no overall for transmission of model.
class AskapParallel : public MPIComms {
    public:

        /// @brief Constructor
        /// @details The command line inputs are needed solely for MPI - currently no
        /// application specific information is passed on the command line.
        /// @param argc Number of command line inputs
        /// @param argv Command line inputs
        AskapParallel(int argc, const char** argv);

        /// @brief Destructor
        ~AskapParallel();

        /// Is this running in parallel?
        virtual bool isParallel() const;

        /// Is this the master?
        virtual bool isMaster() const;

        /// Is this a worker?
        virtual bool isWorker() const;

        /// Rank
        virtual int rank() const;

        /// Number of processes
        virtual int nProcs() const;

        /// Receive the data blob sent by the connected MWConnection
        /// and wait until data has been received into \a buf.
        /// The buffer is resized as needed.
        /// By default it uses the functions \a getMessageLength and \a receive
        /// to determine the length of the message and to receive the data.
        virtual void receiveBlob(LOFAR::BlobString& buf, int source);

        /// Send the data to the connected MWConnection
        /// and wait until the data has been sent.
        /// By default is uses function \a send to send the data.
        virtual void sendBlob(const LOFAR::BlobString& buf, int dest);

        /// @brief broadcast blob to all ranks by the connected MWConnection
        /// @details this method waits until all data has arrived into \a buf.
        /// The buffer is resized as needed.
        /// @param[in] buf blob string
        /// @param[in] root root rank which has the data
        virtual void broadcastBlob(LOFAR::BlobString& buf, int root);

        /// Substitute %w by worker number, and %n by number of workers
        // (one less than the number of nodes) This allows workers to
        // do different work.
        std::string substitute(const std::string& s) const;

    protected:
        /// Rank of this process : 0 for the master, >0 for workers
        int itsRank;

        /// Number of nodes
        int itsNProcs;

        /// Is this parallel? itsNNode > 1?
        bool itsIsParallel;

        /// Is this the Master?
        bool itsIsMaster;

        /// Is this a worker?
        bool itsIsWorker;
};

}
}
#endif
