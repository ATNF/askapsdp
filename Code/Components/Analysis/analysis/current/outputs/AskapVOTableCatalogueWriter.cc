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
	
	std::map<std::string,std::string> posUCDmap;
	posUCDmap.insert(std::pair<std::string,std::string>("ra","pos.eq.ra"));
	posUCDmap.insert(std::pair<std::string,std::string>("dec","pos.eq.dec"));
	posUCDmap.insert(std::pair<std::string,std::string>("glon","pos.galactic.lng"));
	posUCDmap.insert(std::pair<std::string,std::string>("glat","pos.galactic.lat"));
	duchamp::Catalogues::Column &raCol=this->itsColumnSpecification->column("RAJD");
	std::string lngUCDbase = posUCDmap[makelower(raCol.getName())];
	duchamp::Catalogues::Column &decCol=this->itsColumnSpecification->column("DECJD");
	std::string latUCDbase = posUCDmap[makelower(decCol.getName())];
	
	std::map<std::string,std::string> specUCDmap;
	specUCDmap.insert(std::pair<std::string,std::string>("VELO","phys.veloc;spect.dopplerVeloc"));
	specUCDmap.insert(std::pair<std::string,std::string>("VOPT","phys.veloc;spect.dopplerVeloc.opt"));
	specUCDmap.insert(std::pair<std::string,std::string>("VRAD","phys.veloc;spect.dopplerVeloc.rad"));
	specUCDmap.insert(std::pair<std::string,std::string>("FREQ","em.freq"));
	specUCDmap.insert(std::pair<std::string,std::string>("ENER","em.energy"));
	specUCDmap.insert(std::pair<std::string,std::string>("WAVN","em.wavenumber"));
	specUCDmap.insert(std::pair<std::string,std::string>("WAVE","em.wl"));
	specUCDmap.insert(std::pair<std::string,std::string>("AWAV","em.wl"));
	specUCDmap.insert(std::pair<std::string,std::string>("ZOPT","src.redshift"));
	specUCDmap.insert(std::pair<std::string,std::string>("BETA","src.redshift; spect.dopplerVeloc"));
	std::string specUCDbase = specUCDmap[this->itsColumnSpecification->column("VEL").getName()];
	
	for(size_t i=0;i<this->itsColumnSpecification->size();i++){
	  
	  duchamp::Catalogues::Column *col = this->itsColumnSpecification->pCol(i);
	  duchamp::VOField field(*col); 
	  if(col->type()=="RAJD")  field.setUCD(lngUCDbase+";meta.main");
	  if(col->type()=="WRA")   field.setUCD("phys.angSize;"+lngUCDbase);
	  if(col->type()=="DECJD") field.setUCD(latUCDbase+";meta.main");
	  if(col->type()=="WDEC")  field.setUCD("phys.angSize;"+latUCDbase);	
	  if(col->type()=="VEL")   field.setUCD(specUCDbase+";meta.main");
	  if(col->type()=="W20")   field.setUCD("spect.line.width;"+specUCDbase);
	  if(col->type()=="W50")   field.setUCD("spect.line.width;"+specUCDbase);
	  if(col->type()=="WVEL")  field.setUCD("spect.line.width;"+specUCDbase);
	  this->itsFileStream << "      ";
	  field.printField(this->itsFileStream);
	
	}
	
	this->itsFileStream<<"      <DATA>\n"
			   <<"        <TABLEDATA>\n";


      }
    }

    void AskapVOTableCatalogueWriter::writeEntry(sourcefitting::RadioSource *source)
    {
      if(this->itsOpenFlag){
	this->itsFileStream.setf(std::ios::fixed);  
	
	for(size_t f=0;f<source->numFits(this->itsFitType);f++){
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
