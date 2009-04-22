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

#include <gridding/TableVisGridder.h>
#include <askap_synthesis.h>
#include <askap/AskapLogging.h>
#include <dataaccess/IDataAccessor.h>
ASKAP_LOGGER(logger, ".gridding");

#include <askap/AskapError.h>
#include <askap/AskapUtil.h>
#include <fft/FFTWrapper.h>

#include <casa/BasicSL/Constants.h>
#include <casa/Arrays/ArrayIter.h>
#include <casa/Arrays/Slicer.h>

#include <measures/Measures/MDirection.h>
#include <measures/Measures/UVWMachine.h>

#include <fitting/Params.h>
#include <fitting/ParamsCasaTable.h>

#include <gridding/GridKernel.h>

#include <measurementequation/PaddingUtils.h>

using namespace askap::scimath;
using namespace askap;

#include <ostream>
#include <sstream>
#include <iomanip>

#include <casa/OS/Timer.h>
#include <casa/Quanta/MVAngle.h>

namespace askap {
namespace synthesis {


/// @brief a helper method for a deep copy of casa arrays held in
/// stl vectors
/// @param[in] in input array
/// @param[out] out output array (will be resized)
template<typename T>
void deepCopyOfSTDVector(const std::vector<T> &in,
                         std::vector<T> &out)
{
   out.resize(in.size());

   const typename std::vector<T>::const_iterator inEnd = in.end();
   typename std::vector<T>::iterator outIt = out.begin();
   for (typename std::vector<T>::const_iterator inIt = in.begin();
        inIt != inEnd; ++inIt,++outIt) {
        *outIt = inIt->copy();
   }
}

// Use // deliberatly as doxygen complains about duplicated documentation here and in h-file
// @brief a helper method to print directions nicely
// @details By default an instance of casa::MVDirection is printed
// as 3 direction cosines. It is not very convenient. This method
// allows to print it in a more log-reader-friendly way. We can move this
// method to a higher level if (when) it becomes necessary in other places.
// I (MV) didn't move it to askap just because it would introduce a
// dependency on casacore, although scimath may be a right place for this
// method.
// @param[in] dir MVDirection object to print
// @return a string containing a nice representation of the direction
std::string printDirection(const casa::MVDirection &dir)
{
   std::ostringstream os;
   os<<std::setprecision(8)<<casa::MVAngle::Format(casa::MVAngle::TIME)
     <<casa::MVAngle(dir.getLong("deg"))
     <<" "<<std::setprecision(8)<<casa::MVAngle::Format(casa::MVAngle::ANGLE)<<
     casa::MVAngle(dir.getLat("deg"));
   return os.str();
}

TableVisGridder::TableVisGridder() :
        itsSupport(-1), itsOverSample(-1),
	itsName(""), itsModelIsEmpty(false), itsSamplesGridded(0),
			itsSamplesDegridded(0), itsVectorsFlagged(0), itsNumberGridded(0), itsNumberDegridded(0),
	itsTimeCoordinates(0.0), itsTimeGridded(0.0), itsTimeDegridded(0.0), itsDopsf(false),
	itsPaddingFactor(1),
	itsFirstGriddedVis(true), itsFeedUsedForPSF(0), itsUseAllDataForPSF(false)

{
  itsSumWeights.resize(1,1,1);
  itsSumWeights.set(0.0);
}

TableVisGridder::TableVisGridder(const int overSample, const int support,
        const int padding, const std::string& name) :
		 itsSupport(support), itsOverSample(overSample), itsName(name),
				itsModelIsEmpty(false), itsSamplesGridded(0),
				itsSamplesDegridded(0), itsVectorsFlagged(0), itsNumberGridded(0), itsNumberDegridded(0),
		itsTimeCoordinates(0.0), itsTimeGridded(0.0), itsTimeDegridded(0.0), itsDopsf(false),
		itsPaddingFactor(padding),
		itsFirstGriddedVis(true), itsFeedUsedForPSF(0), itsUseAllDataForPSF(false)
	{
		
		ASKAPCHECK(overSample>0, "Oversampling must be greater than 0");
		ASKAPCHECK(support>0, "Maximum support must be greater than 0");
		ASKAPCHECK(padding>0, "Padding factor must be greater than 0");
	}

