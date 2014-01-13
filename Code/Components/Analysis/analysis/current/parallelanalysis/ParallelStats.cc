/// @file
///
/// Obtaining image statistics through distributed processing
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
#include <parallelanalysis/ParallelStats.h>

#include <askap_analysis.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <analysisutilities/AnalysisUtilities.h>
#include <askapparallel/AskapParallel.h>
#include <duchamp/Cubes/cubes.hh>

#include <Blob/BlobString.h>
#include <Blob/BlobIBufString.h>
#include <Blob/BlobOBufString.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>
using namespace LOFAR::TYPES;


///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".parallelstats");

namespace askap {

    namespace analysis {

	ParallelStats::ParallelStats(askap::askapparallel::AskapParallel& comms, duchamp::Cube *cube):
	    itsComms(&comms), itsCube(cube)
	{
	}

	ParallelStats::ParallelStats(const ParallelStats& other):
	    itsComms(other.itsComms),
	    itsCube(other.itsCube)
	{
	}

	ParallelStats& ParallelStats::operator= (const ParallelStats& other)
	{
	    if(this == &other) return *this;
	    this->itsComms = other.itsComms;
	    this->itsCube = other.itsCube;
	    return *this;
	}


	void ParallelStats::findDistributedStats()
	{
	    if(this->itsComms->isParallel()){
		ASKAPLOG_INFO_STR(logger, "Finding stats via distributed analysis.");
		this->findMeans();
		this->combineMeans();
		this->broadcastMean();
		this->findStddevs();
		this->combineStddevs();
	    }
	}

	void ParallelStats::findMeans()
	{
            /// @details This finds the mean or median (according to
            /// the flagRobustStats parameter) of the worker's
            /// image/cube, then sends that value to the master via
            /// LOFAR Blobs.

	    if (this->itsComms->isWorker()) {

		if (this->itsCube->pars().getFlagATrous()) this->itsCube->ReconCube();
		else if (this->itsCube->pars().getFlagSmooth()) this->itsCube->SmoothCube();
		
		int32 size = 0;
		float mean = 0., stddev;
		if(!this->itsCube->pars().getFlagStatSec() || this->itsCube->pars().statsec().isValid()) {
		    float *array=0;
		    // make a mask in case there are blank pixels.
		    bool *mask = this->itsCube->pars().makeStatMask(this->itsCube->getArray(), this->itsCube->getDimArray());
		    for(size_t i=0;i<this->itsCube->getSize();i++) if(mask[i]) size++;		      

		    if (size > 0) {
                        if (this->itsCube->pars().getFlagATrous())       array = this->itsCube->getArray();
                        else if (this->itsCube->pars().getFlagSmooth())  array = this->itsCube->getRecon();
                        else                                             array = this->itsCube->getArray();
			
                        // calculate mean
                        if (this->itsCube->pars().getFlagRobustStats()) 
			    mean = findMedian<float>(array, mask, this->itsCube->getSize());
                        else
			    mean = findMean<float>(array, mask, this->itsCube->getSize());
		    }
		    ASKAPLOG_INFO_STR(logger, "Worker Mean = " << mean);
		}
		else {
		    // No good points in the stats section
		    mean = 0.;
		}
		double dmean = mean;
		LOFAR::BlobString bs;
		bs.resize(0);
		LOFAR::BlobOBufString bob(bs);
		LOFAR::BlobOStream out(bob);
		out.putStart("meanW2M", 1);
		int16 rank = this->itsComms->rank();
		out << rank << dmean << size;
		out.putEnd();
		this->itsComms->sendBlob(bs, 0);
	    }
	}

	void ParallelStats::findStddevs()
	{
	    /// @details This finds the stddev or the
            /// median absolute deviation from the median (MADFM) (dictated
            /// by the flagRobustStats parameter) of the worker's
            /// image/cube, then sends that value to the master via LOFAR
            /// Blobs. To calculate the stddev/MADFM, the mean of the full
            /// dataset must be read from the master (again passed via LOFAR
            /// Blobs). The calculation uses the findSpread() function.

            if (this->itsComms->isWorker()){
                // first read in the overall mean for the cube
                double mean = 0;

		LOFAR::BlobString bs1;
		this->itsComms->receiveBlob(bs1, 0);
		LOFAR::BlobIBufString bib(bs1);
		LOFAR::BlobIStream in(bib);
		int version = in.getStart("meanM2W");
		ASKAPASSERT(version == 1);
		in >> mean;
		in.getEnd();

                // use it to calculate the stddev for this section
                int32 size = 0;
                double stddev = 0.;
		if(this->itsCube->pars().getFlagStatSec() && !this->itsCube->pars().statsec().isValid()) {
		  // Only way to get here is if flagStatSec=true but statsec is invalid (ie. has no pixels in this worker)
		  stddev = 0.;
		}
		else {
		  float *array=0;
		  
		  if (this->itsCube->pars().getFlagATrous()) {
		      // create an array that has the residual values from the reconstruction
		    array = new float[this->itsCube->getSize()];
		    
		    for (size_t i = 0; i < this->itsCube->getSize(); i++) array[i] = this->itsCube->getPixValue(i) - this->itsCube->getReconValue(i);
		  } else if (this->itsCube->pars().getFlagSmooth()) array = this->itsCube->getRecon();
		  else array = this->itsCube->getArray();
		  
		  bool *mask = this->itsCube->pars().makeStatMask(array, this->itsCube->getDimArray());
		  for(size_t i=0;i<this->itsCube->getSize();i++) if(mask[i]) size++;		      

		  if(size>0)
		    stddev = findSpread(this->itsCube->pars().getFlagRobustStats(), mean, this->itsCube->getSize(), array, mask);
		  
		  if (this->itsCube->pars().getFlagATrous()) delete [] array;

		  ASKAPLOG_INFO_STR(logger, "StdDev = " << stddev);
		}

                // return it to the master
                LOFAR::BlobString bs;
                bs.resize(0);
                LOFAR::BlobOBufString bob(bs);
                LOFAR::BlobOStream out(bob);
                out.putStart("stddevW2M", 1);
                int16 rank = this->itsComms->rank();
                out << rank << stddev << size;
                out.putEnd();
                this->itsComms->sendBlob(bs, 0);
	    }
	}

