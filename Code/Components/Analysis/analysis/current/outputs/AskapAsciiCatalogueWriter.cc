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

#include <duchamp/Outputs/ASCIICatalogueWriter.hh>

///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".askapasciicatwriter");

namespace askap { 

  namespace analysis {

    AskapAsciiCatalogueWriter::AskapAsciiCatalogueWriter():
      duchamp::ASCIICatalogueWriter()
    {
      this->itsFlagWriteFits = true;
      this->itsSourceList = 0;
      this->itsFitType = "best";
    }

    AskapAsciiCatalogueWriter::AskapAsciiCatalogueWriter(std::string name):
      duchamp::ASCIICatalogueWriter(name)
    {
	this->itsDestination = duchamp::Catalogues::FILE;
      this->itsFlagWriteFits = true;
      this->itsSourceList = 0;
      this->itsFitType = "best";
    }

    AskapAsciiCatalogueWriter::AskapAsciiCatalogueWriter(duchamp::Catalogues::DESTINATION dest):
      duchamp::ASCIICatalogueWriter(dest)
    {
      this->itsFlagWriteFits = true;
      this->itsSourceList = 0;
      this->itsFitType = "best";
    }

    AskapAsciiCatalogueWriter::AskapAsciiCatalogueWriter(std::string name,duchamp::Catalogues::DESTINATION dest):
      duchamp::ASCIICatalogueWriter(name,dest)
    {
      this->itsFlagWriteFits = true;
      this->itsSourceList = 0;
      this->itsFitType = "best";
    }

    AskapAsciiCatalogueWriter::AskapAsciiCatalogueWriter(const AskapAsciiCatalogueWriter& other)
    {
      this->operator=(other);
    }

    AskapAsciiCatalogueWriter& AskapAsciiCatalogueWriter::operator= (const AskapAsciiCatalogueWriter& other)
    {
      if(this==&other) return *this;
      ((ASCIICatalogueWriter &) *this) = other;
      this->itsFlagWriteFits = other.itsFlagWriteFits;
      this->itsSourceList = other.itsSourceList;
      this->itsFitType = other.itsFitType;
      return *this;
    }
    
    void AskapAsciiCatalogueWriter::writeEntries()
    {
      if(this->itsFlagWriteFits){
	if(this->itsOpenFlag){
	  for(std::vector<sourcefitting::RadioSource>::iterator src=this->itsSourceList->begin(); 
	      src<this->itsSourceList->end(); src++)
	    this->writeEntry(&*src);
	}
      }
      else this->CatalogueWriter::writeEntries();
      
    }    

    void AskapAsciiCatalogueWriter::writeTableHeader()
    {
      if(this->itsOpenFlag){
	*this->itsStream << "#";
	// for(size_t i=0;i<this->itsColumnSpecification->size();i++)
	//   this->itsColumnSpecification->column(i).printDash(*this->itsStream);
	// *this->itsStream << "\n#";
	for(size_t i=0;i<this->itsColumnSpecification->size();i++)
	  this->itsColumnSpecification->column(i).printTitle(*this->itsStream);
	*this->itsStream << "\n#";
	for(size_t i=0;i<this->itsColumnSpecification->size();i++)
	  this->itsColumnSpecification->column(i).printUnits(*this->itsStream);
	// *this->itsStream << "\n#";
	// for(size_t i=0;i<this->itsColumnSpecification->size();i++)
	//   this->itsColumnSpecification->column(i).printDash(*this->itsStream);
	*this->itsStream << "\n";
      }
    }

    void AskapAsciiCatalogueWriter::writeEntry(sourcefitting::RadioSource *source)
    {
      if(this->itsOpenFlag){
	for(size_t i=0;i<source->numFits(this->itsFitType);i++) {
	  *this->itsStream << " "; // to match the '#' at the start of the header rows
	  source->printTableRow(*this->itsStream,*this->itsColumnSpecification,i,this->itsFitType);
	}
      }
    }

  }
  
}
