/// @file AskapAsciiCatalogueWriter.cc
///
/// @copyright (c) 2014 CSIRO
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

#include <outputs/AskapAsciiCatalogueWriter.h>
#include <askap_analysis.h>
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <parallelanalysis/DuchampParallel.h>
#include <sourcefitting/RadioSource.h>
#include <sourcefitting/FitResults.h>
#include <catalogues/CasdaIsland.h>
#include <catalogues/CasdaComponent.h>

#include <duchamp/Outputs/ASCIICatalogueWriter.hh>
#include <duchamp/Outputs/columns.hh>

///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".askapasciicatwriter");

namespace askap {

namespace analysis {

using namespace duchamp::Catalogues;

AskapAsciiCatalogueWriter::AskapAsciiCatalogueWriter():
    duchamp::ASCIICatalogueWriter()
{
}

AskapAsciiCatalogueWriter::AskapAsciiCatalogueWriter(std::string name):
    duchamp::ASCIICatalogueWriter(name)
{
    itsDestination = duchamp::Catalogues::FILE;
}

AskapAsciiCatalogueWriter::AskapAsciiCatalogueWriter(duchamp::Catalogues::DESTINATION dest):
    duchamp::ASCIICatalogueWriter(dest)
{
}

AskapAsciiCatalogueWriter::AskapAsciiCatalogueWriter(std::string name,
        duchamp::Catalogues::DESTINATION dest):
    duchamp::ASCIICatalogueWriter(name, dest)
{
}

void AskapAsciiCatalogueWriter::writeTableHeader()
{
    if (itsOpenFlag) {
        *itsStream << "#";
        for (size_t i = 0; i < itsColumnSpecification->size(); i++) {
            itsColumnSpecification->column(i).printTitle(*itsStream);
        }
        *itsStream << "\n#";
        for (size_t i = 0; i < itsColumnSpecification->size(); i++) {
            itsColumnSpecification->column(i).printUnits(*itsStream);
        }
        *itsStream << "\n";
    }
}

template <class T>
void AskapAsciiCatalogueWriter::writeEntries(std::vector<T> &objlist)
{
    if (itsOpenFlag) {
        for (size_t i = 0; i < objlist.size(); i++) {
            this->writeEntry<T>(objlist[i]);
        }
    }
}
template void
AskapAsciiCatalogueWriter::writeEntries<CasdaIsland>(std::vector<CasdaIsland> &objlist);
template void
AskapAsciiCatalogueWriter::writeEntries<CasdaComponent>(std::vector<CasdaComponent> &objlist);

template <class T>
void AskapAsciiCatalogueWriter::writeEntry(T &obj)
{
    if (itsOpenFlag) {
        itsFileStream.setf(std::ios::fixed);
        *itsStream << " "; // to match the '#' at the start of the header rows
        for (size_t i = 0; i < itsColumnSpecification->size(); i++) {
            Column col = itsColumnSpecification->column(i);
            obj.printTableEntry(itsFileStream, col);
        }
        *itsStream << std::endl;

    }
}
template void AskapAsciiCatalogueWriter::writeEntry<CasdaIsland>(CasdaIsland &obj);
template void AskapAsciiCatalogueWriter::writeEntry<CasdaComponent>(CasdaComponent &obj);


}

}
