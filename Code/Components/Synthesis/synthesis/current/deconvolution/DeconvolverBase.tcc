/// @file
/// @brief Base class for a deconvolver
/// @details This interface class defines a deconvolver used to estimate an
/// image from a dirty image, psf optionally using a mask and a weights image.
/// @ingroup Deconvolver
///  
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
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///

#include <casa/aips.h>
#include <boost/shared_ptr.hpp>

#include <casa/Arrays/Array.h>

#include <string>

#include <deconvolution/DeconvolverBase.h>

namespace askap {

  namespace synthesis {

    /// @brief Base class for a deconvolver
    /// @details This base class defines a deconvolver used to estimate an
    /// image from a dirty image, psf optionally using a mask and a weights image.
    /// The template argument T is the type, and FT is the transform
    /// e.g. Deconvolver<Double, DComplex>
    /// @ingroup Deconvolver

    template<class T, class FT>
      DeconvolverBase<T,FT>::~DeconvolverBase() {
      };
  
      template<class T, class FT>
      DeconvolverBase<T,FT>::DeconvolverBase(const Array<T>& dirty, const Array<T>& psf)
        : itsDirty(dirty), itsPSF(psf) 
      {
      };

      template<class T, class FT>
      void DeconvolverBase<T,FT>::setModel(const Array<T>& model) {
        itsModel = model;
      }

      template<class T, class FT>
      void DeconvolverBase<T,FT>::updateDirty(const Array<T>& dirty) {
        itsDirty = dirty;
      }

      template<class T, class FT>
      void DeconvolverBase<T,FT>::setMask(const Array<T> & mask) {
    	itsMask = mask;
      }

      template<class T, class FT>
      void DeconvolverBase<T,FT>::setWeight(const Array<T> & weight) {
    	itsWeight = mask;
      }

      template<class T, class FT>
      const Array<T> & DeconvolverBase<T,FT>::mask() const
      {
        return itsMask;
      }

      template<class T, class FT>
      const Array<T> & DeconvolverBase<T,FT>::weight() const
      {
        return itsWeight;
      }

      template<class T, class FT>
      boost::shared_ptr<DeconvolverControl<T> > DeconvolverBase<T,FT>::DC() const
      {
        return itsDC;
      }

      template<class T, class FT>
      void DeconvolverBase<T,FT>::setDC(boost::shared_ptr<DeconvolverControl<T> > DC)
      {
        itsDC = DC;
      }

      template<class T, class FT>
      boost::shared_ptr<DeconvolverMonitor<T> > DeconvolverBase<T,FT>::DM() const
      {
        return itsDM;
      }

      template<class T, class FT>
      void DeconvolverBase<T,FT>::setDM(boost::shared_ptr<DeconvolverMonitor<T> > DM)
      {
        itsDM = DM;
      }

      template<class T, class FT>
      boost::shared_ptr<DeconvolverState<T> > DeconvolverBase<T,FT>::DS() const
      {
        return itsDS;
      }

      template<class T, class FT>
      void DeconvolverBase<T,FT>::setDS(boost::shared_ptr<DeconvolverState<T> > DS)
      {
        itsDS = DS;
      }

  } // namespace synthesis

} // namespace askap


