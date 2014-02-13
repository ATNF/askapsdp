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
#include <libgen.h>

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
    // Logging may have already been configured, for example by
    // askap::Application so first check
    if (!ASKAPLOG_ISCONFIGURED) {
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

    // Get program name
    const std::string progName = AskapParallel::getProgramName(argc, argv);

    if (isParallel()) {
        if (isMaster()) {
            ASKAPLOG_INFO_STR(logger, "ASKAP " << progName << " (parallel) running on " << itsNProcs
                                  << " nodes (master/master)");
        } else {
            ASKAPLOG_INFO_STR(logger, "ASKAP " << progName << " (parallel) running on " << itsNProcs
                                  << " nodes (worker " << itsRank << ")");
        }
    } else {
        ASKAPLOG_INFO_STR(logger, "ASKAP " << progName << " (serial)");
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
  ASKAPLOG_DEBUG_STR(logger, "MPI broadcast of the model will cover all workers");
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
  ASKAPLOG_DEBUG_STR(logger, "MPI broadcast of the model will cover group "<<group<<" of workers");             
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
  const int nWorkers = nProcs() - 1;
  ASKAPDEBUGASSERT(nWorkers > 0);
  const size_t nWorkersPerGroup = size_t(nWorkers) / itsNGroups;
  return (rank() - 1) % nWorkersPerGroup + itsNGroups + 1;
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
       ASKAPLOG_DEBUG_STR(logger, "Group "<<group<<" of workers will include ranks "<<ranks);
       const size_t commIndex = createComm(ranks);
       ASKAPCHECK(commIndex == (group + 1), "Unexpected commIndex value of "<<commIndex<<
                  " for group="<<group);
  }
  itsNGroups = nGroups;
  ASKAPDEBUGASSERT(nGroups > 1);
  // define intergroup communicator. There could be a better way of doing this.
  ranks.resize(nGroups,-1);  
  for (size_t wrk = 0; wrk < workersPerGroup; ++wrk) {
      for (size_t grp = 0; grp < nGroups; ++grp) {
           ranks[grp] = 1 + wrk + grp * workersPerGroup;
      }
      if (rank() == int(wrk + 1)) {
         ASKAPLOG_DEBUG_STR(logger, "Intergroup communicator for worker at rank "<<(wrk + 1)<<" will include ranks "<<ranks);
      }
      const size_t commIndex = createComm(ranks);
      ASKAPLOG_DEBUG_STR(logger, "Intergroup communicator index is "<<commIndex);
      ASKAPCHECK(commIndex == (itsNGroups + wrk + 1), "Unexpected commIndex value of "<<commIndex<<
                 " for worker "<<wrk<<" at rank"<<rank());
  }
}
        
/// @return number of groups of workers
size_t AskapParallel::nGroups() const
{
   return itsNGroups;
}

// this is a special tag for messages used in notifyMaster/waitForNotification communication pattern
// can be anything, but it provides extra protection if this tag is different from the tag used for data. 
#define ASKAPPARALLEL_NOTIFYMSG_TAG 1

/// @brief notify master that the worker is ready for some operation
/// @detais It is sometimes convenient to wait for a response from workers
/// that they're ready for some operation, e.g. to send the data. This method
/// along with waitForNotification allows us to implement this pattern and avoid
/// waiting for a reply from every worker in the order of their ranks. The pattern
/// could have been implemented as part of the general send/receive calls, but
/// the extra overhead of the current approach doesn't seem to be critical.
/// @param[in] msg optional message that is passed to the master (can be used as a 
/// continuation flag or as a notification that no more messages are expected), entirely
/// user-defined
void AskapParallel::notifyMaster(const int msg)
{
  ASKAPCHECK(isWorker(), "notifyMaster is only supposed to be called from workers");
  // note, we deliberately use 0 as communicator index, this will give MPI_WORLD
  // we also send directly to rank 0 which is the master
  send(&msg, sizeof(int), 0, ASKAPPARALLEL_NOTIFYMSG_TAG,0);    
}
        
/// @brief wait for a notification from a worker
/// @details This method is supposed to be used in pair with notifyMaster. It waits
/// for a short notification message from any source and returns passed message and
/// the rank of the worker.
/// @return a pair of two integers, the first element is the sender's rank and the second
/// is the optional message passed to notifyMaster
std::pair<int,int> AskapParallel::waitForNotification()
{
  ASKAPCHECK(isMaster(), "waitForNotification is only supposed to be called from the master");

  int msg = -1;
  // note, we deliberately use 0 as communicator index, this will give MPI_WORLD
  const int sender = receiveAnySrc(&msg, sizeof(int), ASKAPPARALLEL_NOTIFYMSG_TAG, 0);
  
  return std::pair<int,int>(sender,msg);
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

std::string AskapParallel::getProgramName(int argc, const char** argv)
{
    if (argc > 0) {
        char* path = strdup(argv[0]);
        char* bname = basename(path);
        if (bname) {
            return std::string(bname);
        }
        free(path);
        // No need to free bname, it is a pointer to somewhere in path
    }

    return "unknown";
}

} // End namespace askapparallel
} // End namespace askap
