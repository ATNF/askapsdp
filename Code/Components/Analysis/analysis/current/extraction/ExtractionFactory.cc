/// @file ExtractionFactory.cc
///
/// Front end handler to deal with all the different types of spectrum/image/cube extraction
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
/// @author XXX XXX <XXX.XXX@csiro.au>
///
#include <extraction/ExtractionFactory.h>
#include <askap_analysis.h>
#include <extraction/SourceSpectrumExtractor.h>
#include <extraction/NoiseSpectrumExtractor.h>
#include <extraction/MomentMapExtractor.h>
#include <extraction/CubeletExtractor.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

//System includes
#include <vector>

//ASKAP includes
#include <askapparallel/AskapParallel.h>
#include <sourcefitting/RadioSource.h>

//3rd-party includes
#include <Common/ParameterSet.h>
using namespace LOFAR::TYPES;
#include <Blob/BlobString.h>
#include <Blob/BlobIBufString.h>
#include <Blob/BlobOBufString.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>

#include <duchamp/param.hh>

ASKAP_LOGGER(logger, ".extractionfactory");

namespace askap {

    namespace analysis {

	ExtractionFactory::ExtractionFactory(askap::askapparallel::AskapParallel& comms, const LOFAR::ParameterSet& parset):
	    itsComms(comms), itsParset(parset)
	{
	    this->itsParam = 0;
	    this->itsSourceList = std::vector<sourcefitting::RadioSource>();
	    this->itsObjectChoice = std::vector<bool>();
	}

	void ExtractionFactory::distribute()
	{
	    
	    if(this->itsComms.isMaster()) {
		if(this->itsComms.isParallel()){
		    int16 rank;
		    LOFAR::BlobString bs;
	      
		    // now send the individual sources to each worker in turn
		    for(size_t i=0;i<this->itsSourceList.size()+itsComms.nProcs()-1;i++){
			rank = i % (itsComms.nProcs() - 1);
			bs.resize(0);
			LOFAR::BlobOBufString bob(bs);
			LOFAR::BlobOStream out(bob);
			out.putStart("extsrc", 1);
			// the first time we write to each worker, send the total number of sources
			if(i/(itsComms.nProcs()-1)==0) out << (unsigned int)(this->itsSourceList.size());
			out << (i<this->itsSourceList.size());
			if(i<this->itsSourceList.size()){
			    // this->itsSourceList[i].defineBox(this->itsParam->section(), this->itsFitParams, this->itsCube.header().getWCS()->spec);
			    out << this->itsSourceList[i];
			}
			out.putEnd();
			itsComms.sendBlob(bs, rank + 1);
		    }
	      
		}
	    }
	  
	    if(this->itsComms.isWorker()){
	    
		unsigned int totalSourceCount=0;
		if(this->itsComms.isParallel()){
	      
		    LOFAR::BlobString bs;
		    // now read individual sources
		    bool isOK=true;
		    this->itsSourceList.clear();
		    while(isOK) {	    
			sourcefitting::RadioSource src;
			itsComms.receiveBlob(bs, 0);
			LOFAR::BlobIBufString bib(bs);
			LOFAR::BlobIStream in(bib);
			int version = in.getStart("extsrc");
			ASKAPASSERT(version == 1);
			if(totalSourceCount == 0) in >> totalSourceCount;
			in >> isOK;
			if(isOK){
			    in >> src;
			    this->itsSourceList.push_back(src);
			}
			in.getEnd();
		    }
	      
		}
		else totalSourceCount = (unsigned int)(this->itsSourceList.size());
	    
		this->itsObjectChoice = this->itsParam->getObjectChoices(totalSourceCount);
	    }
	}

	void ExtractionFactory::extract()
	{

	    if(this->itsObjectChoice.size() != this->itsSourceList.size()){
		ASKAPLOG_ERROR_STR(logger, "Extraction - object choice and source list vectors do not match. No extraction performed.");
	    }
	    else{

		const unsigned int numTypes = 4;

		std::string parsetNames[numTypes]={"Spectra","NoiseSpectra","MomentMap","Cubelet"};

		for(unsigned int type=0;type<numTypes;type++){

		    std::string parameter="extract"+parsetNames[type];
		    bool flag=this->itsParset.getBool(parameter,false);
		    if(flag){
			std::vector<sourcefitting::RadioSource>::iterator src;
			LOFAR::ParameterSet extractSubset=this->itsParset.makeSubset(parameter+".");
			ASKAPLOG_INFO_STR(logger, "Beginnging " << parsetNames[type] << " extraction for " << this->itsSourceList.size() << " sources");
			for (src = this->itsSourceList.begin(); src < this->itsSourceList.end(); src++) {
			    if(itsObjectChoice.at(src->getID()-1)){
				SourceDataExtractor *extractor;
				switch(type){
				case 0: extractor = new SourceSpectrumExtractor(extractSubset); break;
				case 1: extractor = new NoiseSpectrumExtractor(extractSubset); break;
				case 2: extractor = new MomentMapExtractor(extractSubset); break;
				case 3: extractor = new CubeletExtractor(extractSubset); break;
				default: ASKAPTHROW(AskapError, "ExtractionFactory - unknown extraction type : " << type); break;
				}

				extractor->setSource(&*src);
				extractor->extract();
				extractor->writeImage();

				delete extractor;
			    }
			}
		    }
		
		}

	    }

	}


    }

}
