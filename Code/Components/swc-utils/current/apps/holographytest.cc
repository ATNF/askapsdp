/// @file
/// @brief an utility to extract holography measurement from the measurement set produced by sw-correlation
/// @copyright (c) 2007 CSIRO
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


#include <dataaccess/TableDataSource.h>
#include <askap_accessors.h>
#include <askap/AskapLogging.h>
#include <askap/AskapUtil.h>
ASKAP_LOGGER(logger, "");

#include <askap/AskapError.h>
#include <dataaccess/SharedIter.h>

#include <dataaccess/TableManager.h>
#include <dataaccess/IDataConverterImpl.h>
#include <swcorrelator/BasicMonitor.h>

// casa
#include <measures/Measures/MFrequency.h>
#include <tables/Tables/Table.h>
#include <casa/OS/Timer.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/MatrixMath.h>
#include <images/Images/PagedImage.h>
#include <lattices/Lattices/ArrayLattice.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/LinearCoordinate.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>



// std
#include <stdexcept>
#include <iostream>
#include <vector>
#include <iomanip>

using std::cout;
using std::cerr;
using std::endl;

using namespace askap;
using namespace askap::accessors;

const casa::uInt refAnt = 0; // the one which doesn't move
const casa::uInt maxMappedAnt = 2;
const casa::uInt maxMappedBeam = 4;

// converts antenna index into plane index (i.e. bypasses the reference antenna
casa::uInt getAntPlaneIndex(const casa::uInt &in) {
  if (in<refAnt) {
      return in;
  }
  ASKAPDEBUGASSERT(in>0);
  return in - 1;
}

casa::Matrix<casa::Complex> processOnePoint(const IConstDataSource &ds, const int ctrl = -1) {
  IDataSelectorPtr sel=ds.createSelector();
  if (ctrl >=0 ) {
      sel->chooseUserDefinedIndex("CONTROL",casa::uInt(ctrl));
  }
  IDataConverterPtr conv=ds.createConverter();  
  conv->setFrequencyFrame(casa::MFrequency::Ref(casa::MFrequency::TOPO),"MHz");
  conv->setEpochFrame(casa::MEpoch(casa::Quantity(55913.0,"d"),
                      casa::MEpoch::Ref(casa::MEpoch::UTC)),"s");
  conv->setDirectionFrame(casa::MDirection::Ref(casa::MDirection::J2000));                    

  casa::Matrix<casa::Complex> result(maxMappedAnt,maxMappedBeam, casa::Complex(0.,0.));
  casa::Matrix<casa::uInt>  counts(maxMappedAnt,maxMappedBeam, 0u);

  casa::Vector<double> freq;
  size_t counter = 0;
  size_t nGoodRows = 0;
  size_t nBadRows = 0;
  casa::uInt nChan = 0;
  casa::uInt nRow = 0;
  double startTime = 0;
  double stopTime = 0;
    
  for (IConstDataSharedIter it=ds.createConstIterator(sel,conv);it!=it.end();++it) {  
       if (nChan == 0) {
           nChan = it->nChannel();
           nRow = it->nRow();
           freq = it->frequency();
       } else { 
           ASKAPCHECK(nChan == it->nChannel(), 
                  "Number of channels seem to have been changed, previously "<<nChan<<" now "<<it->nChannel());
       }
       
       ASKAPASSERT(it->nPol() >= 1);
       ASKAPASSERT(it->nChannel() > 1);
       
       // add new value to the buffer
       for (casa::uInt row=0; row<nRow; ++row) {
            casa::Vector<casa::Bool> flags = it->flag().xyPlane(0).row(row);
            bool flagged = false;
            for (casa::uInt ch = 0; ch < flags.nelements(); ++ch) {
                 flagged |= flags[ch];
            }
            
            casa::Vector<casa::Complex> measuredRow = it->visibility().xyPlane(0).row(row);
            
            
            // flagging based on the amplitude (to remove extreme outliers)
            casa::Complex currentAvgVis = casa::sum(measuredRow) / float(it->nChannel());

            /*
            if ((casa::abs(currentAvgVis) > 0.05) && (row % 3 == 2)) {
                flagged = true;
            } 
            
            // optional flagging based on time-range
            if ((counter>1) && ((it->time() - startTime)/60.>1050.)) {
                flagged = true;
            }
            */
            
            // we have to discard rows which do not contain the reference antenna
            if ((it->antenna1()[row] != refAnt) && (it->antenna2()[row] != refAnt)) {
                flagged = true;
            }
            //
            
            if (flagged) {
               ++nBadRows;
            } else {

                ++nGoodRows;
                const casa::uInt beam = it->feed1()[row];
                ASKAPDEBUGASSERT(beam == it->feed2()[row]);
                casa::uInt ant = it->antenna2()[row];
                bool needConjugate = false;
                if (ant == refAnt) {
                    ant = it->antenna1()[row];
                    needConjugate = true;
                }
                ASKAPASSERT(ant != refAnt);
                const casa::uInt planeIndex = getAntPlaneIndex(ant);
                ASKAPDEBUGASSERT(planeIndex < result.nrow());
                ASKAPDEBUGASSERT(beam < result.ncolumn());
                if (needConjugate) {
                   result(planeIndex,beam) += conj(currentAvgVis);
                } else {
                   result(planeIndex,beam) += currentAvgVis;
                }
                counts(planeIndex,beam) += 1;
            }
       }
       if ((counter == 0) && (nGoodRows == 0)) {
           // all data are flagged, completely ignoring this iteration and consider the next one to be first
           nChan = 0;
           continue;
       }
      
       if (++counter == 1) {
           startTime = it->time();
       }
       stopTime = it->time() + 1; // 1s integration time is hardcoded
  }
  if (counter>1) {
      for (casa::uInt row=0; row<result.nrow(); ++row) {
           for (casa::uInt col=0; col<result.ncolumn(); ++col) {
                if (counts(row,col) > 0) {
                    result(row,col) /= float(counts(row,col));
                }
           }
      }
      std::cout<<"Processed "<<counter<<" integration cycles for ctrl="<<ctrl<<", "<<nGoodRows<<" good and "<<nBadRows<<" bad rows, time span "<<(stopTime-startTime)/60.<<" minutues"<<std::endl;
  } else {
     std::cout<<"No data found for ctrl="<<ctrl<<std::endl;
  }
  return result;
}

