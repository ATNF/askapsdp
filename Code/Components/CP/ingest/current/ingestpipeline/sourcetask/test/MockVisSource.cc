/// @file MockVisSource.cc
///
/// @copyright (c) 2010 CSIRO
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
#include "MockVisSource.h"

// System includes
#include <deque>

// ASKAPsoft includes
#include "boost/shared_ptr.hpp"
#include "cpcommon/VisDatagram.h"

// Local package includes
#include "ingestpipeline/sourcetask/IVisSource.h"

using namespace askap;
using namespace askap::cp;
using namespace askap::cp::ingest;

MockVisSource::MockVisSource()
{
}

MockVisSource::~MockVisSource()
{
}

void MockVisSource::add(boost::shared_ptr< VisDatagram > obj)
{
    itsBuffer.add(obj);
}

boost::shared_ptr< VisDatagram >
MockVisSource::next(const long timeout)
{
    // Timeout is ignored for the MockVisSource. It is expected that anything
    // you wish to get out of it, you must have added to it with the add() call
    return itsBuffer.next();
}
