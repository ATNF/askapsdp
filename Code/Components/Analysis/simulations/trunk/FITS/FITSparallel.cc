/// @file
///
/// Implementation of the parallel handling of FITS creation
///
/// @copyright (c) 2008 CSIRO
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
#include <askap_simulations.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <FITS/FITSparallel.h>
#include <FITS/FITSfile.h>

#include <askapparallel/AskapParallel.h>
#include <duchamp/Utils/Section.hh>
#include <analysisutilities/SubimageDef.h>

#include <Common/LofarTypedefs.h>
using namespace LOFAR::TYPES;
#include <Blob/BlobString.h>
#include <Blob/BlobIBufString.h>
#include <Blob/BlobOBufString.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>
#include <Common/Exceptions.h>

#include <Common/ParameterSet.h>
#include <Common/KVpair.h>

#include <vector>
#include <iostream>
#include <sstream>

ASKAP_LOGGER(logger, ".fitsparallel");

namespace askap {

    namespace simulations {

        namespace FITS {


            FITSparallel::FITSparallel(askap::mwbase::AskapParallel& comms, const LOFAR::ParameterSet& parset)
                    : itsComms(comms)
            {
                /// @details Assignment of the necessary parameters, reading from the ParameterSet.

                ASKAPLOG_DEBUG_STR(logger, "Starting the definition of FITSparallel");

                LOFAR::ParameterSet newparset = parset;

                this->itsSubimageDef = analysis::SubimageDef(parset);
                int numSub = this->itsSubimageDef.nsubx() * this->itsSubimageDef.nsuby();

                if (itsComms.isParallel() && (numSub != itsComms.nNodes() - 1))
                    ASKAPTHROW(AskapError, "Number of requested subimages (" << numSub << ", = "
                                   << this->itsSubimageDef.nsubx() << "x" << this->itsSubimageDef.nsuby()
                                   << ") does not match the number of worker nodes (" << itsComms.nNodes() - 1 << ")");

                size_t dim = parset.getInt32("dim", 2);
                std::vector<int> axes = parset.getInt32Vector("axes");

                this->itsSubimageDef.define(dim);
                this->itsSubimageDef.setImageDim(axes);

                if (axes.size() != dim)
                    ASKAPTHROW(AskapError, "Dimension mismatch: dim = " << dim << ", but axes has " << axes.size() << " dimensions.");

                if (itsComms.isParallel() && itsComms.isWorker()) {

                    this->itsSubsection = this->itsSubimageDef.section(itsComms.rank() - 1, duchamp::nullSection(dim));
                    this->itsSubsection.parse(axes);

                    ASKAPLOG_DEBUG_STR(logger, "Worker #" << itsComms.rank() << " has offsets (" << this->itsSubsection.getStart(0) << "," << this->itsSubsection.getStart(1)
                                           << ") and dimensions " << this->itsSubsection.getDim(0) << "x" << this->itsSubsection.getDim(1));

		    // Update the subsection parameter to the appropriate string for this worker
		    newparset.replace("subsection",this->itsSubsection.getSection());

                } else {
                    this->itsSubsection.setSection(duchamp::nullSection(dim));
                    this->itsSubsection.parse(axes);
                }

                // For the parallel version only, one the first worker
                // should write an outputlist. This is done here because
                // FITSfile has no knowledge of its place in the distributed
                // program
                if (itsComms.isParallel() && (itsComms.rank() != 1)) {
                    newparset.replace("outputList", "false");
                }

                ASKAPLOG_DEBUG_STR(logger, "Defining FITSfile");
                this->itsFITSfile = new FITSfile(newparset);
                ASKAPLOG_DEBUG_STR(logger, "Defined");

                ASKAPLOG_DEBUG_STR(logger, "Finished defining FITSparallel");

            }

            //--------------------------------------------------

