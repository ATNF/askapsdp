/// @file AskapComponentParsetWriter.cc
///
/// XXX Notes on program XXX
///
/// @copyright (c) 2011 CSIRO
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
///
/// @author Matthew Whiting <Matthew.Whiting@csiro.au>
///
#include <outputs/AskapComponentParsetWriter.h>
#include <askap_analysis.h>
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <sourcefitting/RadioSource.h>
#include <analysisutilities/Analysisutilities.h>

#include <duchamp/Outputs/ASCIICatalogueWriter.hh>

#include <scimath/Functionals/Gaussian2D.h>

///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".askapcomponentparsetwriter");

namespace askap {

    namespace analysis { 

	AskapComponentParsetWriter::AskapComponentParsetWriter():
	    duchamp::ASCIICatalogueWriter(), itsSourceList(0), itsFitType("best"), itsRefRA(0.), itsRefDec(0.), itsFlagReportSize(false), itsSourceIDlist("")
	{
	    this->itsOpenFlag=false;
	    this->itsDestination=duchamp::Catalogues::FILE;
	}

	AskapComponentParsetWriter::AskapComponentParsetWriter(std::string name):
	    duchamp::ASCIICatalogueWriter(name), itsSourceList(0), itsFitType("best"), itsRefRA(0.), itsRefDec(0.), itsFlagReportSize(false), itsSourceIDlist("")
	{
	    this->itsOpenFlag=false;
	    this->itsDestination=duchamp::Catalogues::FILE;
	}

	AskapComponentParsetWriter::AskapComponentParsetWriter(duchamp::Catalogues::DESTINATION dest):
	    duchamp::ASCIICatalogueWriter(dest), itsSourceList(0), itsFitType("best"), itsRefRA(0.), itsRefDec(0.), itsFlagReportSize(false), itsSourceIDlist("")
	{
	    this->itsOpenFlag=false;
	}

	AskapComponentParsetWriter::AskapComponentParsetWriter(std::string name, duchamp::Catalogues::DESTINATION dest):
	    duchamp::ASCIICatalogueWriter(name, dest), itsSourceList(0), itsFitType("best"), itsRefRA(0.), itsRefDec(0.), itsFlagReportSize(false), itsSourceIDlist("")
	{
	    this->itsOpenFlag=false;
	}

	AskapComponentParsetWriter::AskapComponentParsetWriter(const AskapComponentParsetWriter& other)
	{
	    this->operator=(other);
	}

	AskapComponentParsetWriter& AskapComponentParsetWriter::operator= (const AskapComponentParsetWriter& other)
	{
	    if(this==&other) return *this;
	    ((ASCIICatalogueWriter &) *this) = other;
	    this->itsSourceList = other.itsSourceList;
	    this->itsFitType = other.itsFitType;
	    this->itsRefRA = other.itsRefRA;
	    this->itsRefDec = other.itsRefDec;
	    this->itsFlagReportSize = other.itsFlagReportSize;
	    this->itsSourceIDlist = other.itsSourceIDlist;
	    return *this;
	}

	void AskapComponentParsetWriter::setup(DuchampParallel *finder)
	{
	    // only thing to do here is get the centre position of the
	    // image so that we can calculate relative offsets for
	    // each of the components, and get the RadioSource list.
	    
	    this->CatalogueWriter::setup(finder->pCube());
	    this->itsRefRA =  this->itsHead->getWCS()->crval[0];
	    this->itsRefDec = this->itsHead->getWCS()->crval[1];

	}

	void AskapComponentParsetWriter::writeTableHeader()
	{
	    if(this->itsOpenFlag){
		*this->itsStream << "sources.names = field1\n";
		std::string raRef = decToDMS(this->itsRefRA,"RA",4,"parset");
		std::string decRef= decToDMS(this->itsRefDec,"DEC",3,"parset");
		*this->itsStream << "sources.field1.direction = ["<<raRef<<", "<<decRef<<", J2000]\n";
	    }
	}

	void AskapComponentParsetWriter::writeEntries()
	{
	    if(this->itsOpenFlag){
		for(std::vector<sourcefitting::RadioSource>::iterator src=this->itsSourceList->begin(); 
		    src<this->itsSourceList->end(); src++)
		    this->writeEntry(&*src);
	    }
	    
	}

	void AskapComponentParsetWriter::writeEntry(sourcefitting::RadioSource *source)
	{
	    if(this->itsOpenFlag){
		std::vector<casa::Gaussian2D<Double> > fitset=source->gaussFitSet(this->itsFitType);
		std::vector<casa::Gaussian2D<Double> >::iterator fit;
		double cosdec=cos(this->itsRefDec*M_PI/180.);
		std::stringstream idlist;
		idlist << itsSourceIDlist;
		for(size_t i=0;i<fitset.size();i++){
		    double thisRA,thisDec,zworld;
		    this->itsHead->pixToWCS(fitset[i].xCenter(),fitset[i].yCenter(),source->getZcentre(),thisRA,thisDec,zworld);
		    float decOffset = (thisDec-this->itsRefDec)*M_PI/180.;
		    float raOffset = (thisRA-this->itsRefRA)*M_PI/180. * cosdec;
		    float intfluxfit = fitset[i].flux();
		    if (this->itsHead->needBeamSize())
			intfluxfit /= this->itsHead->beam().area(); // Convert from Jy/beam to Jy
		    std::stringstream prefix,ID;
		    ID << source->getID() << sourcefitting::getSuffix(i);
		    prefix << "sources.src"<< ID.str();
		    *this->itsStream << prefix.str() << ".flux.i        = " << intfluxfit <<"\n";
		    *this->itsStream << prefix.str() << ".direction.ra  = " << raOffset << "\n";
		    *this->itsStream << prefix.str() << ".direction.dec = " << decOffset << "\n";
		    if(this->itsFlagReportSize){
			*this->itsStream << prefix.str() << ".shape.bmaj  = " << fitset[i].majorAxis()*this->itsHead->getAvPixScale()*3600. << "\n";
			*this->itsStream << prefix.str() << ".shape.bmin  = " << fitset[i].minorAxis()*this->itsHead->getAvPixScale()*3600. << "\n";
			*this->itsStream << prefix.str() << ".shape.bpa   = " << fitset[i].PA()*180. / M_PI << "\n";
		    }
		    else{
			*this->itsStream << prefix.str() << ".shape.bmaj  = 0.\n";
			*this->itsStream << prefix.str() << ".shape.bmin  = 0.\n";
			*this->itsStream << prefix.str() << ".shape.bpa   = 0.\n";
		    }
		    
		    // update source ID list
		    if(itsSourceIDlist.size()>0) idlist<<",";
		    idlist<<"src"<<ID.str();
		    
		}
		this->itsSourceIDlist = idlist.str();

	    }
	}

 	void AskapComponentParsetWriter::writeFooter()
	{
	    if(this->itsOpenFlag){
		*this->itsStream <<  "sources.field1.components = [" << this->itsSourceIDlist << "]\n";
	    }
	}

    }

}