	/// @brief copy constructor
	/// @details it is required to decouple arrays between the input object
	/// and the copy.
	/// @param[in] other input object
	TableVisGridder::TableVisGridder(const TableVisGridder &other) : 
	     itsAxes(other.itsAxes), itsShape(other.itsShape), 
	     itsUVCellSize(other.itsUVCellSize.copy()), 
	     itsSumWeights(other.itsSumWeights.copy()), 
     itsSupport(other.itsSupport), itsOverSample(other.itsOverSample),
     itsCSize(other.itsCSize), itsCCenter(other.itsCCenter), itsName(other.itsName),
     itsModelIsEmpty(other.itsModelIsEmpty), itsSamplesGridded(other.itsSamplesGridded),
     itsSamplesDegridded(other.itsSamplesDegridded), itsVectorsFlagged(other.itsVectorsFlagged),
     itsNumberGridded(other.itsNumberGridded),
     itsNumberDegridded(other.itsNumberDegridded), itsTimeCoordinates(other.itsTimeCoordinates),
     itsTimeGridded(other.itsTimeGridded),
     itsTimeDegridded(other.itsTimeDegridded),
     itsDopsf(other.itsDopsf), itsPaddingFactor(other.itsPaddingFactor),
     itsFirstGriddedVis(other.itsFirstGriddedVis),
     itsFeedUsedForPSF(other.itsFeedUsedForPSF),
     itsPointingUsedForPSF(other.itsPointingUsedForPSF),
     itsUseAllDataForPSF(other.itsUseAllDataForPSF)
{
   deepCopyOfSTDVector(other.itsConvFunc,itsConvFunc);
   deepCopyOfSTDVector(other.itsGrid, itsGrid);   
   if(other.itsVisWeight) {
      itsVisWeight = other.itsVisWeight->clone();
   } else {
      itsVisWeight = other.itsVisWeight;
   }
}
     

TableVisGridder::~TableVisGridder() {
	if (itsNumberGridded>0) {
		ASKAPLOG_INFO_STR(logger, "TableVisGridder gridding statistics");
		if (isPSFGridder()) {
		    ASKAPLOG_INFO_STR(logger, "   PSF samples gridded       = "
                              << itsSamplesGridded);
            ASKAPLOG_INFO_STR(logger, "   Visibility vectors flagged (psf)     = "
                              << itsVectorsFlagged);                  
		    ASKAPLOG_INFO_STR(logger, "   Total time for PSF gridding   = "
				<< itsTimeGridded << " (s)");
		    ASKAPLOG_INFO_STR(logger, "   PSF gridding time         = " << 1e6
			 	*itsTimeGridded/itsSamplesGridded << " (us) per sample");
		    ASKAPLOG_INFO_STR(logger, "   Total time converting for PSF = "
	            << itsTimeCoordinates << " (s)");
		    ASKAPLOG_INFO_STR(logger, "   PSF coord conversion      = "
				  << 1e6 * itsTimeCoordinates/itsSamplesGridded << " (us) per sample");
		    ASKAPLOG_INFO_STR(logger, "   " << GridKernel::info());
		    ASKAPLOG_INFO_STR(logger, "   Points gridded (psf)        = "
	              << itsNumberGridded);
		    ASKAPLOG_INFO_STR(logger, "   Time per point (psf)        = " << 1e9
	              *itsTimeGridded/itsNumberGridded << " (ns)");
		    ASKAPLOG_INFO_STR(logger, "   Performance for PSF         = "
				<< 8.0 * 1e-9 * itsNumberGridded/itsTimeGridded << " GFlops");
		} else {
		    ASKAPLOG_INFO_STR(logger, "   Samples gridded       = "
                         << itsSamplesGridded);		
            ASKAPLOG_INFO_STR(logger, "   Visibility vectors flagged       = "
                          << itsVectorsFlagged);                                           
		    ASKAPLOG_INFO_STR(logger, "   Total time gridding   = "
			             << itsTimeGridded << " (s)");
		    ASKAPLOG_INFO_STR(logger, "   Gridding time         = " << 1e6
			  	*itsTimeGridded/itsSamplesGridded << " (us) per sample");
		    ASKAPLOG_INFO_STR(logger, "   Total time converting = "
				<< itsTimeCoordinates << " (s)");
		    ASKAPLOG_INFO_STR(logger, "   Coord conversion      = "
				  << 1e6 * itsTimeCoordinates/itsSamplesGridded << " (us) per sample");
		    ASKAPLOG_INFO_STR(logger, "   " << GridKernel::info());
		    ASKAPLOG_INFO_STR(logger, "   Points gridded        = "
				<< itsNumberGridded);
		    ASKAPLOG_INFO_STR(logger, "   Time per point        = " << 1e9
				*itsTimeGridded/itsNumberGridded << " (ns)");
		    ASKAPLOG_INFO_STR(logger, "   Performance           = "
				<< 8.0 * 1e-9 * itsNumberGridded/itsTimeGridded << " GFlops");
	    }			
	}
	if (itsNumberDegridded>0) {
		ASKAPLOG_INFO_STR(logger, "TableVisGridder degridding statistics");
		ASKAPLOG_INFO_STR(logger, "   Samples degridded     = "
				<< itsSamplesDegridded);
		ASKAPLOG_INFO_STR(logger, "   Total time degridding = "
				<< itsTimeDegridded << " (s)");
		ASKAPLOG_INFO_STR(logger, "   Degridding time       = " << 1e6
				*itsTimeDegridded/itsSamplesDegridded << " (us) per sample");
		ASKAPLOG_INFO_STR(logger, "   Total time converting = "
				<< itsTimeCoordinates << " (s)");
		ASKAPLOG_INFO_STR(logger, "   Coord conversion      = "
				  << 1e6 * itsTimeCoordinates/itsSamplesDegridded << " (us) per sample");
		ASKAPLOG_INFO_STR(logger, "   " << GridKernel::info());
		ASKAPLOG_INFO_STR(logger, "   Points degridded      = "
				<< itsNumberDegridded);
		ASKAPLOG_INFO_STR(logger, "   Time per point        = " << 1e9
				*itsTimeDegridded/itsNumberDegridded << " (ns)");
		ASKAPLOG_INFO_STR(logger, "   Performance           = "
				<< 8.0 * 1e-9 * itsNumberDegridded/itsTimeDegridded << " GFlops");
	}
	if((itsNumberGridded<1) && (itsNumberDegridded<1)) {
	  ASKAPLOG_WARN_STR(logger, "Unused gridder");
	} else {
	  ASKAPLOG_INFO_STR(logger, "   Padding factor    = " << itsPaddingFactor);
	  if(itsName!="") {
	    save(itsName);
	  }
	}
}

void TableVisGridder::save(const std::string& name) {
	askap::scimath::ParamsCasaTable iptable(name, false);
	askap::scimath::Params ip;
	ASKAPLOG_INFO_STR(logger, "Saving " << itsConvFunc.size() << " entries in convolution function");
	for (unsigned int i=0; i<itsConvFunc.size(); i++) {
		{
			casa::Array<double> realC(itsConvFunc[i].shape());
			toDouble(realC, itsConvFunc[i]);
			//			ASKAPLOG_INFO_STR(logger, "Entry[" <<  i <<  "] has shape " <<  itsConvFunc[i].shape());
			std::ostringstream os;
			os<<"Real.Convolution";
			os.width(5);
			os.fill('0');
			os<<i;
			ip.add(os.str(), realC);
		}
	}
	iptable.setParameters(ip);
}

/// This is a generic grid/degrid
void TableVisGridder::generic(IDataAccessor& acc, bool forward) {
   if (forward&&itsModelIsEmpty)
		return;
   
   if (forward && isPSFGridder()) {
       ASKAPTHROW(AskapError, "Logic error: the gridder is not supposed to be used for degridding in the PSF mode")
   }

   //casa::Vector<casa::RigidVector<double, 3> > outUVW;
   //casa::Vector<double> delay;
      
   casa::Timer timer;
   
   // Time the coordinate conversions, etc.
   timer.mark();
   
   const casa::Vector<casa::RigidVector<double, 3> > &outUVW = acc.rotatedUVW(getTangentPoint());
   const casa::Vector<double> &delay = acc.uvwRotationDelay(getTangentPoint(), getImageCentre());
   
   //rotateUVW(acc, outUVW, delay);
   
   initIndices(acc);
   initConvolutionFunction(acc);
   
   itsTimeCoordinates+=timer.real();

   // Now time the gridding
   timer.mark();

   ASKAPCHECK(itsSupport>0, "Support must be greater than 0");
   ASKAPCHECK(itsUVCellSize.size()==2, "UV cell sizes not yet set");
   
   const uint nSamples = acc.nRow();
   const uint nChan = acc.nChannel();
   const uint nPol = acc.nPol();
   const casa::Vector<casa::Double>& frequencyList = acc.frequency();
			      
   ASKAPDEBUGASSERT(itsShape.nelements()>=2);
   const casa::IPosition onePlane4D(4, itsShape(0), itsShape(1), 1, 1);
   const casa::IPosition onePlane(2, itsShape(0), itsShape(1));
   
   // Loop over all samples adding them to the grid
   // First scale to the correct pixel location
   // Then find the fraction of a pixel to the nearest pixel
   // Loop over the entire itsSupport, calculating weights from
   // the convolution function and adding the scaled
   // visibility to the grid.
   
   ASKAPDEBUGASSERT(casa::uInt(nChan) <= frequencyList.nelements());
   ASKAPDEBUGASSERT(casa::uInt(nSamples) == acc.uvw().nelements());
   
   for (uint i=0; i<nSamples; ++i) {
       if (itsFirstGriddedVis && isPSFGridder()) {
           // data members related to representative feed and field are used for
           // reverse problem only (from visibilities to image). 
           if (itsUseAllDataForPSF) {
               ASKAPLOG_INFO_STR(logger, "All data are used to estimate PSF");       
           } else {
               itsFeedUsedForPSF = acc.feed1()(i);
               itsPointingUsedForPSF = acc.dishPointing1()(i);    
               ASKAPLOG_INFO_STR(logger, "Using the data for feed "<<itsFeedUsedForPSF<<
                  " and field at "<<printDirection(itsPointingUsedForPSF)<<" to estimate the PSF");
           }
           itsFirstGriddedVis = false;
       }
	   /// Temporarily fix to do MFS only
	   int imageChan=0;
	   int imagePol=0;
	   
	   for (uint chan=0; chan<nChan; ++chan) {
		   
		   /// Scale U,V to integer pixels plus fractional terms
		   double uScaled=frequencyList[chan]*outUVW(i)(0)/(casa::C::c *itsUVCellSize(0));
		   int iu = askap::nint(uScaled);
		   int fracu=askap::nint(itsOverSample*(double(iu)-uScaled));
		   if (fracu<0) {
			   iu+=1;
		   }
		   if (fracu>=itsOverSample) {
			   iu-=1;
		   }
		   fracu=askap::nint(itsOverSample*(double(iu)-uScaled));
		   ASKAPCHECK(fracu>-1, "Fractional offset in u is negative");
		   ASKAPCHECK(fracu<itsOverSample,
				   "Fractional offset in u exceeds oversampling");
		   iu+=itsShape(0)/2;
		   
		   double vScaled=frequencyList[chan]*outUVW(i)(1)/(casa::C::c *itsUVCellSize(1));
		   int iv = askap::nint(vScaled);
		   int fracv=askap::nint(itsOverSample*(double(iv)-vScaled));
		   if (fracv<0) {
			   iv+=1;
		   }
		   if (fracv>=itsOverSample) {
			   iv-=1;
		   }
		   fracv=askap::nint(itsOverSample*(double(iv)-vScaled));
		   ASKAPCHECK(fracv>-1, "Fractional offset in v is negative");
		   ASKAPCHECK(fracv<itsOverSample,
				   "Fractional offset in v exceeds oversampling");
		   iv+=itsShape(1)/2;
		   
		   // Calculate the delay phasor
		   const double phase=2.0f*casa::C::pi*frequencyList[chan]*delay(i)/(casa::C::c);
			          
		   const casa::Complex phasor(cos(phase), sin(phase));
		   
		   bool allPolGood=true;
		   for (uint pol=0; pol<nPol; ++pol) {
			   if (acc.flag()(i, chan, pol))
				   allPolGood=false;
		   }

		   // Ensure that we only use unflagged data, incomplete polarisation vectors are 
		   // ignored
		   // @todo Be more careful about matching polarizations
		   if (allPolGood) {
		     
		     // Now loop over all visibility polarizations
		     for (uint pol=0; pol<nPol; ++pol) {
		          // temporary fix as we always do MFS-like operation for polarisations
		          // at the moment. Otherwise, the weight would be incremented for cross-pols
		          // as if we would have two more measurements for the same point (causing 
		          // resulting flux to be reduced.
		          if ((pol == 1) || (pol == 2)) {
		              continue;
		          }
			   // Lookup the portion of grid to be
			   // used for this row, polarisation and channel
			   const int gInd=gIndex(i, pol, chan);
			   ASKAPCHECK(gInd>-1,
					   "Index into image grid is less than zero");
			   ASKAPCHECK(gInd<int(itsGrid.size()),
					   "Index into image grid exceeds number of planes");
			   
			   // MFS override of imagePol applies to gridding only
			   // degridding should treat polarisations independently
			   if (forward) {
			      ASKAPCHECK((nPol == 1) || (nPol == 2) || (nPol == 4),
					      "degridding onto only 1,2 and 4 correlations are supported,current number of correlations is "<< nPol);
			      //
			      // Indexing : grid[nx,ny,npol,nchan] , vis(i,pol,chan)
			      //
			      // The following convention is implemented 
			      // to degrid multiple image planes onto visibility correlations.
			      // 
			      //  nImagePols   nPol       
			      //       1         1    : grid[,,0,]->vis(,0,)
			      //       1         2    : grid[,,0,]->vis(,0,) and grid[,,0,]->vis(,1,)
			      //       1         4    : grid[,,0,]->vis(,0,) and grid[,,0,]->vis(,3,) and vis(,1,)=vis(,2,)=0 
			      //       2         2    : grid[,,0,]->vis(,0,) and grid[,,1,]->vis(,1,)
			      //       2         4    : grid[,,0,]->vis(,0,) and grid[,,1,]->vis(,3,) and vis(,1,)=vis(,2,)=0 
			      //       4         4    : grid[,,0,]->vis(,0,) and grid[,,1,]->vis(,1,)
			      //                        grid[,,2,]->vis(,2,) and grid[,,3,]->vis(,3,)
			      //
			      const casa::IPosition gridShape = itsGrid[gInd].shape();
			      uint nImagePols = 1;
			      nImagePols = (gridShape.nelements()<=2) ? 1 : gridShape[2];
				      
			      ASKAPCHECK(nImagePols <= nPol," Number of image planes should be <= number of visibility correlations,currently nImagePols = " << nImagePols << ", nVisPols = " << nPol);
			      			      
			      ASKAPCHECK((nImagePols == 1) || (nImagePols == 2) || (nImagePols == 4),
				     "only 1,2 and 4 polarisations are supported,current grid shape is "<<gridShape);

			      // If there are 4 visibility pols, but no cross pol images...
			      if( (nPol==4) && (nImagePols != 4) && (pol==1 || pol==2) ) {
			           continue;
			      }
				      
			      // For most cases, imagepol and vispol indices will align
			      imagePol = pol; 

			      // Two exceptions
			      if(nImagePols==1 && (pol==0 || pol==3)) {
			         imagePol = 0;
			      }
			      if(nImagePols==2 && pol==3) {
			         imagePol = 1;
			      }
			   }// end of if(forward)
			   
			   /// Make a slicer to extract just this plane
			   /// @todo Enable pol and chan maps
			   const casa::IPosition ipStart(4, 0, 0, imagePol, imageChan);
			   const casa::Slicer slicer(ipStart, onePlane4D);
			   
			   // Lookup the convolution function to be
			   // used for this row, polarisation and channel
			   // cIndex gives the index for this row, polarization and channel. On top of
			   // that, we need to adjust for the oversampling since each oversampled
			   // plane is kept as a separate matrix.
			   const int cInd=fracu+itsOverSample*(fracv+itsOverSample*cIndex(i, pol, chan));
			   ASKAPCHECK(cInd>-1,
					   "Index into convolution functions is less than zero");
			   ASKAPCHECK(cInd<int(itsConvFunc.size()),
					   "Index into convolution functions exceeds number of planes");
			   
			   casa::Matrix<casa::Complex> & convFunc(itsConvFunc[cInd]);
			   
			   casa::Array<casa::Complex> aGrid(itsGrid[gInd](slicer));
			   casa::Matrix<casa::Complex> grid(aGrid.nonDegenerate());
			   
			   /// Need to check if this point lies on the grid (taking into 
			   /// account the support)
			   if (((iu-itsSupport)>0)&&((iv-itsSupport)>0)&&((iu
							   +itsSupport) <itsShape(0))&&((iv+itsSupport)
							   <itsShape(1))) {
			     if (forward) {
			       // the code never reaches this point for cross-pols due to a continue
			       // operator upstream
			       casa::Complex cVis(acc.visibility()(i, chan, pol));
			       GridKernel::degrid(cVis, convFunc, grid, iu, iv,
						  itsSupport);
			       itsSamplesDegridded+=1.0;
			       itsNumberDegridded+=double((2*itsSupport+1)*(2*itsSupport+1));
                   if(itsVisWeight) {
                      cVis *= itsVisWeight->getWeight(i,frequencyList[chan],pol);
                   }
				     acc.rwVisibility()(i, chan, pol)+=cVis*phasor;
			     } else {
			       if (!isPSFGridder()) {
			           /// Gridding visibility data onto grid
			           casa::Complex rVis = phasor*conj(acc.visibility()(i, chan, pol));
                       if(itsVisWeight) {
				          rVis *= itsVisWeight->getWeight(i,frequencyList[chan],pol);
				       }
				   
			           GridKernel::grid(grid, convFunc, rVis, iu, iv, itsSupport);
			           
			           itsSamplesGridded+=1.0;
			           itsNumberGridded+=double((2*itsSupport+1)*(2*itsSupport+1));
			       
				       ASKAPCHECK(itsSumWeights.nelements()>0, "Sum of weights not yet initialised");
				       ASKAPCHECK(cIndex(i,pol,chan) < int(itsSumWeights.shape()(0)), "Index " << cIndex(i,pol,chan) << " greater than allowed " << int(itsSumWeights.shape()(0)));
				       ASKAPDEBUGASSERT(imagePol < int(itsSumWeights.shape()(1)));
				       ASKAPDEBUGASSERT(imageChan < int(itsSumWeights.shape()(2)));
				
				       itsSumWeights(cIndex(i,pol,chan), imagePol, imageChan)+=1.0;
				   }
				   /// Grid PSF?
				   if (isPSFGridder() && (itsUseAllDataForPSF || ((itsFeedUsedForPSF == acc.feed1()(i)) &&
				             (itsPointingUsedForPSF.separation(acc.dishPointing1()(i))<1e-6)))) {
				      casa::Complex uVis(1.,0.);
				      if(itsVisWeight) {
                         uVis *= itsVisWeight->getWeight(i,frequencyList[chan],pol);
                      }
                      GridKernel::grid(grid, convFunc, uVis, iu, iv, itsSupport);
                      
                      itsSamplesGridded+=1.0;
			          itsNumberGridded+=double((2*itsSupport+1)*(2*itsSupport+1));
			       
				      ASKAPCHECK(itsSumWeights.nelements()>0, "Sum of weights not yet initialised");
				      ASKAPCHECK(cIndex(i,pol,chan) < int(itsSumWeights.shape()(0)), "Index " << cIndex(i,pol,chan) << " greater than allowed " << int(itsSumWeights.shape()(0)));
				      ASKAPDEBUGASSERT(imagePol < int(itsSumWeights.shape()(1)));
				      ASKAPDEBUGASSERT(imageChan < int(itsSumWeights.shape()(2)));
				
				      itsSumWeights(cIndex(i,pol,chan), imagePol, imageChan)+=1.0;
                      
                   } // end if psf needs to be done
			     } // end if forward (else case, reverse operation)
			   }
		     }//end of pol loop
		   } else { 
		       if (!forward) {
		           itsVectorsFlagged+=1;
		       } 
		   } // end of if (allPolGood), else statement
	   }//end of chan loop
   }//end of i loop
   if (forward) {
	   itsTimeDegridded+=timer.real();
   } else {
	   itsTimeGridded+=timer.real();
   }
}
void TableVisGridder::degrid(IDataSharedIter& idi) {
	return generic(*idi, true);
}

void TableVisGridder::grid(IDataSharedIter& idi) {
	return generic(*idi, false);
}

/// @brief Find the change in delay required
/// @details
/// @param[in] acc data accessor to take the input data from
/// @param[out] outUVW Rotated uvw
/// @param[out] delay Delay change (m)
/// @note output vectors are resized to match the accessor's number of rows
void TableVisGridder::rotateUVW(const IConstDataAccessor& acc,
		casa::Vector<casa::RigidVector<double, 3> >& outUVW,
		casa::Vector<double>& delay) const {
    const casa::MVDirection tangentPoint(getTangentPoint());
	const casa::MDirection out(tangentPoint, casa::MDirection::J2000);

    const casa::MVDirection imgCentre(getImageCentre());
    
	
	// offsets between image centre and the tangent point	
	const double dl = sin(imgCentre.getLong()-tangentPoint.getLong())*cos(imgCentre.getLat());
	const double dm = sin(imgCentre.getLat())*cos(tangentPoint.getLat()) - 
	      cos(imgCentre.getLat())*sin(tangentPoint.getLat())
	    *cos(imgCentre.getLong()-tangentPoint.getLong());
    
    //const casa::MVPosition imgOffset = casa::MVPosition(getImageCentre())-casa::MVPosition(getTangentPoint());

	const casa::uInt nSamples = acc.uvw().size();
	delay.resize(nSamples);
	outUVW.resize(nSamples);

	const casa::Vector<casa::RigidVector<double, 3> >& uvwVector = acc.uvw();
	const casa::Vector<casa::MVDirection>& pointingDir1Vector =
			acc.pointingDir1();
	for (casa::uInt row=0; row<nSamples; ++row) {
	    //std::cout<<printDirection(pointingDir1Vector(row))<<" "<<printDirection(out.getValue())<<std::endl;
	    
		const casa::RigidVector<double, 3> &uvwRow = uvwVector(row);
		casa::Vector<double> uvw(3);
		/// @todo Decide what to do about pointingDir1!=pointingDir2
		for (int i=0; i<2; ++i) {
			uvw(i)=-1.0*uvwRow(i);
		}
		uvw(2)=uvwRow(2);

		casa::UVWMachine machine(pointingDir1Vector(row), out, false, true);
		machine.convertUVW(delay(row), uvw);
		delay(row)*=-1.0;

		for (int i=0; i<3; ++i) {
		  outUVW(row)(i)=-1.0*uvw(i);
		}
		
		// to account for situation where tangent point is not image centre
		// the following doesn't work for some reason. Need to investigate
		//delay(row)-=imgOffset*casa::MVPosition(uvw);
		delay(row)+=outUVW(row)(0)*dl+outUVW(row)(1)*dm;
	}
}

/// @brief obtain the centre of the image
/// @details This method extracts RA and DEC axes from itsAxes and
/// forms a direction measure corresponding to the middle of each axis.
/// @return direction measure corresponding to the image centre
casa::MVDirection TableVisGridder::getImageCentre() const
{
   casa::Quantum<double> refLon((itsAxes.start("RA")+itsAxes.end("RA"))/2.0, "rad");
   casa::Quantum<double> refLat((itsAxes.start("DEC")+itsAxes.end("DEC")) /2.0, "rad");
   casa::MVDirection out(refLon, refLat);
   return out;	
}

/// @brief obtain the tangent point
/// @details For faceting all images should be constructed for the same tangent
/// point. We currently pass its coordinates in two specialised axes RA-TANGENT and
/// DEC-TANGENT. If these axes are present a proper MVDirection quantity is 
/// extracted and returned, otherwise this method does the same as getImageCentre()
/// @return direction measure corresponding to the tangent point
casa::MVDirection TableVisGridder::getTangentPoint() const
{
   ASKAPCHECK(itsAxes.has("RA-TANGENT") == itsAxes.has("DEC-TANGENT"), 
       "Either both RA and DEC have to be defined for a tangent point or none of them");
   if (itsAxes.has("RA-TANGENT")) {
       // this is the faceting case
       const casa::Quantum<double> refLon(itsAxes.start("RA-TANGENT"), "rad");
       const casa::Quantum<double> refLat(itsAxes.start("DEC-TANGENT"), "rad");
       const casa::MVDirection out(refLon, refLat);
       return out;	     
   }
   // tangent point is not defined. This is not the faceting case - return the image centre
   return getImageCentre();
}

/// @brief Conversion helper function
/// @details Copies in to out expanding double into complex values and
/// padding appropriately if necessary (itsPaddingFactor is more than 1)
/// @param[out] out complex output array
/// @param[in] in double input array
/// @param[in] padding padding factor
void TableVisGridder::toComplex(casa::Array<casa::Complex>& out,
		const casa::Array<double>& in, const int padding) {
	casa::IPosition outShape(in.shape());
    // effectively its in.shape
	const int nx=outShape(0);
	const int ny=outShape(1);

	ASKAPDEBUGASSERT(outShape.nelements()>=2);
	outShape(0) *= padding;
	outShape(1) *= padding;
	out.resize(outShape);
	
	const int xOffset = (outShape(0) - in.shape()(0))/2;
	const int yOffset = (outShape(1) - in.shape()(1))/2;


	casa::ReadOnlyArrayIterator<double> inIt(in, 2);
	casa::ArrayIterator<casa::Complex> outIt(out, 2);
	while (!inIt.pastEnd()&&!outIt.pastEnd()) {
		casa::Matrix<double> inMat(inIt.array());
		casa::Matrix<casa::Complex> outMat(outIt.array());
		for (int iy=0; iy<ny; iy++) {
			for (int ix=0; ix<nx; ix++) {
				outMat(ix + xOffset, iy + yOffset)=casa::Complex(float(inMat(ix,iy)));
			}
		}
		inIt.next();
		outIt.next();
	}
}

/// @brief Conversion helper function
/// @details Copies real part of in into double array and
/// extracting an inner rectangle if necessary (itsPaddingFactor is more than 1)
/// @param[out] out real output array
/// @param[in] in complex input array
/// @param[in] padding padding factor      
void TableVisGridder::toDouble(casa::Array<double>& out,
		const casa::Array<casa::Complex>& in, const int padding) {
	casa::IPosition outShape(in.shape());
	ASKAPDEBUGASSERT(outShape.nelements()>=2);
	outShape(0) /= padding;
	outShape(1) /= padding;
	out.resize(outShape);
	const int nx=outShape(0);
	const int ny=outShape(1);
	
	const int xOffset = (in.shape()(0) - outShape(0))/2;
	const int yOffset = (in.shape()(1) - outShape(1))/2;
	
	casa::ReadOnlyArrayIterator<casa::Complex> inIt(in, 2);
	casa::ArrayIterator<double> outIt(out, 2);
	while (!inIt.pastEnd()&&!outIt.pastEnd()) {
		casa::Matrix<casa::Complex> inMat(inIt.array());
		casa::Matrix<double> outMat(outIt.array());
		for (int iy=0; iy<ny; ++iy) {
			for (int ix=0; ix<nx; ++ix) {
				outMat(ix, iy)=double(casa::real(inMat(ix + xOffset,iy + yOffset)));
			}
		}
		inIt.next();
		outIt.next();
	}
}

void TableVisGridder::initialiseGrid(const scimath::Axes& axes,
		const casa::IPosition& shape, const bool dopsf) {
	itsAxes=axes;
	itsShape=shape;
	ASKAPDEBUGASSERT(shape.nelements()>=2);
	itsShape(0) *= itsPaddingFactor;
	itsShape(1) *= itsPaddingFactor;
	
	configureForPSF(dopsf);

	/// We only need one grid
	itsGrid.resize(1);
	itsGrid[0].resize(itsShape);
	itsGrid[0].set(0.0);
	if (isPSFGridder()) {
		// for a proper PSF calculation
		initRepresentativeFieldAndFeed();
	}

	ASKAPCHECK(itsSumWeights.nelements()>0, "SumWeights not yet initialised");
	itsSumWeights.set(0.0);

	ASKAPCHECK(itsAxes.has("RA")&&itsAxes.has("DEC"),
			"RA and DEC specification not present in axes");

	double raStart=itsAxes.start("RA");
	double raEnd=itsAxes.end("RA");

	double decStart=itsAxes.start("DEC");
	double decEnd=itsAxes.end("DEC");

	itsUVCellSize.resize(2);
	itsUVCellSize(0)=1.0/(raEnd-raStart)/double(itsPaddingFactor);
	itsUVCellSize(1)=1.0/(decEnd-decStart)/double(itsPaddingFactor);

}

/// @brief a helper method to initialize gridding of the PSF
/// @details The PSF is calculated using the data for a
/// representative field/feed only. By default, the first encountered
/// feed/field is chosen. If the same gridder is reused for another
/// sequence of data points a new representative feed/field have to be
/// found. This is done by resetting the cache in initialiseGrid. However,
/// the latter method can be overridden in the derived classes. To avoid
/// a duplication of the code, this helper method resets the representative
/// feed/field cache. It is called from initialiseGrid.
void TableVisGridder::initRepresentativeFieldAndFeed()
{
  itsFirstGriddedVis = true;

  /*
  // temporary code for debuggig
  std::cout<<"TableVisGridder::initRepresentativeFieldAndFeed"<<std::endl;
  itsFirstGriddedVis = false;
  casa::Quantity ra(0.,"rad"), dec(0.,"rad");
  casa::MVAngle::read(ra,"13:32:22.48");
  casa::MVAngle::read(dec,"-042.16.56.93");
  itsPointingUsedForPSF = casa::MVDirection(ra,dec);
  ASKAPLOG_INFO_STR(logger, "Field override for PSF, will use "<<printDirection(itsPointingUsedForPSF));
  itsFeedUsedForPSF = 0;
  // end of temporary code
  */
}

/// This is the default implementation
void TableVisGridder::finaliseGrid(casa::Array<double>& out) {
	ASKAPDEBUGASSERT(itsGrid.size() > 0);
	// buffer for result as doubles
	casa::Array<double> dBuffer(itsGrid[0].shape());
	ASKAPDEBUGASSERT(dBuffer.shape().nelements()>=2);

    /// Loop over all grids Fourier transforming and accumulating
	for (unsigned int i=0; i<itsGrid.size(); i++) {
		casa::Array<casa::Complex> scratch(itsGrid[i].copy());
		fft2d(scratch, false);
		if (i==0) {
			toDouble(dBuffer, scratch);
		} else {
			casa::Array<double> work(dBuffer.shape());
			toDouble(work, scratch);
			dBuffer+=work;
		}
	}
	// Now we can do the convolution correction
	correctConvolution(dBuffer);
	dBuffer*=double(dBuffer.shape()(0))*double(dBuffer.shape()(1));
	out = PaddingUtils::extract(dBuffer,itsPaddingFactor);
}


/// This is the default implementation
void TableVisGridder::finaliseWeights(casa::Array<double>& out) {
	ASKAPDEBUGASSERT(itsShape.nelements() >= 4);

	int nPol=itsShape(2);
	int nChan=itsShape(3);

	ASKAPCHECK(itsSumWeights.nelements()>0, "Sum of weights not yet initialised");
	int nZ=itsSumWeights.shape()(0);

	for (int chan=0; chan<nChan; chan++) {
		for (int pol=0; pol<nPol; pol++) {
			double sumwt=0.0;
			for (int iz=0; iz<nZ; iz++) {
			  //			  float sumConvFunc=real(casa::sum(casa::abs(itsConvFunc[iz])));
			  //			  ASKAPLOG_INFO_STR(logger, "Sum of conv func " << sumConvFunc);
				sumwt+=itsSumWeights(iz, pol, chan);
			}
			ASKAPDEBUGASSERT(out.shape().nelements() == 4);
			casa::IPosition ipStart(4, 0, 0, pol, chan);
			casa::IPosition onePlane(4, out.shape()(0), out.shape()(1), 1, 1);
			casa::Slicer slicer(ipStart, onePlane);
			out(slicer).set(sumwt);
		}
	}
}

void TableVisGridder::initialiseDegrid(const scimath::Axes& axes,
		const casa::Array<double>& in) {
    configureForPSF(false);
	itsAxes=axes;
	itsShape = PaddingUtils::paddedShape(in.shape(),itsPaddingFactor);

	ASKAPCHECK(itsAxes.has("RA")&&itsAxes.has("DEC"),
			"RA and DEC specification not present in axes");

	double raStart=itsAxes.start("RA");
	double raEnd=itsAxes.end("RA");

	double decStart=itsAxes.start("DEC");
	double decEnd=itsAxes.end("DEC");

	itsUVCellSize.resize(2);
	itsUVCellSize(0)=1.0/(raEnd-raStart)/double(itsPaddingFactor);
	itsUVCellSize(1)=1.0/(decEnd-decStart)/double(itsPaddingFactor);

	/// We only need one grid
	itsGrid.resize(1);
	itsGrid[0].resize(itsShape);

	if (casa::max(casa::abs(in))>0.0) {
		itsModelIsEmpty=false;
		casa::Array<double> scratch(itsShape);
		PaddingUtils::extract(scratch, itsPaddingFactor) = in;
		correctConvolution(scratch);
		toComplex(itsGrid[0], scratch);
		fft2d(itsGrid[0], true);
	} else {
		ASKAPLOG_INFO_STR(logger, "No need to degrid: model is empty");
		itsModelIsEmpty=true;
		itsGrid[0].set(casa::Complex(0.0));
	}
}

/// This is the default implementation
void TableVisGridder::finaliseDegrid() {
	/// Nothing to do
}

// This ShPtr should get deep-copied during cloning.
void TableVisGridder::initVisWeights(IVisWeights::ShPtr viswt)
{
	itsVisWeight = viswt;
}

// Customize for the specific type of Visibility weight.
// Input string is whatever is after " image.i " => " image.i.0.xxx " gives " .0.xxx "
// TODO Needs to change when polarisations are properly supported.
void TableVisGridder::customiseForContext(casa::String context)
{
	// RVU : Set up model dependant gridder behaviour
	//       For MFS, gridders for each Taylor term need different VisWeights.
	//  parse the 'context' string, and generate the "order" parameter.
	char corder[2];
	corder[0] = *(context.data()+1); // read the second character to get the order of the Taylor coefficient.
	corder[1] = '\n';
	int order = atoi(corder);
	if(order <0 || order >9) order = 0;
	if(itsVisWeight)
		itsVisWeight->setParameters(order);
}



/// This is the default implementation
int TableVisGridder::cIndex(int row, int pol, int chan) {
	return 0;
}

/// This is the default implementation
int TableVisGridder::gIndex(int row, int pol, int chan) {
	return 0;
}

}

}
