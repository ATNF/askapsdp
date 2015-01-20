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
    this->itsSourceList = 0;
    this->itsFitType = "best";
    this->itsEntryType = COMPONENT;
}

AskapVOTableCatalogueWriter::AskapVOTableCatalogueWriter(string name):
    duchamp::VOTableCatalogueWriter(name)
{
    this->itsSourceList = 0;
    this->itsFitType = "best";
    this->itsEntryType = COMPONENT;
}

AskapVOTableCatalogueWriter::AskapVOTableCatalogueWriter(const AskapVOTableCatalogueWriter& other)
{
    this->operator=(other);
}

AskapVOTableCatalogueWriter& AskapVOTableCatalogueWriter::operator= (const AskapVOTableCatalogueWriter& other)
{
    if (this == &other) return *this;
    ((VOTableCatalogueWriter &) *this) = other;
    this->itsSourceList = other.itsSourceList;
    this->itsFitType = other.itsFitType;
    this->itsEntryType = other.itsEntryType;
    return *this;
}

// void AskapVOTableCatalogueWriter::setup(DuchampParallel *finder)
// {
//     this->CatalogueWriter::setup(finder->pCube());
//     this->itsSourceList = finder->pSourceList();
// }

void AskapVOTableCatalogueWriter::writeEntries()
{
    if (this->itsOpenFlag) {
        for (std::vector<RadioSource>::iterator src = this->itsSourceList->begin();
                src < this->itsSourceList->end(); src++)
            this->writeEntry(&*src);
    }
}

void AskapVOTableCatalogueWriter::writeTableHeader()
{
    if (this->itsOpenFlag) {

        std::map<string, string> posUCDmap;
        posUCDmap.insert(pair<string, string>("ra", "pos.eq.ra"));
        posUCDmap.insert(pair<string, string>("ra_deg_cont", "pos.eq.ra"));
        posUCDmap.insert(pair<string, string>("dec", "pos.eq.dec"));
        posUCDmap.insert(pair<string, string>("dec_deg_cont", "pos.eq.dec"));
        posUCDmap.insert(pair<string, string>("glon", "pos.galactic.lng"));
        posUCDmap.insert(pair<string, string>("glat", "pos.galactic.lat"));
        Column &raCol = this->itsColumnSpecification->column("RAJD");
        string lngUCDbase = posUCDmap[makelower(raCol.getName())];
        Column &decCol = this->itsColumnSpecification->column("DECJD");
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
        Column &velCol = this->itsColumnSpecification->column("VEL");
        string specUCDbase = specUCDmap[makelower(velCol.getName())];

        for (size_t i = 0; i < this->itsColumnSpecification->size(); i++) {

            Column *col = this->itsColumnSpecification->pCol(i);
            string type = col->type();

            // A hack as the VOField currently tries to change a few
            // column names & we don't want it to.
            col->setType("IGNORETHIS");
            duchamp::VOField field(*col);
            col->setType(type);

            if (col->type() == "RAJD")  field.setUCD(lngUCDbase + ";meta.main");
            // if(col->type()=="WRA")   field.setUCD("phys.angSize;"+lngUCDbase);
            if (col->type() == "DECJD") field.setUCD(latUCDbase + ";meta.main");
            // if(col->type()=="WDEC")  field.setUCD("phys.angSize;"+latUCDbase);
            // if(col->type()=="VEL")   field.setUCD(specUCDbase+";meta.main");
            // if(col->type()=="W20")   field.setUCD("spect.line.width;"+specUCDbase);
            // if(col->type()=="W50")   field.setUCD("spect.line.width;"+specUCDbase);
            // if(col->type()=="WVEL")  field.setUCD("spect.line.width;"+specUCDbase);
            this->itsFileStream << "      ";
            field.printField(this->itsFileStream);

        }

        this->itsFileStream << "      <DATA>\n"
                            << "        <TABLEDATA>\n";


    }
}

void AskapVOTableCatalogueWriter::writeEntry(RadioSource *source)
{
    if (this->itsOpenFlag) {
        this->itsFileStream.setf(std::ios::fixed);

        if (this->itsEntryType == COMPONENT) {

            // write out an entry for all fits
            for (size_t f = 0; f < source->numFits(this->itsFitType); f++) {
                this->itsFileStream << "        <TR>\n";
                this->itsFileStream << "          ";
                for (size_t i = 0; i < this->itsColumnSpecification->size(); i++) {
                    Column col = this->itsColumnSpecification->column(i);
                    this->itsFileStream << "<TD>";
                    source->printTableEntry(this->itsFileStream, col,
                                            f, this->itsFitType);
                    this->itsFileStream << "</TD>";
                }
                this->itsFileStream << "\n";
                this->itsFileStream << "        </TR>\n";
            }

        } else {

            this->itsFileStream << "        <TR>\n";
            this->itsFileStream << "          ";
            for (size_t i = 0; i < this->itsColumnSpecification->size(); i++) {
                Column col = this->itsColumnSpecification->column(i);
                this->itsFileStream << "<TD>";
                if (col.type() == "NCOMP") {
                    // n_components printing not defined elsewhere
                    col.printEntry(this->itsFileStream,
                                   source->numFits(this->itsFitType));
                } else if (col.type() == "NUM") {
                    // ensure we print the island ID, not the 1st component ID
                    col.printEntry(this->itsFileStream, source->getID());
                } else {
                    // use Duchamp library to print all other columns
                    source->duchamp::Detection::printTableEntry(this->itsFileStream, col);
                }
                this->itsFileStream << "</TD>";
            }
            this->itsFileStream << "\n";
            this->itsFileStream << "        </TR>\n";


        }
    }
}

}

}
