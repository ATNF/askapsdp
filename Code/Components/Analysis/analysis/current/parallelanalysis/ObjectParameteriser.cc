/// @file
///
/// Handle the parameterisation of objects that require reading from a file on disk
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
///
/// @author Matthew Whiting <Matthew.Whiting@csiro.au>
///
#include <parallelanalysis/ObjectParameteriser.h>

#include <askap_analysis.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <askapparallel/AskapParallel.h>
#include <duchamp/Detection/Detection.hh>

#include <parallelanalysis/DuchampParallel.h>

#include <Blob/BlobString.h>
#include <Blob/BlobIBufString.h>
#include <Blob/BlobOBufString.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>
using namespace LOFAR::TYPES;

///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".objectparam");

namespace askap {

    namespace analysis {

	ObjectParameteriser::ObjectParameteriser(askap::askapparallel::AskapParallel& comms):
	    itsComms(&comms), itsDP(),itsInputList(),itsOutputList()
	{
	}

	ObjectParameteriser::ObjectParameteriser(const ObjectParameteriser& other)
	{
	    this->operator=(other);
	}
	
	ObjectParameteriser& ObjectParameteriser::operator= (const ObjectParameteriser& other)
	{
	    if(this==&other) return *this;
	    this->itsComms = other.itsComms;
	    this->itsDP = other.itsDP;
	    this->itsInputList = other.itsInputList;
	    this->itsOutputList = other.itsOutputList;
	    return *this;
	}


	ObjectParameteriser::~ObjectParameteriser()
	{
	}

	void ObjectParameteriser::initialise(DuchampParallel *dp)
	{
	    this->itsDP=dp;
	    if(this->itsComms->isMaster()){
		for(std::vector<sourcefitting::RadioSource>::iterator src=this->itsDP->pEdgeList()->begin();
		    src!=this->itsDP->pEdgeList()->end(); src++){
		    itsInputList.push_back(*src);
		}
	    }
	    
	}

	void ObjectParameteriser::distribute()
	{
	    if(this->itsComms->isMaster()){
		// send objects in itsInputList to workers in round-robin fashion
		// broadcast 'finished' signal
		LOFAR::BlobString bs;
		for(size_t i=0;i<this->itsInputList.size();i++){
		    unsigned int rank = i % (this->itsComms->nProcs() - 1);
		    ASKAPLOG_DEBUG_STR(logger, "Sending source #"<<i+1<<" to worker "<<rank+1);
		    bs.resize(0);
		    LOFAR::BlobOBufString bob(bs);
		    LOFAR::BlobOStream out(bob);
		    out.putStart("OP", 1);
		    out << true << this->itsInputList[i];
		    out.putEnd();
		    this->itsComms->sendBlob(bs, rank + 1);
		}

		// now notify all workers that we're finished.
		LOFAR::BlobOBufString bob(bs);
		LOFAR::BlobOStream out(bob);
		bs.resize(0);
		bob = LOFAR::BlobOBufString(bs);
		out = LOFAR::BlobOStream(bob);
		ASKAPLOG_DEBUG_STR(logger, "Broadcasting 'finished' signal to all workers");
		out.putStart("OP", 1);
		out << false;
		out.putEnd();
		this->itsComms->broadcastBlob(bs,0);

	    }
	    else{
		// receive objects and put in itsInputList until receive 'finished' signal
		LOFAR::BlobString bs;
	      
		// now read individual sources
		bool isOK=true;
		this->itsInputList.clear();
		while(isOK) {	    
		    sourcefitting::RadioSource src;
		    this->itsComms->receiveBlob(bs, 0);
		    LOFAR::BlobIBufString bib(bs);
		    LOFAR::BlobIStream in(bib);
		    int version = in.getStart("OP");
		    ASKAPASSERT(version == 1);
		    in >> isOK;
		    if(isOK){
			in >> src;
			this->itsInputList.push_back(src);
		    }
		    in.getEnd();
		}

	    }
	}

