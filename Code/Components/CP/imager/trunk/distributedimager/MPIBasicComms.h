/// @file MPIBasicComms.h
///
/// @copyright (c) 2009 CSIRO
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
/// @author Ben Humphreys <ben.humphreys@csiro.au>

#ifndef ASKAP_CP_MPICOMMS_H
#define ASKAP_CP_MPICOMMS_H

// System includes
#include <string>
#include <cstring>
#include <mpi.h>

// Local package includes
#include "IBasicComms.h"

namespace askap {
namespace cp {

class MPIBasicComms : public IBasicComms
{
public:
    MPIBasicComms(int argc, char *argv[]);
    virtual ~MPIBasicComms();

    virtual int getId(void);
    virtual int getNumNodes(void);
    virtual void abort(void);

    virtual void broadcastModel(askap::scimath::Params::ShPtr model);
    virtual askap::scimath::Params::ShPtr receiveModel(void);

    virtual void sendNE(askap::scimath::INormalEquations::ShPtr ne, int id, int count);
    virtual askap::scimath::INormalEquations::ShPtr receiveNE(int& id, int& count);

    virtual void sendString(const std::string& str, int dest);
    virtual std::string receiveString(int source);
    virtual std::string receiveStringAny(int& source);

private:
    void send(const void* buf, size_t size, int dest);
    void receive(void* buf, size_t size, int source, MPI_Status& status);
    void broadcast(void* buf, size_t size, int root);

    void handleError(const int error, const std::string location);

    void* addOffset(const void *ptr, size_t offset);

    // Root for broadcasts
    static const int c_root = 0;

    // Specific MPI Communicator for this class
    MPI_Comm m_communicator;
};

};
};

#endif
