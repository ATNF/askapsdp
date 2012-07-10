/// @file AskapParallel.cc
///
/// @brief Base class for parallel applications
/// @details
/// Supports algorithms by providing methods for initialization
/// of MPI connections, sending and models around.
/// There is assumed to be one master and many workers.
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
#include "askapparallel/AskapParallel.h"

// Package level header file
#include "askap_askapparallel.h"

// System includes
#include <sstream>
#include <fstream>
#include <string>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "casa/Utilities/Regex.h"
#include "casa/BasicSL/String.h"

// Local package includes
#include "askapparallel/MPIComms.h"

using namespace std;
using namespace askap;

namespace askap {
namespace askapparallel {

/// Logger
ASKAP_LOGGER(logger, ".askapparallel");

AskapParallel::AskapParallel(int argc, const char** argv)
        : MPIComms(argc, const_cast<char**>(argv)), itsCommIndex(0),
          itsNGroups(1)
{
    // Now we have to initialize the logger before we use it
    // If a log configuration exists in the current directory then
    // use it, otherwise try to use the programs default one
    const std::ifstream config("askap.log_cfg", std::ifstream::in);

    if (config) {
        ASKAPLOG_INIT("askap.log_cfg");
    } else {
        std::ostringstream ss;
        ss << argv[0] << ".log_cfg";
        ASKAPLOG_INIT(ss.str().c_str());
    }

    itsNProcs = MPIComms::nProcs();
    itsRank = MPIComms::rank();

    // To aid in debugging, now we know the MPI rank
    // set the ID in the logger
    std::ostringstream ss;
    ss << itsRank;
    ASKAPLOG_REMOVECONTEXT("mpirank");
    ASKAPLOG_PUTCONTEXT("mpirank", ss.str().c_str());

    // Also set the nodename
    ASKAPLOG_REMOVECONTEXT("hostname");
    ASKAPLOG_PUTCONTEXT("hostname", nodeName().c_str());

    itsIsParallel = (itsNProcs > 1);
    itsIsMaster = (itsRank == 0);
    itsIsWorker = (!itsIsParallel) || (itsRank > 0);

    if (isParallel()) {
        if (isMaster()) {
            ASKAPLOG_INFO_STR(logger, "ASKAP program (parallel) running on " << itsNProcs
                                  << " nodes (master/master)");
        } else {
            ASKAPLOG_INFO_STR(logger, "ASKAP program (parallel) running on " << itsNProcs
                                  << " nodes (worker " << itsRank << ")");
        }
    } else {
        ASKAPLOG_INFO_STR(logger, "ASKAP program (serial)");
    }

    ASKAPLOG_INFO_STR(logger, ASKAP_PACKAGE_VERSION);
#ifdef _OPENMP
    ASKAPLOG_INFO_STR(logger, "Compiled with OpenMP support");
#else    
    ASKAPLOG_INFO_STR(logger, "Compiled without OpenMP support");
#endif

}

AskapParallel::~AskapParallel()
{
    ASKAPLOG_INFO_STR(logger, "Exiting MPI");
}

/// Is this running in parallel?
bool AskapParallel::isParallel() const
{
    return itsIsParallel;
}

/// Is this the master?
bool AskapParallel::isMaster() const
{
    return itsIsMaster;
}

/// Is this a worker?
bool AskapParallel::isWorker() const
{
    return itsIsWorker;
}

/// Rank
int AskapParallel::rank() const
{
    return itsRank;
}

/// Number of nodes
int AskapParallel::nProcs() const
{
    return itsNProcs;
}

/// @brief configure to communicate with all workers
/// @details This method selects the default communicator allowing
/// to broadcast across all workers (default state).
void AskapParallel::useAllWorkers()
{
  itsCommIndex = 0;
}

/// @brief configure to communicate with a group of workers
/// @param[in] group group number (0..itsNGroups-1)
/// @note This method should only be used in the parallel mode
void AskapParallel::useGroupOfWorkers(size_t group) 
{
  ASKAPCHECK(isParallel(), 
         "AskapParallel::useGroupOfWorkers should only be used in the parallel mode");
  ASKAPCHECK(group < itsNGroups, "AskapParallel::useGroupOfWorkers: group="<<group<<
             " total number of groups is "<<itsNGroups);
  itsCommIndex = group + 1;
}

/// @brief get intergroup communicator index 
/// @details This method returns communicator index for operations across
/// all groups workers (excluding the master and only for the current rank)
/// @return communicator index
/// @note This method should only be used in the parallel mode
size_t AskapParallel::interGroupCommIndex() const
{
  ASKAPCHECK(isParallel(), 
         "AskapParallel::interGroupCommIndex should only be used in the parallel mode");
  if (itsNGroups <= 1) {
      return 0;
  }
  return itsNGroups + 1;
}
        
        
/// @brief check if this process belongs to the given group
/// @param[in] group group number (0..itsNGroups-1)
/// @return true, if this process belongs to the given group
bool AskapParallel::inGroup(size_t group) const
{
  if (isMaster()) {
      return true; 
  }
  if (group < itsNGroups) {
      const int nWorkers = nProcs() - 1;
      ASKAPDEBUGASSERT(nWorkers > 0);
      const size_t nWorkersPerGroup = size_t(nWorkers) / itsNGroups;
      return group == (rank() - 1) / nWorkersPerGroup;
  }
  return false;
}
        
/// @brief obtain the current group number
/// @details This information can be changed at run time (although only once currently).
/// Therefore, it finds the current group using multiple calls to inGroup. It is 
/// supposed to be used in workers only as the master belongs to all groups.
/// @return the group containing the current worker process
size_t AskapParallel::group() const
{
  ASKAPCHECK(!isMaster() && isWorker(), "group() method is supposed to be used only in workers and only in the parallel mode");
  size_t currentGroup = nGroups(); // just a flag that group index is not found
  for (size_t grp = 0; grp< nGroups(); ++grp) {
       if (inGroup(grp)) {
           ASKAPCHECK(currentGroup == nGroups(), 
                      "Each worker can belong to one and only one group! "
                      "For some reason it belongs to groups "<<currentGroup<<" and "<<grp);
           currentGroup = grp;
       }
  }
  ASKAPCHECK(currentGroup < nGroups(), "The worker at rank="<<rank()<<
             "does not seem to belong to any group!");
  return currentGroup;
}
        
/// @brief define groups of workers
/// @details Master belongs to all groups (the communication pattern
/// is between master and all workers of the same group). Note, 
/// currently this method can only be called once per lifetime of the 
/// object.
/// @param[in] nGroups number of groups required
void AskapParallel::defineGroups(size_t nGroups)
{
  ASKAPDEBUGASSERT(nGroups > 0);
  ASKAPCHECK(itsNGroups == 1, "Currently, AskapParallel::defineGroups can only be called once");
  if (nGroups == 1) {
      return;
  }
  ASKAPCHECK(isParallel(), "AskapParallel::defineGroups is only supposed to be used in the parallel mode");
  const int nWorkers = nProcs() - 1;
  ASKAPCHECK(nWorkers % nGroups == 0, "Number of workers ("<<nWorkers<<
             ") cannot be evenly devided into "<<nGroups<<" groups");
  ASKAPDEBUGASSERT(nWorkers > 0);
  const size_t workersPerGroup = size_t(nWorkers) / nGroups;
  std::vector<int> ranks(workersPerGroup + 1, -1);
  ranks[0] = 0; // we always include master into all groups
  for (size_t group = 0; group<nGroups; ++group) {
       for (size_t worker = 0; worker < workersPerGroup; ++worker) {
            ranks[worker + 1] = 1 + worker + group * workersPerGroup;
       }
       ASKAPLOG_INFO_STR(logger, "Group "<<group<<" of workers will include ranks "<<ranks);
       const size_t commIndex = createComm(ranks);
       ASKAPCHECK(commIndex == group + 1, "Unexpected commIndex value of "<<commIndex<<
                  " for group="<<group);
  }
  itsNGroups = nGroups;
  if (isWorker()) {
      ASKAPDEBUGASSERT(nGroups > 1);
      const size_t firstWorker = (rank() - 1) % workersPerGroup;
      // define intergroup communicator. There could be a better way of doing this.
      ranks.resize(nGroups,-1);      
      for (size_t grp = 0; grp < nGroups; ++grp) {
           ranks[grp] = 1 + firstWorker + grp * workersPerGroup;
      }
      ASKAPLOG_INFO_STR(logger, "Intergroup communicator for worker "<<(rank() - 1)<<" will include ranks "<<ranks);
      const size_t commIndex = createComm(ranks);
      ASKAPCHECK(commIndex == itsNGroups + 1, "Unexpected commIndex value of "<<commIndex<<
                 " for worker="<<(rank() - 1)<<", first worker in the group is "<<firstWorker);
  }
}
        
/// @return number of groups of workers
size_t AskapParallel::nGroups() const
{
   return itsNGroups;
}

void AskapParallel::receiveBlob(LOFAR::BlobString& buf, int source)
{
    // First receive the size of the buffer so it can be resized first
    // before receiving the actual payload
    unsigned long size = 0;
    receive(&size, sizeof(unsigned long), source,0,itsCommIndex);
    buf.resize(size);
    receive(buf.data(), size, source,0,itsCommIndex);
}

void AskapParallel::sendBlob(const LOFAR::BlobString& buf, int dest)
{
    // First send the size of the buffer
    const unsigned long size = buf.size();
    send(&size, sizeof(unsigned long), dest,0,itsCommIndex);
    send(buf.data(), size, dest, 0, itsCommIndex);
}

void AskapParallel::broadcastBlob(LOFAR::BlobString& buf, int root)
{
    const bool isRoot = (rank() == root);

    // First broadcast the length of the message
    unsigned long size = isRoot ? buf.size() : 0;
    broadcast(&size, sizeof(unsigned long), root, itsCommIndex);

    if (!isRoot) {
        buf.resize(size);
    }
    broadcast(buf.data(), size, root, itsCommIndex);
}

std::string AskapParallel::substitute(const std::string& s) const
{
    casa::String cs(s);
    {
        const casa::Regex regWork("\%w");
        ostringstream oos;

        if (itsNProcs > 1) {
            ASKAPDEBUGASSERT(itsNGroups >= 1);
            const int nWorkers = nProcs() - 1;
            ASKAPDEBUGASSERT(nWorkers > 0);
            const size_t nWorkersPerGroup = size_t(nWorkers) / itsNGroups;
            oos << (itsRank - 1) % nWorkersPerGroup;
        } else {
            oos << 0;
        }

        cs.gsub(regWork, oos.str());
    }
    const casa::Regex regNode("\%n");
    {
        ostringstream oos;

        if (itsNProcs > 1) {
            oos << itsNProcs - 1;
        } else {
            oos << 1;
        }

        cs.gsub(regNode, oos.str());
    }
    return string(cs);
}

} // End namespace askapparallel
} // End namespace askap
