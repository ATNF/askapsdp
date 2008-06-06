/// @file
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
#ifndef ASKAP_CP_ASKAPPARALLEL_H_
#define ASKAP_CP_ASKAPPARALLEL_H_

#include <mwcommon/MPIConnectionSet.h>


namespace askap
{
  namespace cp
  {
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
    ///
    /// @ingroup parallel
    class AskapParallel
    {
  public:

      /// @brief Constructor 
      /// @details The command line inputs are needed solely for MPI - currently no
      /// application specific information is passed on the command line.
      /// @param argc Number of command line inputs
      /// @param argv Command line inputs
      AskapParallel(int argc, const char** argv);

      ~AskapParallel();

      /// Is this running in parallel?
      bool isParallel();

      /// Is this the master?
      bool isMaster();

      /// Is this a worker?
      bool isWorker();

  protected:
      /// Initialize the MPI connections
      void initConnections();

      /// The set of all connections between processes. For the master, there
      /// are connections to every worker, but each worker has only one
      /// connection, which is to the master.
      askap::cp::MPIConnectionSet::ShPtr itsConnectionSet;

      /// Rank of this process : 0 for the master, >0 for workers
      int itsRank;

      /// Number of nodes
      int itsNNode;

      /// Is this parallel? itsNNode > 1?
      bool itsIsParallel;

      /// Is this the Master?
      bool itsIsMaster;

      /// Is this a worker?
      bool itsIsWorker;

      /// Substitute %w by worker number, and %n by number of workers (one less than the number of nodes) This allows workers to do different work!
      std::string substitute(const std::string& s);

    };

  }
}
#endif
