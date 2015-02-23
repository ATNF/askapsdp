/// @file AskapVOTableCatalogueWriter.cc
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

#include <outputs/AskapVOTableCatalogueWriter.h>
#include <askap_analysis.h>

#include <parallelanalysis/DuchampParallel.h>
#include <sourcefitting/RadioSource.h>
#include <sourcefitting/FitResults.h>
#include <catalogues/CasdaIsland.h>
#include <catalogues/CasdaComponent.h>

#include <duchamp/Outputs/VOTableCatalogueWriter.hh>
#include <duchamp/Outputs/columns.hh>
#include <duchamp/Utils/VOField.hh>
#include <duchamp/Utils/utils.hh>

namespace askap {

namespace analysis {

using sourcefitting::RadioSource;
using std::pair;
using std::string;
using namespace duchamp::Catalogues;

AskapVOTableCatalogueWriter::AskapVOTableCatalogueWriter():
    duchamp::VOTableCatalogueWriter()
{
}

AskapVOTableCatalogueWriter::AskapVOTableCatalogueWriter(string name):
    duchamp::VOTableCatalogueWriter(name)
{
}


void AskapVOTableCatalogueWriter::writeTableHeader()
{
    if (itsOpenFlag) {

        std::map<string, string> posUCDmap;
        posUCDmap.insert(pair<string, string>("ra", "pos.eq.ra"));
        posUCDmap.insert(pair<string, string>("ra_deg_cont", "pos.eq.ra"));
        posUCDmap.insert(pair<string, string>("dec", "pos.eq.dec"));
        posUCDmap.insert(pair<string, string>("dec_deg_cont", "pos.eq.dec"));
        posUCDmap.insert(pair<string, string>("glon", "pos.galactic.lng"));
        posUCDmap.insert(pair<string, string>("glat", "pos.galactic.lat"));
        Column &raCol = itsColumnSpecification->column("RAJD");
        string lngUCDbase = posUCDmap[makelower(raCol.getName())];
        Column &decCol = itsColumnSpecification->column("DECJD");
        string latUCDbase = posUCDmap[makelower(decCol.getName())];

        std::map<string, string> specUCDmap;
        specUCDmap.insert(pair<string, string>("velo", "phys.veloc;spect.dopplerVeloc"));
        specUCDmap.insert(pair<string, string>("vopt", "phys.veloc;spect.dopplerVeloc.opt"));
        specUCDmap.insert(pair<string, string>("vrad", "phys.veloc;spect.dopplerVeloc.rad"));
        specUCDmap.insert(pair<string, string>("freq", "em.freq"));
        specUCDmap.insert(pair<string, string>("ener", "em.energy"));
        specUCDmap.insert(pair<string, string>("wavn", "em.wavenumber"));
        specUCDmap.insert(pair<string, string>("wave", "em.wl"));
        specUCDmap.insert(pair<string, string>("awav", "em.wl"));
        specUCDmap.insert(pair<string, string>("zopt", "src.redshift"));
        specUCDmap.insert(pair<string, string>("beta", "src.redshift; spect.dopplerVeloc"));
        Column &velCol = itsColumnSpecification->column("VEL");
        string specUCDbase = specUCDmap[makelower(velCol.getName())];

        for (size_t i = 0; i < itsColumnSpecification->size(); i++) {

            Column *col = itsColumnSpecification->pCol(i);
            string type = col->type();

            // A hack as the VOField currently tries to change a few
            // column names & we don't want it to.
            col->setType("IGNORETHIS");
            duchamp::VOField field(*col);
            col->setType(type);

            if (col->type() == "RAJD")  field.setUCD(lngUCDbase + ";meta.main");
            if (col->type() == "DECJD") field.setUCD(latUCDbase + ";meta.main");
            itsFileStream << "      ";
            field.printField(itsFileStream);

        }

        itsFileStream << "      <DATA>\n"
                      << "        <TABLEDATA>\n";


    }
}

void AskapVOTableCatalogueWriter::writeFrequencyParam()
{
    double ra, dec, freq;
    int spec = itsHead->WCS().spec;
    if (spec >= 0) {
        // if there is a spectral axis, write the frequency of the image

        // get the RA/DEC/FREQ values for the centre of the 1st channel
        itsHead->pixToWCS(itsCubeDim[0] / 2., itsCubeDim[1] / 2., 0.,
                          ra, dec, freq);
        std::string frequnits(itsHead->WCS().cunit[spec]);
        duchamp::VOParam freqParam("Reference frequency", "em.freq;meta.main", "float",
                                   freq, 0, frequnits);
        this->writeParameter(freqParam);
    }
}


template <class T>
void AskapVOTableCatalogueWriter::writeEntries(std::vector<T> &objlist)
{
    if (itsOpenFlag) {
        for (size_t i = 0; i < objlist.size(); i++) {
            // this->writeSource(objlist[i]);
            this->writeEntry<T>(objlist[i]);
        }
    }
}
template void
AskapVOTableCatalogueWriter::writeEntries<CasdaIsland>(std::vector<CasdaIsland>
        &objlist);
template void
AskapVOTableCatalogueWriter::writeEntries<CasdaComponent>(std::vector<CasdaComponent>
        &objlist);

template <class T>
void AskapVOTableCatalogueWriter::writeEntry(T &obj)
{
    if (itsOpenFlag) {
        itsFileStream.setf(std::ios::fixed);
        itsFileStream << "        <TR>\n";
        itsFileStream << "          ";
        for (size_t i = 0; i < itsColumnSpecification->size(); i++) {
            Column col = itsColumnSpecification->column(i);
            itsFileStream << "<TD>";
            obj.printTableEntry(itsFileStream, col);
            itsFileStream << "</TD>";
        }
        itsFileStream << "\n";
        itsFileStream << "        </TR>\n";

    }
}
template void AskapVOTableCatalogueWriter::writeEntry<CasdaIsland>(CasdaIsland &obj);
template void AskapVOTableCatalogueWriter::writeEntry<CasdaComponent>(CasdaComponent &obj);


}

}