            void FITSparallel::toMaster()
            {

                /// @details For the workers, this function sends the x&y
                /// position of each pixel and the corresponding flux value to
                /// the Master node. For the master node, it receives that
                /// information from each worker and fills its copy of the
                /// flux array. When run in serial mode, this function does
                /// nothing.

                if (itsComms.isParallel()) {

                    if (itsComms.isWorker()) {
                        ASKAPLOG_DEBUG_STR(logger, "Worker #" << itsComms.rank() << ": about to send data to Master");
                        LOFAR::BlobString bs;
                        bs.resize(0);
                        LOFAR::BlobOBufString bob(bs);
                        LOFAR::BlobOStream out(bob);
                        out.putStart("pixW2M", 1);
			int spInd = this->itsFITSfile->getSpectralAxisIndex();
			ASKAPLOG_DEBUG_STR(logger, "Using index " << spInd << " as spectral axis");
                        out << this->itsSubsection.getStart(0) << this->itsSubsection.getStart(1) << this->itsSubsection.getStart(spInd);
			out << this->itsSubsection.getEnd(0)   << this->itsSubsection.getEnd(1)   << this->itsSubsection.getEnd(spInd);
                        ASKAPLOG_DEBUG_STR(logger, "Worker #" << itsComms.rank() << ": sent minima of " << this->itsSubsection.getStart(0)
					   << " and " << this->itsSubsection.getStart(1) << " and " << this->itsSubsection.getStart(spInd));
                        ASKAPLOG_DEBUG_STR(logger, "Worker #" << itsComms.rank() << ": sent maxima of " << this->itsSubsection.getEnd(0)
					   << " and " << this->itsSubsection.getEnd(1) << " and " << this->itsSubsection.getEnd(spInd));

			for(int z=0;z<this->itsFITSfile->getZdim();z++){
			  for(int y=0;y<this->itsFITSfile->getYdim();y++){
			    for(int x=0;x<this->itsFITSfile->getXdim();x++){
//                               int xpt = x+this->itsSubsection.getStart(0);
//                               int ypt = y+this->itsSubsection.getStart(1);
//                               int zpt = z+this->itsSubsection.getStart(spInd);
                              float fpt = this->itsFITSfile->array(x, y, z);
                              out << fpt;
//                               out << xpt << ypt << zpt << fpt;
                            }
			  }
			}

                        out.putEnd();
                        itsComms.connectionSet()->write(0, bs);

                    } else if (itsComms.isMaster()) {

                        LOFAR::BlobString bs;

                        for (int n = 1; n < itsComms.nNodes(); n++) {
                            ASKAPLOG_DEBUG_STR(logger, "MASTER: about to read data from Worker #" << n);
                            itsComms.connectionSet()->read(n - 1, bs);
                            LOFAR::BlobIBufString bib(bs);
                            LOFAR::BlobIStream in(bib);
                            int version = in.getStart("pixW2M");
                            ASKAPASSERT(version == 1);
                            int xmin, ymin, zmin, xmax, ymax, zmax;
                            in >> xmin >> ymin >> zmin >> xmax >> ymax >> zmax;
			    int xdim=(xmax - xmin + 1);
			    int ydim=(ymax - ymin + 1);
			    int zdim=(zmax - zmin + 1);
                            ASKAPLOG_DEBUG_STR(logger, "MASTER: Read minima of " << xmin << " and " << ymin << " and " << zmin);
                            ASKAPLOG_DEBUG_STR(logger, "MASTER: Read maxima of " << xmax << " and " << ymax << " and " << zmax);
			    ASKAPLOG_DEBUG_STR(logger, "MASTER: About to read " << xdim*ydim*zdim << " pixels");

                            for (int pix = 0; pix < xdim*ydim*zdim; pix++) {
// 			        int xpt, ypt, zpt;
                                float flux;
				int x=pix%xdim;
				int y=(pix/xdim)%ydim;
				int z=pix/(xdim*ydim);
//                                 in >> xpt >> ypt >> zpt >> flux;
//                                 ASKAPASSERT(xpt == (xmin + x));
//                                 ASKAPASSERT(ypt == (ymin + y));
// 				ASKAPASSERT(zpt == (zmin + z));
// 				flux += this->itsFITSfile->array(xpt, ypt, zpt);
//                                 this->itsFITSfile->setArray(xpt, ypt, zpt, flux);
                                in >> flux;
				size_t pos = (x+xmin)+xdim*(y+ymin)+ydim*xdim*(z+zmin);
				ASKAPASSERT(pos < this->itsFITSfile->getSize());
				flux += this->itsFITSfile->array(x+xmin, y+ymin, z+zmin);
//                                 this->itsFITSfile->setArray(x+xmin, y+ymin, z+zmin, flux);
                                this->itsFITSfile->setArray(pos, flux);
                            }

                            in.getEnd();

                        }

                    }

                }


            }

            //--------------------------------------------------

            void FITSparallel::addNoise()
            {
                if (itsComms.isWorker())
                    itsFITSfile->addNoise();
            }

            void FITSparallel::processSources()
            {
                if (itsComms.isWorker()) {
                    ASKAPLOG_DEBUG_STR(logger, "Worker #" << itsComms.rank() << ": About to add sources");
                    itsFITSfile->processSources();
                }
            }

            void FITSparallel::convolveWithBeam()
            {
                itsFITSfile->convolveWithBeam();
            }

            void FITSparallel::saveFile()
            {
                if (itsComms.isMaster())
                    itsFITSfile->saveFile();
            }

            void FITSparallel::writeCASAimage()
            {
                if (itsComms.isMaster())
                    itsFITSfile->writeCASAimage();
            }



        }

    }

}
