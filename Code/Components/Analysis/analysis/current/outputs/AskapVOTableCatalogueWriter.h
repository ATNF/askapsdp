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

    class AskapVOTableCatalogueWriter : public duchamp::VOTableCatalogueWriter
    {
    public:
      AskapVOTableCatalogueWriter();
      AskapVOTableCatalogueWriter(std::string name);
      AskapVOTableCatalogueWriter(const AskapVOTableCatalogueWriter& other);
      AskapVOTableCatalogueWriter& operator= (const AskapVOTableCatalogueWriter& other);
      virtual ~AskapVOTableCatalogueWriter(){};

      bool writeFits(){return itsFlagWriteFits;};
      void setFlagWriteFits(bool b){itsFlagWriteFits = b;};
      std::vector<sourcefitting::RadioSource> *sourcelist(){return itsSourceList;};
      void setSourceList(std::vector<sourcefitting::RadioSource> *srclist){itsSourceList = srclist;};
      std::string fitType(){return itsFitType;};
      void setFitType(std::string s){itsFitType = s;};

      void setup(DuchampParallel *finder);
      void writeTableHeader();
      void writeEntries();
      using duchamp::VOTableCatalogueWriter::writeEntry;
      void writeEntry(sourcefitting::RadioSource *source);
      
    protected:
      bool itsFlagWriteFits; ///< Do we write the information on the fits to each source?
      std::vector<sourcefitting::RadioSource> *itsSourceList;
      std::string itsFitType; ///< Which fit type to write out.
    };

  }

}

#endif
