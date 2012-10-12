#include <outputs/AskapAsciiCatalogueWriter.h>
#include <askap_analysis.h>

#include <parallelanalysis/DuchampParallel.h>
#include <sourcefitting/RadioSource.h>
#include <sourcefitting/FitResults.h>

#include <duchamp/Outputs/ASCIICatalogueWriter.hh>

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

    void AskapAsciiCatalogueWriter::setup(DuchampParallel *finder)
    {
      this->CatalogueWriter::setup(finder->pCube());
      this->itsSourceList = finder->pSourceList();
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
	for(size_t i=0;i<this->itsColumnSpecification->size();i++)
	  this->itsColumnSpecification->column(i).printDash(*this->itsStream);
	*this->itsStream << "\n#";
	for(size_t i=0;i<this->itsColumnSpecification->size();i++)
	  this->itsColumnSpecification->column(i).printTitle(*this->itsStream);
	*this->itsStream << "\n#";
	for(size_t i=0;i<this->itsColumnSpecification->size();i++)
	  this->itsColumnSpecification->column(i).printUnits(*this->itsStream);
	*this->itsStream << "\n#";
	for(size_t i=0;i<this->itsColumnSpecification->size();i++)
	  this->itsColumnSpecification->column(i).printDash(*this->itsStream);
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
