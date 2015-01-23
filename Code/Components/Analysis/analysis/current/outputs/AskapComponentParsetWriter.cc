/// @file AskapComponentParsetWriter.cc
///
/// XXX Notes on program XXX
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
/// @author Matthew Whiting <Matthew.Whiting@csiro.au>
///
#include <outputs/AskapComponentParsetWriter.h>
#include <askap_analysis.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <outputs/ParsetComponent.h>
#include <sourcefitting/RadioSource.h>
#include <coordutils/PositionUtilities.h>

#include <duchamp/Outputs/ASCIICatalogueWriter.hh>

#include <scimath/Functionals/Gaussian2D.h>

///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".askapcomponentparsetwriter");

namespace askap {

namespace analysis {

AskapComponentParsetWriter::AskapComponentParsetWriter():
    duchamp::ASCIICatalogueWriter(), itsSourceList(0), itsFitType("best"),
    itsRefRA(0.), itsRefDec(0.), itsFlagReportSize(false), itsSourceIDlist("")
{
    itsOpenFlag = false;
    itsDestination = duchamp::Catalogues::FILE;
}

AskapComponentParsetWriter::AskapComponentParsetWriter(std::string name):
    duchamp::ASCIICatalogueWriter(name), itsSourceList(0), itsFitType("best"),
    itsRefRA(0.), itsRefDec(0.), itsFlagReportSize(false), itsSourceIDlist("")
{
    itsOpenFlag = false;
    itsDestination = duchamp::Catalogues::FILE;
}

AskapComponentParsetWriter::AskapComponentParsetWriter(duchamp::Catalogues::DESTINATION dest):
    duchamp::ASCIICatalogueWriter(dest), itsSourceList(0), itsFitType("best"),
    itsRefRA(0.), itsRefDec(0.), itsFlagReportSize(false), itsSourceIDlist("")
{
    itsOpenFlag = false;
}

AskapComponentParsetWriter::AskapComponentParsetWriter(std::string name,
        duchamp::Catalogues::DESTINATION dest):
    duchamp::ASCIICatalogueWriter(name, dest), itsSourceList(0), itsFitType("best"),
    itsRefRA(0.), itsRefDec(0.), itsFlagReportSize(false), itsSourceIDlist("")
{
    itsOpenFlag = false;
}

AskapComponentParsetWriter::AskapComponentParsetWriter(const AskapComponentParsetWriter& other)
{
    this->operator=(other);
}

AskapComponentParsetWriter&
AskapComponentParsetWriter::operator= (const AskapComponentParsetWriter& other)
{
    if (this == &other) return *this;
    ((ASCIICatalogueWriter &) *this) = other;
    itsSourceList = other.itsSourceList;
    itsFitType = other.itsFitType;
    itsRefRA = other.itsRefRA;
    itsRefDec = other.itsRefDec;
    itsFlagReportSize = other.itsFlagReportSize;
    itsSourceIDlist = other.itsSourceIDlist;
    itsMaxNumComponents = other.itsMaxNumComponents;
    return *this;
}

void AskapComponentParsetWriter::setup(duchamp::Cube *cube)
{
    this->CatalogueWriter::setup(cube);
    itsRefRA =  itsHead->getWCS()->crval[0];
    itsRefDec = itsHead->getWCS()->crval[1];

}

void AskapComponentParsetWriter::writeTableHeader()
{
    if (itsOpenFlag) {
        *itsStream << "sources.names = field1\n";
        std::string raRef = analysisutilities::decToDMS(itsRefRA, "RA", 4, "parset");
        std::string decRef = analysisutilities::decToDMS(itsRefDec, "DEC", 3, "parset");
        *itsStream << "sources.field1.direction = [" <<
                         raRef << ", " << decRef << ", J2000]\n";
    }
}

void AskapComponentParsetWriter::writeEntries()
{

    if (itsOpenFlag) {

        std::multimap <float, ParsetComponent> componentList;
        std::multimap <float, ParsetComponent>::reverse_iterator cmpntIter;
        ParsetComponent cmpnt;
        cmpnt.setHeader(itsHead);
        cmpnt.setReference(itsRefRA, itsRefDec);
        cmpnt.setSizeFlag(itsFlagReportSize);

        // First iterate over all components, storing them in a multimap indexed by their flux.
        std::vector<sourcefitting::RadioSource>::iterator src;
        for (src = itsSourceList->begin();
                src < itsSourceList->end();
                src++) {
            std::vector<casa::Gaussian2D<Double> > fitset = src->gaussFitSet(itsFitType);
            for (size_t i = 0; i < fitset.size(); i++) {
                cmpnt.defineComponent(&*src, i, itsFitType);
                componentList.insert(std::pair<float, ParsetComponent>(cmpnt.flux(), cmpnt));
            }
        }

        int count = 0;
        // only do this many components. If negative, do them all.
        int maxCount = (itsMaxNumComponents > 0) ?
                       itsMaxNumComponents : componentList.size();
        std::stringstream idlist;
        idlist << itsSourceIDlist;

        // Work down the list, starting at the brightest
        // component, writing out the parset details to the
        // file and keeping track of the list of source IDs
        for (cmpntIter = componentList.rbegin();
                cmpntIter != componentList.rend() && count < maxCount;
                cmpntIter++, count++) {

            *itsStream << cmpntIter->second;
            // update source ID list
            if (itsSourceIDlist.size() > 0) idlist << ",";
            idlist << "src" << cmpntIter->second.ID();
            itsSourceIDlist = idlist.str();
        }

    }

}

void AskapComponentParsetWriter::writeFooter()
{
    if (itsOpenFlag) {
        *itsStream <<  "sources.field1.components = [" << itsSourceIDlist << "]\n";
    }
}

}

}
