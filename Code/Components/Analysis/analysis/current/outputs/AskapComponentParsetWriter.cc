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

#include <outputs/ParsetComponent.h>
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
	    this->itsMaxNumComponents = other.itsMaxNumComponents;
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
	    /// @details Write out the compoennt list to the
	    /// parset. We may only want to write out a certain number
	    /// of components, starting with the brightest, so we need
	    /// to make a first pass over the source list. We sort the
	    /// components by their total flux, and then work our way
	    /// down, printing out their parset details.

	    if(this->itsOpenFlag){

		std::multimap <float, ParsetComponent> componentList;
		std::multimap <float, ParsetComponent>::reverse_iterator cmpntIter;
		ParsetComponent cmpnt;
		cmpnt.setHeader(this->itsHead);
		cmpnt.setReference(this->itsRefRA,this->itsRefDec);
		cmpnt.setSizeFlag(this->itsFlagReportSize);

		// First iterate over all components, storing them in a multimap indexed by their flux.
		for(std::vector<sourcefitting::RadioSource>::iterator src=this->itsSourceList->begin(); src<this->itsSourceList->end(); src++){
		    std::vector<casa::Gaussian2D<Double> > fitset=src->gaussFitSet(this->itsFitType);
		    for(size_t i=0;i<fitset.size();i++){
			cmpnt.defineComponent(&*src,i,this->itsFitType);
			componentList.insert(std::pair<float,ParsetComponent>(cmpnt.flux(),cmpnt));
		    }
		}

		int count=0;
		// only do this many components. If negative, do them all.
		int maxCount = (this->itsMaxNumComponents>0) ? this->itsMaxNumComponents : componentList.size();
		std::stringstream idlist;
		idlist << itsSourceIDlist;

		// Work down the list, starting at the brightest
		// component, writing out the parset details to the
		// file and keeping track of the list of source IDs
		for(cmpntIter=componentList.rbegin(); cmpntIter!=componentList.rend() && count<maxCount;cmpntIter++,count++) {
		    *this->itsStream << cmpntIter->second;
		    // update source ID list
		    if(itsSourceIDlist.size()>0) idlist<<",";
		    idlist<<"src"<<cmpntIter->second.ID();
		    this->itsSourceIDlist = idlist.str();
		}

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