	void ObjectParameteriser::parameterise()
	{
	    if(this->itsComms->isWorker()){
		// For each object, get the bounding subsection for that object
		// Define a DuchampParallel and use it to do the parameterisation
		// put parameterised objects into itsOutputList

		std::vector<size_t> dim(this->itsDP->cube().getDimArray(),this->itsDP->cube().getDimArray()+this->itsDP->cube().getNumDim());
		LOFAR::ParameterSet parset=this->itsDP->parset();
		parset.replace("flagsubsection","true");
		const int lng=this->itsDP->cube().pHeader()->getWCS()->lng;
		const int lat=this->itsDP->cube().pHeader()->getWCS()->lat;
		const int spec=this->itsDP->cube().pHeader()->getWCS()->spec;

		int padsize=0;
		for(size_t i=0;i<this->itsInputList.size();i++){

		    // get bounding subsection & transform into a Subsection string
		    std::vector<std::string> sectionlist(dim.size(),"1:1");
		    for(int ax=0;ax<dim.size();ax++){
			std::stringstream ss;
			if (ax==spec)
			    ss << "1:"<<dim[ax]+1;
			else if(ax==lng)
			    ss << std::max(1L,this->itsInputList[i].getXmin()-padsize+1)<<":"<<std::min(long(dim[ax]),this->itsInputList[i].getXmax()-+padsize+1);
			else if (ax==lat)
			    ss << std::max(1L,this->itsInputList[i].getYmin()-padsize+1)<<":"<<std::min(long(dim[ax]),this->itsInputList[i].getYmax()-+padsize+1);
			else
			    ss << "1:1";
			sectionlist[ax]=ss.str();
		    }
		    std::stringstream secstr;
		    secstr << "[ " << sectionlist[0];
		    for(size_t i=1;i<dim.size();i++) secstr << "," << sectionlist[i];
		    secstr << "]";
		    parset.replace("subsection",secstr.str());

		    // define a duchamp Cube using the filename from the this->itsDP->cube()
		    // set the subsection
		    DuchampParallel tempDP(*this->itsComms, parset);

		    // open the image
		    tempDP.readData();

		    // store the current object to the cube
		    tempDP.cube().addObject(this->itsInputList[i]);

		    // parameterise
		    tempDP.cube().calcObjectWCSparams();

		    sourcefitting::RadioSource src(tempDP.cube().getObject(0));

		    if(tempDP.fitParams()->doFit()) tempDP.fitSource(src,true);

		    // get the parameterised object and store to itsOutputList
		    this->itsOutputList.push_back(src);
		}

	    }

	}

	void ObjectParameteriser::gather()
	{
	    
	    if(this->itsComms->isMaster()){
		// for each worker, read completed objects until we get a 'finished' signal

		// now read back the sources from the workers
		this->itsDP->pEdgeList()->clear();
		LOFAR::BlobString bs;
		for (int n=0;n<this->itsComms->nProcs()-1;n++){
		    int numSrc;
		    ASKAPLOG_INFO_STR(logger, "Master about to read from worker #"<< n+1);
		    this->itsComms->receiveBlob(bs, n + 1);
		    LOFAR::BlobIBufString bib(bs);
		    LOFAR::BlobIStream in(bib);
		    int version = in.getStart("OPfinal");
		    ASKAPASSERT(version == 1);
		    in >> numSrc;
		    for(int i=0;i<numSrc;i++){
			sourcefitting::RadioSource src;
			in >> src;
			src.setHeader(this->itsDP->cube().pHeader());  // make sure we have the right WCS etc information
			this->itsDP->pEdgeList()->push_back(src);
		    }
		    in.getEnd();
		}

	    }
	    else{
		// for each object in itsOutputList, send to master
		ASKAPLOG_INFO_STR(logger, "Have parameterised " << this->itsInputList.size() << " edge sources. Returning results to master.");
		LOFAR::BlobString bs;
		bs.resize(0);
		LOFAR::BlobOBufString bob(bs);
		LOFAR::BlobOStream out(bob);
		out.putStart("final", 1);
		out << int(this->itsInputList.size());
		for(size_t i=0;i<this->itsInputList.size();i++) out << this->itsInputList[i];
		out.putEnd();
		this->itsComms->sendBlob(bs, 0);
	    }
	    

	}


    
    }


}
