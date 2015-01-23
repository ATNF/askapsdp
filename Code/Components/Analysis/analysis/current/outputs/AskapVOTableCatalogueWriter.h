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

/// Enumeration to distinguish between the options of writing
/// out an Island catalogue or a component catalogue.
enum ENTRYTYPE { ISLAND, COMPONENT};

/// A class to handle writing of VOTables, adapted for use with
/// ASKAP/Selavy catalogues. This builds on the Duchamp library,
/// adding interfaces to the RadioSource objects and, from
/// there, the fitted components. This offers the options of
/// either writing out a catalogue of components, or a catalogue
/// of the islands from which they come (these are the duchamp
/// Detection objects, but a differently-formatted catalogue to
/// that provided by Duchamp).

class AskapVOTableCatalogueWriter : public duchamp::VOTableCatalogueWriter {
    public:
        AskapVOTableCatalogueWriter();
        AskapVOTableCatalogueWriter(std::string name);
        AskapVOTableCatalogueWriter(const AskapVOTableCatalogueWriter& other);
        AskapVOTableCatalogueWriter& operator= (
            const AskapVOTableCatalogueWriter& other);
        virtual ~AskapVOTableCatalogueWriter() {};

        std::vector<RadioSource> *srclist() {return itsSourceList;};
        void setSourceList(std::vector<RadioSource> *cmplist)
        {
            itsSourceList = cmplist;
        };
        std::string fitType() {return itsFitType;};
        void setFitType(std::string s) {itsFitType = s;};
        ENTRYTYPE entryType() {return itsEntryType;};
        void setEntryType(ENTRYTYPE type) {itsEntryType = type;};

        // /// Initialise with a given DuchampParallel, taking the
        // /// duchamp::Cube and the source list
        // void setup(DuchampParallel *finder);

        /// Writes out the header information for each column, making
        /// appropriate WCS substitutions for columns that need it
        /// (RA, DEC, VEL etc)
        void writeTableHeader();

        /// Loops over all sources in itsSourceList, writing them out
        /// individually.
        void writeEntries();

        // Declare that we are using the writeEntry from
        // VOTableCatalogueWriter as well.
        using duchamp::VOTableCatalogueWriter::writeEntry;

        /// Modified version of writeEntry to take a RadioSource
        /// source and treat it either as an island (if
        /// itsEntryType==ISLAND) or as a collection of one or more
        /// components (itsEntryType==COMPONENT).
        void writeEntry(RadioSource &source);

    protected:
        std::vector<RadioSource> *itsSourceList;
        std::string itsFitType; ///< Which fit type to write out.
        ENTRYTYPE itsEntryType;

}; // end 'class AskapVOTableCatalogueWriter'

}

}

#endif
