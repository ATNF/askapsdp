/// @file ComponentResultSet.cc
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

// Include own header file first
#include "ComponentResultSet.h"

// Include package level header file
#include "askap_cpdataservices.h"

// System includes

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "SkyModelService.h" // Ice generated interface

// Local package includes
#include "skymodelclient/Component.h"

// Using
using namespace std;
using namespace askap::cp::skymodelservice;

ComponentResultSet::ComponentResultSet(const askap::interfaces::skymodelservice::ComponentIdSeq& componentList,
        askap::interfaces::skymodelservice::ISkyModelServicePrx& service)
    : itsComponentList(componentList), itsService(service)
{
}

size_t ComponentResultSet::size() const
{
    return itsComponentList.size();
}

ComponentResultSet::Iterator ComponentResultSet::createIterator()
{
    Iterator it;
    it.init(&itsComponentList, &itsService);
    return it;
}

ComponentResultSet::Iterator::Iterator()
    : itsIndex(0), itsComponentList(0), itsService(0)
{
}

bool ComponentResultSet::Iterator::hasNext() const
{
    if (itsIndex < (itsComponentList->size() - 1)) {
        return true;
    } else {
        return false;
    }
}

void ComponentResultSet::Iterator::next()
{
    ASKAPDEBUGASSERT(itsComponentList->size() > 0);

    askap::interfaces::skymodelservice::ComponentIdSeq ids;
    ids.push_back(itsComponentList->at(itsIndex));
    itsIndex++;

    askap::interfaces::skymodelservice::ComponentSeq resultset = (*itsService)->getComponents(ids);
    ASKAPDEBUGASSERT(resultset.size() > 0);
    askap::interfaces::skymodelservice::Component* c = &resultset[0]; 
    itsComponent.reset(new Component(c->id,
                c->rightAscension,
                c->declination,
                c->positionAngle,
                c->majorAxis,
                c->minorAxis,
                c->i1400));
}

const Component& ComponentResultSet::Iterator::operator*()
{
    ASKAPDEBUGASSERT(itsComponent.get() != 0);
    return *itsComponent;
}

const Component* ComponentResultSet::Iterator::operator->()
{
    return itsComponent.get();
}

void ComponentResultSet::Iterator::init(
        const askap::interfaces::skymodelservice::ComponentIdSeq* componentList,
        askap::interfaces::skymodelservice::ISkyModelServicePrx* service)
{
    itsComponentList = componentList;
    itsService = service;
    next();
}
