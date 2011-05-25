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

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "SkyModelService.h" // Ice generated interface
#include "casa/Quanta/Quantum.h"

// Local package includes
#include "skymodelclient/Component.h"

// Using
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
    ASKAPCHECK(itsComponentList && itsService, "Iterator not initialised");
    if ((itsComponentBuffer.size() > 1) || (itsIndex < (itsComponentList->size()))) {
        return true;
    } else {
        return false;
    }
}

// The iterator always points to the component at the front of the 
// itsComponentBuffer queue. The next() method has a post condition
// which ensures the buffer is not empty.
void ComponentResultSet::Iterator::next()
{
    // Pre-conditions
    ASKAPCHECK(hasNext(), "Component result set overrun");
    ASKAPCHECK(itsComponentList && itsService, "Iterator not initialised");

    // If the deque has less than two components, and if there
    // are more to be read from the server then buffer another batch
    if ((itsComponentBuffer.size() <= 2) &&
            (itsIndex+1 < itsComponentList->size())) {
        fillBuffer();
    }

    // Assuming the buffer has more than two components (checked above)
    // then incrementing the iterator simply involves poping the component
    // off the front of the deque.
    itsComponentBuffer.pop_front();

    // Post-conditions
    ASKAPCHECK(itsIndex <= itsComponentList->size(),
            "Index to component list out of bounds");
    ASKAPCHECK(!itsComponentBuffer.empty(), "Component list is empty");
}

const Component& ComponentResultSet::Iterator::operator*()
{
    ASKAPCHECK(itsComponentList && itsService, "Iterator not initialised");
    ASKAPDEBUGASSERT(itsComponentBuffer.front().get() != 0);
    return *(itsComponentBuffer.front());
}

const Component* ComponentResultSet::Iterator::operator->()
{
    ASKAPCHECK(itsComponentList && itsService, "Iterator not initialised");
    return itsComponentBuffer.front().get();
}

void ComponentResultSet::Iterator::init(
        const askap::interfaces::skymodelservice::ComponentIdSeq* componentList,
        askap::interfaces::skymodelservice::ISkyModelServicePrx* service)
{
    ASKAPCHECK(componentList->size() > 0, "Component list is empty");
    itsComponentList = componentList;
    itsService = service;
    fillBuffer();
}

void ComponentResultSet::Iterator::fillBuffer(void)
{
    // Pre-conditions
    ASKAPCHECK(itsComponentList && itsService, "Iterator not initialised");
    ASKAPCHECK(itsIndex < itsComponentList->size(),
            "Index to component list out of bounds");

    // Maximum number of components to get in one batch
    const size_t batchSize = 1000;

    // Build a list of which components to obtain
    askap::interfaces::skymodelservice::ComponentIdSeq ids;
    for (size_t i = 0; i < batchSize; ++i) {
        if (itsIndex >= itsComponentList->size()) {
            break;
        }
        ids.push_back(itsComponentList->at(itsIndex));
        itsIndex++;
    }
    ASKAPCHECK(ids.size() > 0, "Component list is empty");

    // Perform the RPC
    askap::interfaces::skymodelservice::ComponentSeq resultset = (*itsService)->getComponents(ids);
    ASKAPCHECK(ids.size() == resultset.size(), "Downloaded list size != requested size");

    // Add those recieved components to the deque
    for (size_t i = 0; i < resultset.size(); ++i) {
        askap::interfaces::skymodelservice::Component* c = &resultset[i]; 
        boost::shared_ptr<Component> component(new Component(c->id,
                    casa::Quantity(c->rightAscension, "deg"),
                    casa::Quantity(c->declination, "deg"),
                    casa::Quantity(c->positionAngle, "rad"),
                    casa::Quantity(c->majorAxis, "arcsec"),
                    casa::Quantity(c->minorAxis, "arcsec"),
                    casa::Quantity(c->i1400, "Jy")));
        itsComponentBuffer.push_back(component);
    }

    // Post-conditions
    ASKAPCHECK(itsIndex <= itsComponentList->size(),
            "Index to component list out of bounds");
    ASKAPCHECK(!itsComponentBuffer.empty(), "Component list is empty");
}
