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

#include <parallelanalysis/DuchampParallel.h>

#include <casainterface/CasaInterface.h>

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
	    if (this->itsComms->isParallel()){

		if(this->itsComms->isMaster()){
		    // send objects in itsInputList to workers in round-robin fashion
		    // broadcast 'finished' signal
		    LOFAR::BlobString bs;
		    for(size_t i=0;i<this->itsInputList.size();i++){
			unsigned int rank = i % (this->itsComms->nProcs() - 1);
			ASKAPLOG_DEBUG_STR(logger, "Sending source #"<<i+1<<", ID="<< this->itsInputList[i].getID() << " to worker "<<rank+1 << " for parameterisation");
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
		    //this->itsComms->broadcastBlob(bs,0);
		    for (int i = 1; i < this->itsComms->nProcs(); ++i) {
			this->itsComms->sendBlob(bs, i);
		    }

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
			    ASKAPLOG_DEBUG_STR(logger, "Worker " << this->itsComms->rank() << " received object ID " << this->itsInputList.back().getID());
			}
			in.getEnd();
		    }
		    ASKAPLOG_DEBUG_STR(logger, "Worker " << this->itsComms->rank() << " received " << this->itsInputList.size() << " objects to parameterise.");

		}
	    }
	}

	void ObjectParameteriser::parameterise()
	{
	    if(this->itsComms->isWorker()){
		// For each object, get the bounding subsection for that object
		// Define a DuchampParallel and use it to do the parameterisation
		// put parameterised objects into itsOutputList

		if (this->itsInputList.size() > 0){

		    //std::vector<size_t> dim(this->itsDP->cube().getDimArray(),this->itsDP->cube().getDimArray()+this->itsDP->cube().getNumDim());
		    std::vector<size_t> dim = analysisutilities::getCASAdimensions(this->itsDP->cube().pars().getImageFile());
		    LOFAR::ParameterSet parset=this->itsDP->parset();
		    parset.replace("flagsubsection","true");

		    int padsize;
		    if(this->itsDP->fitParams()->doFit() && !this->itsDP->fitParams()->fitJustDetection())
			padsize = std::max(this->itsDP->fitParams()->boxPadSize(),this->itsDP->fitParams()->noiseBoxSize());
		    else
			padsize = 0;

		    for(size_t i=0;i<this->itsInputList.size();i++){

			ASKAPLOG_DEBUG_STR(logger, "Parameterising object #"<<i << " out of " << this->itsInputList.size());

			// get bounding subsection & transform into a Subsection string
			parset.replace("subsection",this->itsInputList[i].boundingSubsection(dim, this->itsDP->cube().pHeader(), padsize, true));
			// turn off the subimaging, so we read the whole lot.
			parset.replace("nsubx","1");
			parset.replace("nsuby","1");
			parset.replace("nsubz","1");

			// define a duchamp Cube using the filename from the this->itsDP->cube()
			// set the subsection
			DuchampParallel tempDP(*this->itsComms, parset);

			// open the image
			tempDP.readData();

			this->itsInputList[i].setOffsets(tempDP.cube().pars());
			this->itsInputList[i].removeOffsets();
			this->itsInputList[i].setFlagText("");

			// store the current object to the cube
			tempDP.cube().addObject(this->itsInputList[i]);

			// parameterise
			tempDP.cube().calcObjectWCSparams();

			sourcefitting::RadioSource src(tempDP.cube().getObject(0));

			if(tempDP.fitParams()->doFit()){

			    src.setFitParams(*tempDP.fitParams());
			    src.setDetectionThreshold(tempDP.cube(), tempDP.getFlagVariableThreshold());
			    src.prepareForFit(tempDP.cube(),true);
			    src.setAtEdge(false);

			    tempDP.fitSource(src);
			}

			src.addOffsets();

			// get the parameterised object and store to itsOutputList
			this->itsOutputList.push_back(src);
		    }

		    ASKAPASSERT(this->itsOutputList.size() == this->itsInputList.size());

		}

	    }

	}

	void ObjectParameteriser::gather()
	{
	    if(this->itsComms->isParallel()){
		
		if(this->itsInputList.size() > 0) {

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
			    ASKAPLOG_DEBUG_STR(logger, "Reading " << numSrc << " objects from worker #"<<n+1);
			    for(int i=0;i<numSrc;i++){
				sourcefitting::RadioSource src;
				in >> src;
				ASKAPLOG_DEBUG_STR(logger, "Read parameterised object " << src.getName() <<", ID="<<src.getID());
				src.setHeader(this->itsDP->cube().pHeader());  // make sure we have the right WCS etc information
				src.setOffsets(this->itsDP->cube().pars());
				this->itsOutputList.push_back(src);
				this->itsDP->pEdgeList()->push_back(src);
			    }
			    in.getEnd();
			}

			ASKAPASSERT(this->itsOutputList.size() == this->itsInputList.size());
			ASKAPASSERT(this->itsOutputList.size() == this->itsDP->pEdgeList()->size());

		    }
		    else{
			// for each object in itsOutputList, send to master
			ASKAPLOG_INFO_STR(logger, "Have parameterised " << this->itsInputList.size() << " edge sources. Returning results to master.");
			LOFAR::BlobString bs;
			bs.resize(0);
			LOFAR::BlobOBufString bob(bs);
			LOFAR::BlobOStream out(bob);
			out.putStart("OPfinal", 1);
			out << int(this->itsOutputList.size());
			for(size_t i=0;i<this->itsOutputList.size();i++) out << this->itsOutputList[i];
			out.putEnd();
			this->itsComms->sendBlob(bs, 0);
		    }
	    
		}
		else {
		    // serial case - need to put output sources into the DP edgelist
		    for(size_t i=0;i<this->itsOutputList.size();i++){
			this->itsOutputList[i].setHeader(this->itsDP->cube().pHeader());  // make sure we have the right WCS etc information
			this->itsDP->pEdgeList()->push_back( this->itsOutputList[i] );
		    }

		}

	    }

	}
    
    }


}
