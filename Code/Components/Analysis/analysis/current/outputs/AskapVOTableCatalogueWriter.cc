#include <outputs/AskapVOTableCatalogueWriter.h>
#include <askap_analysis.h>

#include <parallelanalysis/DuchampParallel.h>
#include <sourcefitting/RadioSource.h>
#include <sourcefitting/FitResults.h>

#include <duchamp/Outputs/VOTableCatalogueWriter.hh>

namespace askap { 

  namespace analysis {

    AskapVOTableCatalogueWriter::AskapVOTableCatalogueWriter():
      duchamp::VOTableCatalogueWriter()
    {
      this->itsFlagWriteFits = true;
      this->itsSourceList = 0;
      this->itsFitType = "best";
    }

    AskapVOTableCatalogueWriter::AskapVOTableCatalogueWriter(std::string name):
      duchamp::VOTableCatalogueWriter(name)
    {
      this->itsFlagWriteFits = true;
      this->itsSourceList = 0;
      this->itsFitType = "best";
    }

    AskapVOTableCatalogueWriter::AskapVOTableCatalogueWriter(const AskapVOTableCatalogueWriter& other)
    {
      this->operator=(other);
    }

    AskapVOTableCatalogueWriter& AskapVOTableCatalogueWriter::operator= (const AskapVOTableCatalogueWriter& other)
    {
      if(this==&other) return *this;
      ((VOTableCatalogueWriter &) *this) = other;
      this->itsFlagWriteFits = other.itsFlagWriteFits;
      this->itsSourceList = other.itsSourceList;
      this->itsFitType = other.itsFitType;
      return *this;
    }

    void AskapVOTableCatalogueWriter::setup(DuchampParallel *finder)
    {
      this->CatalogueWriter::setup(finder->pCube());
      this->itsSourceList = finder->pSourceList();
    }
    
    void AskapVOTableCatalogueWriter::writeEntries()
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

    void AskapVOTableCatalogueWriter::writeTableHeader()
    {
      if(this->itsOpenFlag){
	this->VOTableCatalogueWriter::writeTableHeader();
      }
    }

    void AskapVOTableCatalogueWriter::writeEntry(sourcefitting::RadioSource *source)
    {
      if(this->itsOpenFlag){
	this->itsFileStream.setf(std::ios::fixed);  
	
	for(size_t f=0;f<source->numFits();f++){
	  this->itsFileStream<<"        <TR>\n";
	  this->itsFileStream<<"          ";
	  for(size_t i=0;i<this->itsColumnSpecification->size();i++){
	    this->itsFileStream<<"<TD>";
	    source->printTableEntry(this->itsFileStream, this->itsColumnSpecification->column(i),f,this->itsFitType);
	    this->itsFileStream<<"</TD>";
	  }
	  this->itsFileStream<<"\n";
	  this->itsFileStream<<"        </TR>\n";
	}
      }
    }

  }
  
}