	void ParallelStats::combineMeans()
	{
            /// @details The master reads the mean/median values from each
            /// of the workers, and combines them to form the mean/median of
            /// the full dataset. Note that if the median of the workers
            /// data has been provided, the values are treated as estimates
            /// of the mean, and are combined as if they were means (ie. the
            /// overall value is the weighted (by size) average of the
            /// means/medians of the individual images). The value is stored
            /// in the StatsContainer in itsCube.

	    if (this->itsComms->isMaster()){
                // get the means from the workers
                LOFAR::BlobString bs;
                int64 size = 0;
                double av = 0;

                for (int i = 1; i < this->itsComms->nProcs(); i++) {
                    this->itsComms->receiveBlob(bs, i);
                    LOFAR::BlobIBufString bib(bs);
                    LOFAR::BlobIStream in(bib);
                    int version = in.getStart("meanW2M");
                    ASKAPASSERT(version == 1);
                    double newav;
                    int32 newsize;
                    int16 rank;
                    in >> rank >> newav >> newsize;
                    in.getEnd();
		    if(newsize > 0){
			size += newsize;
			av += newav * newsize;
		    }
		}

                if (size > 0) {
                    av /= double(size);
                }

                ASKAPLOG_INFO_STR(logger, "Overall size = " << size);
                ASKAPLOG_INFO_STR(logger, "Overall mean = " << av);

                this->itsCube->stats().setMean(av);
            } 
	}	   
		    
	void ParallelStats::broadcastMean()
	{
	    /// @details The mean/median value of the full dataset is sent
            /// via LOFAR Blobs to the workers.
            if (this->itsComms->isMaster()){
                double av = this->itsCube->stats().getMean();
                LOFAR::BlobString bs;
                bs.resize(0);
                LOFAR::BlobOBufString bob(bs);
                LOFAR::BlobOStream out(bob);
                out.putStart("meanM2W", 1);
                out << av;
                out.putEnd();
                for (int i = 1; i < this->itsComms->nProcs(); ++i) {
                    this->itsComms->sendBlob(bs, i);
                }
            }
	}

	void ParallelStats::combineStddevs()
	{
            /// @details The master reads the stddev/MADFM values from each of
            /// the workers, and combines them to produce an estimate of the
            /// stddev for the full cube. Again, if MADFM values have been
            /// calculated on the workers, they are treated as estimates of
            /// the stddev and are combined as if they are stddev values. The
            /// overall value is stored in the StatsContainer in itsCube->

            if (this->itsComms->isMaster()){
                // get the means from the workers
                LOFAR::BlobString bs;
                int64 size = 0;
                double stddev = 0;

                for (int i = 1; i < this->itsComms->nProcs(); i++) {
                    this->itsComms->receiveBlob(bs, i);
                    LOFAR::BlobIBufString bib(bs);
                    LOFAR::BlobIStream in(bib);
                    int version = in.getStart("stddevW2M");
                    ASKAPASSERT(version == 1);
                    double newstddev;
                    int32 newsize;
                    int16 rank;
                    in >> rank >> newstddev >> newsize;
                    in.getEnd();
		    if(newsize>0){
			size += newsize;
			stddev += (newstddev * newstddev * (newsize - 1));
		    }
		}

                if (size > 0) {
                    stddev = sqrt(stddev / double(size - 1));
                }

                this->itsCube->stats().setStddev(stddev);
                this->itsCube->stats().setRobust(false);
		this->itsCube->stats().define(this->itsCube->stats().getMiddle(),0.F,this->itsCube->stats().getSpread(),1.F);

                if (!this->itsCube->pars().getFlagUserThreshold()) {
		  ASKAPLOG_INFO_STR(logger, "Setting threshold to be " << this->itsCube->pars().getCut() << " sigma");
                    this->itsCube->stats().setThresholdSNR(this->itsCube->pars().getCut());
		    ASKAPLOG_INFO_STR(logger, "Threshold now " << this->itsCube->stats().getThreshold() << " since middle = " << this->itsCube->stats().getMiddle() << " and spread = " << this->itsCube->stats().getSpread());
                    this->itsCube->pars().setFlagUserThreshold(true);
                    this->itsCube->pars().setThreshold(this->itsCube->stats().getThreshold());
                }

                ASKAPLOG_INFO_STR(logger, "Overall StdDev = " << stddev);
            }
	}

	void ParallelStats::printStats()
	{
	    
	}

    }

}
