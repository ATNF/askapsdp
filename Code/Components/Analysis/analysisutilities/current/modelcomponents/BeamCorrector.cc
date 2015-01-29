/// @file
///
/// Simple class to manage beam metadata from image that generated a catalogue
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
/// @author Matthew Whiting <Matthew.Whiting@csiro.au>
///
#include <askap_analysisutilities.h>

#include <modelcomponents/BeamCorrector.h>

#include <casainterface/CasaInterface.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <modelcomponents/Spectrum.h>

#include <duchamp/FitsIO/Beam.hh>
#include <Common/ParameterSet.h>
#include <boost/shared_ptr.hpp>

#include <casa/aipstype.h>
#include <images/Images/FITSImage.h>
#include <images/Images/MIRIADImage.h>
#include <images/Images/ImageOpener.h>
#include <coordinates/Coordinates/Coordinate.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/Unit.h>

#include <string>
#include <vector>

ASKAP_LOGGER(logger, ".beamcorrector");

namespace askap {

namespace analysisutilities {

BeamCorrector::BeamCorrector()
{
    itsBeam = duchamp::Beam(1., 1., 0.);
    itsFilename = "";
}

BeamCorrector::BeamCorrector(const BeamCorrector& other)
{
    this->operator=(other);
}

BeamCorrector& BeamCorrector::operator= (const BeamCorrector& other)
{
    if (this == &other) return *this;
    itsFilename = other.itsFilename;
    itsBeam = other.itsBeam;
    itsPixelScale = other.itsPixelScale;
    itsDirUnits = other.itsDirUnits;
    return *this;
}

BeamCorrector::BeamCorrector(const LOFAR::ParameterSet& parset)
{
    itsFilename = "";
    if (parset.isDefined("image")) {
        itsFilename = parset.getString("image");
        if (itsFilename != "") this->findBeam();
    } else {
        // Do not have BeamCorrector defined, so read beam and pixel info separately
        std::vector<float> beam;
        if (parset.isDefined("beam")) {
            beam = parset.getFloatVector("beam");
        } else {
            ASKAPLOG_ERROR_STR(logger, "You have not defined 'image' or 'beam'" <<
                               " in the beam correction parset:\n" << parset);
        }
        ASKAPASSERT(beam.size() == 3);
        itsPixelScale = parset.getFloat("pixscale");
        itsDirUnits = parset.getString("dirunits");
        itsBeam.define(beam[0], beam[1], beam[2]);
    }
}

void BeamCorrector::findBeam()
{
    const boost::shared_ptr<ImageInterface<Float> > imagePtr = openImage(itsFilename);

    casa::Vector<casa::Quantum<casa::Double> > beam = imagePtr->imageInfo().restoringBeam();
    ASKAPLOG_DEBUG_STR(logger, "Read beam from " << itsFilename << " of " << beam);
    casa::CoordinateSystem csys = imagePtr->coordinates();
    int dirCoord = csys.findCoordinate(casa::Coordinate::DIRECTION);
    casa::Vector<casa::Double> increment = csys.directionCoordinate(dirCoord).increment();
    casa::Vector<casa::String> dirUnits = csys.directionCoordinate(dirCoord).worldAxisUnits();
    ASKAPASSERT(increment.size() == 2);
    itsPixelScale = sqrt(fabs(increment[0] * increment[1]));
    ASKAPLOG_DEBUG_STR(logger, "Read direction axis increment of " << increment <<
                       " with units " << dirUnits << " and got pixel scale of " <<
                       itsPixelScale);
    ASKAPASSERT(dirUnits[0] == dirUnits[1]);
    itsDirUnits = dirUnits[0];

    if (beam.size() == 0)
        itsBeam.setArea(1.);
    else {
        double bmaj = beam[0].getValue(itsDirUnits) / itsPixelScale;
        double bmin = beam[1].getValue(itsDirUnits) / itsPixelScale;
        double bpa = beam[2].getValue("deg");
        itsBeam.define(bmaj, bmin, bpa);
        ASKAPLOG_DEBUG_STR(logger,
                           "Defined BeamCorrector beam with maj=" << itsBeam.maj() <<
                           ", min=" << itsBeam.min() <<
                           ", pa=" << itsBeam.pa() <<
                           " and area=" << itsBeam.area());
    }

}

void BeamCorrector::convertSource(boost::shared_ptr<Spectrum> src)
{

    src->setFluxZero(src->fluxZero() * itsBeam.area());

}

std::vector<float> BeamCorrector::beam()
{

    std::vector<float> outputbeam(3);
    outputbeam[0] = casa::Quantity(itsBeam.maj() * itsPixelScale,
                                   itsDirUnits).getValue("deg");
    outputbeam[1] = casa::Quantity(itsBeam.min() * itsPixelScale,
                                   itsDirUnits).getValue("deg");
    outputbeam[2] = itsBeam.pa();
    return outputbeam;
}

}

}
