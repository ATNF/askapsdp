/// @file tMpiComms.cc
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

// Include package level header file
#include <askap_imager.h>

// System includes
#include <iostream>
#include <vector>

// ASKAPsoft includes
#include <fitting/Params.h>

// Local Package includes
#include "distributedimager/common/MPIBasicComms.h"
#include "messages/UpdateModel.h"

using namespace askap;
using namespace askap::cp;
using askap::scimath::Params;

// For logging, store the rank
static int id;

static void reportPass(const std::string& str)
{
    std::cout << "[Rank " << id << "]" << str << ": PASS" << std::endl;
}

static void reportFail(const std::string& str)
{
    std::cout << "[Rank " << id << "]" << str << ": FAIL" << std::endl;
}

static int testGetId(MPIBasicComms& comms)
{
    if (comms.getId() < 0) {
        std::cout << "testGetId(): getId() returned < 0" << std::endl;
    }

    return 0;
}

static int testGetNumNodes(MPIBasicComms& comms)
{
    if (comms.getNumNodes() < 2) {
        std::cout << "testGetId(): getId() returned < 2" << std::endl;
    }

    return 0;
}

static int testBroadcastUpdateModel(MPIBasicComms& comms, const casa::IPosition& dim)
{
    const int root = 0;
    if (comms.getId() == root) {
        // Send the message
        Params::ShPtr model_p(new Params());

        // Inside block tofree array after add().
        {
            casa::Array<double> array(dim);
            array.set(8);
            model_p->add("testparam", array);
        }

        UpdateModel message;
        message.set_model(model_p);
        comms.sendMessageBroadcast(message);
    } else {
        // Receive the message
        UpdateModel updatemsg;
        comms.receiveMessageBroadcast(updatemsg, root);
        Params::ShPtr model_p = updatemsg.get_model();

        // Verify message
        if (model_p->size() != 1) {
            std::cout << "testBroadcastUpdateModel: Wrong size" << std::endl;
            return 1;
        }

        const casa::Array<double>& array = model_p->value("testparam");
        if (array.shape() != dim) {
            std::cout << "testBroadcastUpdateModel: Wrong shape" << std::endl;
            return 1;
        }
    }

    return 0;
}

int main(int argc, char *argv[])
{
    MPIBasicComms comms(argc, argv);

    id = comms.getId();

    for (int i = 0; i < 10; ++i) {
    if (testGetId(comms) == 0) {
        reportPass("testGetId()");
    } else {
        reportFail("testGetId()");
    }

    if (testGetNumNodes(comms) == 0) {
        reportPass("testGetNumNodes()");
    } else {
        reportFail("testGetNumNodes()");
    }


    const casa::IPosition dimSmall(2, 1024, 1024);
    if (testBroadcastUpdateModel(comms, dimSmall) == 0) {
        reportPass("testBroadcastUpdateModel(1x1024x1028)");
    } else {
        reportFail("testBroadcastUpdateModel(1x1024x1028)");
    }

    const casa::IPosition dimLarge(3, 4, 8192, 8192);
    if (testBroadcastUpdateModel(comms, dimLarge) == 0) {
        reportPass("testBroadcastUpdateModel(4x8192x8192)");
    } else {
        reportFail("testBroadcastUpdateModel(4x8192x8192)");
    }
    }

    return 0;
}
