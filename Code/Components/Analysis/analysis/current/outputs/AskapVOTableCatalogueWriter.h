/// @file AskapVOTableCatalogueWriter.h
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

#ifndef ASKAP_VOT_CAT_WRITER_H_
#define ASKAP_VOT_CAT_WRITER_H_

#include <duchamp/Outputs/VOTableCatalogueWriter.hh>
#include <parallelanalysis/DuchampParallel.h>
#include <sourcefitting/RadioSource.h>

namespace askap {

namespace analysis {

using sourcefitting::RadioSource;

/// A class to handle writing of VOTables, adapted for use with
/// ASKAP/Selavy catalogues. This builds on the Duchamp library,
/// adding interfaces to the various Catalogue objects. This offers
/// the options of either writing out a catalogue of components, or a
/// catalogue of the islands from which they come.
class AskapVOTableCatalogueWriter : public duchamp::VOTableCatalogueWriter {
    public:
        AskapVOTableCatalogueWriter();
        AskapVOTableCatalogueWriter(std::string name);
        virtual ~AskapVOTableCatalogueWriter() {};

        /// Writes out the header information for each column, making
        /// appropriate WCS substitutions for columns that need it
        /// (RA, DEC, VEL etc)
        void writeTableHeader();

        /// Writes a VOPARAM to the header of the VOTable indicating
        /// the frequency that the image was observed at.
        void writeFrequencyParam();

        /// Modified, templated version of writeEntries that takes as
        /// an argument a list of things to be written to the
        /// VOTable. These are passed individually to writeEntry, and
        /// need to have a printTableEntry method.
        template <class T> void writeEntries(std::vector<T> &objlist);

        /// Modified, templated version of writeEntry that will write
        /// a single object to the VOTable. This object needs to have
        /// a printTableEntry method.
        template <class T> void writeEntry(T &obj);


    protected:

}; // end 'class AskapVOTableCatalogueWriter'

}

}

#endif
