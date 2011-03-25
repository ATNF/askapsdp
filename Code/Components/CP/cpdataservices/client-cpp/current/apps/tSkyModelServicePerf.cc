/// @file tSkyModelServicePerf.cc
///
/// @description
///
/// @copyright (c) 2011 CSIRO
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

// System includes
#include <string>
#include <cstdlib>

// Local package includes
#include "skymodelclient/SkyModelServiceClient.h"
#include "skymodelclient/Component.h"

using namespace askap::cp::skymodelservice;

Component genRandomComponent(void)
{
    long id = -1;
    double rightAscension = drand48();
    double declination = drand48();
    double positionAngle = drand48();
    double majorAxis = drand48();
    double minorAxis = drand48();
    double i1400 = drand48();

    return Component(id, rightAscension, declination, positionAngle, majorAxis, minorAxis, i1400);
}

void populate(SkyModelServiceClient& svc, const unsigned long count)
{
    std::vector<Component> components;
    for (unsigned long i = 0; i < count; ++i) {
        components.push_back(genRandomComponent());
    }

    svc.addComponents(components);
}

void coneSearch(SkyModelServiceClient& svc, double rightAscension, double declination, double searchRadius)
{
    std::vector<ComponentId> resultset = svc.coneSearch(rightAscension, declination, searchRadius);
    std::cout << "Cone search returned " << resultset.size() << " components" << std::endl;
}

// main()
int main(int argc, char *argv[])
{
    SkyModelServiceClient svc("localhost", "4061");
    populate(svc, 100);
    coneSearch(svc, 0.0, 0.0, 180.0);
    return 0;
}
