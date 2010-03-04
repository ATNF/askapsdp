/// @file
///
/// This file contains utilities to search for a support of the convolution function.
/// They could, in principle, be moved to a higher level (to Base), but left here
/// for now as they are not logically a part of fitting, but would introduce
/// a casacore dependence if moved to askap.
///
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
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#include <gridding/SupportSearcher.h>

#include <askap/AskapError.h>
#include <askap/AskapUtil.h>

using namespace askap;
using namespace askap::synthesis;

/// @brief initialize the searcher with some cutoff
/// @details
/// @param[in] cutoff the cutoff value. The meaning could be either relative
/// (with respect to the absolute peak of the image) or absolute, depending
/// on which method is used.
SupportSearcher::SupportSearcher(double cutoff) : itsCutoff(cutoff),
       // scalars mean that the value is undefined 
       itsPeakPos(1,-1), itsBLC(1,-1), itsTRC(1,-1) {}

/// @brief obtain peak position
/// @details The method throws exception if no prior support search has
/// been done.
/// @return peak position determined during the last search for support
casa::IPosition SupportSearcher::peakPos() const
{
  ASKAPCHECK(itsPeakPos.nelements()==2, "peakPos is called prior to the search of the peak");
  return itsPeakPos;
}

/// @brief obtain peak value
/// @details The method throws exception if no prior support search has
/// been done.
/// @return peak value determined during the last search for support
double SupportSearcher::peakVal() const
{
  ASKAPCHECK(itsPeakPos.nelements()==2, "peakVal is called prior to the search of the peak");
  return itsPeakVal;
}

/// @brief obtain the bottom left corner of the support
/// @details This method returns the bottom left corner of the support. It
/// throws an exception of no prior search for support has been done.
/// @return bottom left corner of the support
casa::IPosition SupportSearcher::blc() const
{
  ASKAPCHECK(itsBLC.nelements()==2, "blc() is called prior to the support search");
  return itsBLC;
}

/// @brief obtain the top right corner of the support
/// @details This method returns the bottom left corner of the support. It
/// throws an exception of no prior search for support has been done.
/// @return bottom left corner of the support
casa::IPosition SupportSearcher::trc() const
{
  ASKAPCHECK(itsBLC.nelements()==2, "trc() is called prior to the support search");
  return itsTRC;
}

/// @brief obtain a size of the smallest square support
/// @details This method essentially returns the largest length across both 
/// axes (i.e. max(trc-blc)). It throws an exception if no prior search for
/// support has been done.
/// @return support size
casa::uInt SupportSearcher::support() const
{ 
  const casa::IPosition length = (trc()-blc());
  ASKAPDEBUGASSERT(length.nelements()==2);
  return std::max(length(0),length(1));
}

/// @brief obtain a size of the smallest symmetrical square support
/// @details This method returns the smallest square support, which is
/// symmetrical with respect to the centre.
/// @param[in] shape defines the centre of symmetry (as shape/2)
casa::uInt SupportSearcher::symmetricalSupport(const casa::IPosition &shape) const
{
  const casa::IPosition centre = shape/2;
  const casa::IPosition length1 = trc()-centre;
  const casa::IPosition length2 = centre-blc();
  const int xMax = std::max(abs(length1(0)),abs(length2(0)));
  const int yMax = std::max(abs(length1(1)),abs(length2(1)));
  return casa::uInt(std::max(xMax,yMax)*2);
}

/// @brief search assuming the peak is in the centre
/// @details This search method assumes the peak is in the centre of the
/// image and has a given value. The search starts at the edges and
/// terminated as soon as the absolute value higher than 
/// cutoff*value has been found. Giving value of 1. effectively means 
/// that the cutoff is an absolute cutoff (default).
/// @param[in] in input 2D matrix with an image 
/// @param[in] value assumed peak value
void SupportSearcher::searchCentered(const casa::Matrix<casa::Complex> &in, double value)
{
  itsPeakVal = value;
  itsPeakPos.resize(in.shape().nelements(), casa::False);
  ASKAPDEBUGASSERT(itsPeakPos.nelements() == 2);
  itsPeakPos = in.shape();
  itsPeakPos(0)/=2;
  itsPeakPos(1)/=2;
  doSupportSearch(in);  
}

/// @brief determine the peak and its position
/// @details This method fillss only itsPeakPos and itsPeakVal. It is
/// normally called from one of the search methods, but could be called
/// separately.
/// @param[in] in input 2D matrix with an image
void SupportSearcher::findPeak(const casa::Matrix<casa::Complex> &in)
{ 
  itsPeakPos.resize(in.shape().nelements(),casa::False);
  itsPeakPos = 0;
  itsPeakVal = -1.;
  for (int iy=0;iy<int(in.ncolumn());++iy) {
       for (int ix=0;ix<int(in.nrow());++ix) {
            const double curVal = casa::abs(in(ix,iy));
            if(itsPeakVal< curVal) {
               itsPeakPos(0)=ix;
               itsPeakPos(1)=iy;
               itsPeakVal = curVal;
            }
       }
  }
}

/// @brief full search which determines the peak
/// @details This search method doesn't assume anything about the peak and
/// searches for its position and peak beforehand. The search starts at the
/// edges and progresses towards the peak. The edge of the support region
/// is where the value first time exceeds the cutoff*peakVal.
/// @param[in] in input 2D matrix with an image 
void SupportSearcher::search(const casa::Matrix<casa::Complex> &in)
{
  findPeak(in);
  doSupportSearch(in);
}

/// @brief do actual support search
/// @details This method assumes that peak has already been found and
/// implements the actual search of blc and trc of the support region.
/// @param[in] in input 2D matrix with an image 
void SupportSearcher::doSupportSearch(const casa::Matrix<casa::Complex> &in)
{
  ASKAPDEBUGASSERT(in.shape().nelements() == 2);
  ASKAPDEBUGASSERT(itsPeakPos.nelements() == 2);
  itsBLC.resize(2,casa::False);
  itsTRC.resize(2,casa::False);
  itsBLC = -1;
  itsTRC = -1;
  const double absCutoff = itsCutoff*itsPeakVal;
  for (int ix = 0; ix<itsPeakPos(0); ++ix) {
       if (casa::abs(in(ix, itsPeakPos(1))) > absCutoff) {
           itsBLC(0) = ix;
           break;
       }
  }
  for (int iy = 0; iy<itsPeakPos(1); ++iy) {
       if (casa::abs(in(itsPeakPos(0),iy)) > absCutoff) {
           itsBLC(1) = iy;
           break;
       }
  }

  for (int ix = int(in.nrow())-1; ix>itsPeakPos(0); --ix) {
       if (casa::abs(in(ix, itsPeakPos(1))) > absCutoff) {
           itsTRC(0) = ix;
           break;
       }
  }
  
  for (int iy = int(in.ncolumn())-1; iy>itsPeakPos(1); --iy) {
       if (casa::abs(in(itsPeakPos(0),iy)) > absCutoff) {
           itsTRC(1) = iy;
           break;
       }
  }
  
  ASKAPCHECK((itsBLC(0)>=0) && (itsBLC(1)>=0) && (itsTRC(0)>=0) && 
             (itsTRC(1)>=0), "Unable to find the support on one of the coorinates. Effective support is 0.");
}

