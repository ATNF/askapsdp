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

        /// @brief Send a BlobString to the specified destination
        /// process.
        ///
        /// @param[in] buf a reference to the BlobString to send.
        /// @param[in] dest    the id of the process to send to.
        virtual void sendBlob(const LOFAR::BlobString& buf, int dest);

        /// @brief Receive a BlobString from the specified source process.
        /// The buffer is resized as needed.
        ///
        /// @param[out] buf a reference to the BlobString to receive data into.
        /// @param[in] source  the id of the process to receive from.
        virtual void receiveBlob(LOFAR::BlobString& buf, int source);

        /// @brief Broadcast a BlobString to all ranks.
        /// On the root rank, the BlobString is resized as needed.
        ///
        /// @param[in,out] buf    BlobString.
        /// @param[in] root       id of the root process.
        virtual void broadcastBlob(LOFAR::BlobString& buf, int root);

        /// Substitute %w by the worker number, and %n by the number of workers
        /// (one less than the number of nodes). Note, if there is more than one
        /// group of workers set up, %w is substituted by the group number.
        /// This allows workers to do different work and share the same setup within
        /// each group of workers.
        /// @param[in] s input string
        /// @return the string with the substitution done
        std::string substitute(const std::string& s) const;

        /// @brief configure to communicate with all workers
        /// @details This method selects the default communicator allowing
        /// to broadcast across all workers (default state).
        void useAllWorkers();

        /// @brief configure to communicate with a group of workers
        /// @param[in] group group number (0..itsNGroups-1)
        /// @note This method should only be used in the parallel mode
        void useGroupOfWorkers(size_t group);

        /// @brief check if this process belong to the given group
        /// @param[in] group group number (0..itsNGroups-1)
        /// @return true, if this process belongs to the given group
        bool inGroup(size_t group);

        /// @brief define groups of workers
        /// @details Master belongs to all groups (the communication pattern
        /// is between master and all workers of the same group). Note, 
        /// currently this method can only be called once per lifetime of the 
        /// object.
        /// @param[in] nGroups number of groups required
        void defineGroups(size_t nGroups);

        /// @return number of groups of workers
        size_t nGroups() const;
 
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
    private:
        /// @brief communicator index to use with all operations
        /// @note zero defaults to the world communicator created at the start
        size_t itsCommIndex;

        /// @brief number of worker groups (default is 1, i.e. no partitioning)
        /// @details We can partition workers into a number of groups. Communication
        /// like broadcasts can then happen within the group. In particular, we can
        /// partition the model between workers.
        size_t itsNGroups;
};

}
}
#endif