void process(const IConstDataSource &ds, const casa::uInt size) {
   const double resolutionInRad = 0.5 / 180. * casa::C::pi; // in radians
   ASKAPDEBUGASSERT(size % 2 == 1);
   ASKAPDEBUGASSERT(size > 1);
   const int halfSize = (int(size) - 1) / 2;
   //const casa::IPosition targetShape(4,int(size),int(size),maxMappedBeam,maxMappedAnt);
   const casa::IPosition targetShape(3,int(size),int(size),maxMappedBeam*maxMappedAnt);
   casa::Array<float> buf(targetShape,0.);

   int counter = 0;
   for (int x = -halfSize; x <= halfSize; ++x) {
       const int dir = (x + halfSize) % 2 == 0 ? 1 : -1; // the first scan is in the increasing order
       for (int y = -halfSize; y <= halfSize; ++y) {

            ++counter; // effectively a 1-based counter

            int planeCounter = 0;
            casa::Matrix<casa::Complex> result = processOnePoint(ds,counter);
            for (casa::uInt ant = 0; ant < result.nrow(); ++ant) {
                 for (casa::uInt beam = 0; beam < result.ncolumn(); ++beam,++planeCounter) {
                      //const casa::IPosition curPos(4, x + halfSize, halfSize - y * dir,int(beam),int(ant));
                      ASKAPDEBUGASSERT(planeCounter < targetShape(2));
                      const casa::IPosition curPos(3, x + halfSize, halfSize - y * dir,planeCounter);
                      buf(curPos) = casa::abs(result(ant,beam));
                 }
            }
       }
   }

   // storing the image
   size_t nDim = buf.shape().nonDegenerate().nelements();
   ASKAPASSERT(nDim>=2);
      
   casa::Matrix<double> xform(2,2,0.);
   xform.diagonal() = 1.;
   casa::DirectionCoordinate dc(casa::MDirection::AZEL, casa::Projection(casa::Projection::SIN),0.,0.,
         resolutionInRad, resolutionInRad, xform, double(halfSize), double(halfSize));
         
   casa::CoordinateSystem coords;
   coords.addCoordinate(dc);
      
   for (size_t dim=2; dim<nDim; ++dim) {
        casa::Vector<casa::String> addname(1);
        if (dim == 2) {
            addname[0] = targetShape.nelements() == 4 ? "beam" : "";
        } else if (dim == 3) {
            addname[0] = "antenna";
        } else {
           addname[0]="addaxis"+utility::toString<size_t>(dim-3);
        }
        casa::Matrix<double> xform(1,1,1.);
        casa::LinearCoordinate lc(addname, addname,
        casa::Vector<double>(1,0.), casa::Vector<double>(1,1.),xform, 
            casa::Vector<double>(1,0.));
        coords.addCoordinate(lc);
   }
   casa::PagedImage<casa::Float> resimg(casa::TiledShape(buf.nonDegenerate().shape()), coords, "beammap.img");
   casa::ArrayLattice<casa::Float> lattice(buf.nonDegenerate());
   resimg.copyData(lattice);
}
   
int main(int argc, char **argv) {
  try {
     if (argc!=2) {
         cerr<<"Usage: "<<argv[0]<<" measurement_set"<<endl;
	 return -2;
     }

     casa::Timer timer;
     const std::string msName = argv[1];

     timer.mark();
     TableDataSource ds(msName,TableDataSource::MEMORY_BUFFERS);     
     std::cerr<<"Initialization: "<<timer.real()<<std::endl;
     timer.mark();
     process(ds,17);
     std::cerr<<"Job: "<<timer.real()<<std::endl;
     
  }
  catch(const AskapError &ce) {
     cerr<<"AskapError has been caught. "<<ce.what()<<endl;
     return -1;
  }
  catch(const std::exception &ex) {
     cerr<<"std::exception has been caught. "<<ex.what()<<endl;
     return -1;
  }
  catch(...) {
     cerr<<"An unexpected exception has been caught"<<endl;
     return -1;
  }
  return 0;
}
