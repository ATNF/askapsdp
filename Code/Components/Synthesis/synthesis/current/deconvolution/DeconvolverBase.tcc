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
#include <deconvolution/DeconvolverState.h>

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
    DeconvolverBase<T,FT>::DeconvolverBase(Array<T>& dirty, Array<T>& psf)
      : itsDirty(dirty), itsPSF(psf)
    {
      itsDS = boost::shared_ptr<DeconvolverState<T> >(new DeconvolverState<T>());
      ASKAPASSERT(itsDS);
      itsDC = boost::shared_ptr<DeconvolverControl<T> >(new DeconvolverControl<T>());
      ASKAPASSERT(itsDC);
      itsDM = boost::shared_ptr<DeconvolverMonitor<T> >(new DeconvolverMonitor<T>());
      ASKAPASSERT(itsDM);
    };
    
    template<class T, class FT>
    void DeconvolverBase<T,FT>::setModel(const Array<T>& model) {
      itsModel = model.copy();
    }
    
    template<class T, class FT>
    void DeconvolverBase<T,FT>::updateDirty(Array<T>& dirty) {
      if (dirty.shape()!=itsDirty.shape()) {
        throw(AskapError("Updated dirty image has different shape"));
      }
      itsDirty = dirty;
    }
    
    template<class T, class FT>
    bool DeconvolverBase<T,FT>::deconvolve()
    {
      throw(AskapError("Called base class deconvolver"));
    }
    
    template<class T, class FT>
    bool DeconvolverBase<T,FT>::oneIteration()
    {
      throw(AskapError("Called base class single iteration"));
    }
    
    template<class T, class FT>
    void DeconvolverBase<T,FT>::setMask(Array<T> & mask) {
      itsMask = mask;
    }
    
    template<class T, class FT>
    void DeconvolverBase<T,FT>::setWeight(Array<T> & weight) {
      itsWeight = weight;
    }
    
    template<class T, class FT>
    Array<T> & DeconvolverBase<T,FT>::mask()
    {
      return itsMask;
    }
    
    template<class T, class FT>
    Array<T> & DeconvolverBase<T,FT>::weight()
    {
      return itsWeight;
    }
    
    template<class T, class FT>
    boost::shared_ptr<DeconvolverControl<T> > DeconvolverBase<T,FT>::control() const
    {
      ASKAPASSERT(itsDC);
      return itsDC;
    }
    
    template<class T, class FT>
    bool DeconvolverBase<T,FT>::setControl(boost::shared_ptr<DeconvolverControl<T> > DC)
    {
      itsDC = DC;
      ASKAPASSERT(itsDC);
      return True;
    }
    
    template<class T, class FT>
    boost::shared_ptr<DeconvolverMonitor<T> > DeconvolverBase<T,FT>::monitor() const
    {
      ASKAPASSERT(itsDM);
      return itsDM;
    }
    
    template<class T, class FT>
    bool DeconvolverBase<T,FT>::setMonitor(boost::shared_ptr<DeconvolverMonitor<T> > DM)
    {
      itsDM = DM;
      ASKAPASSERT(itsDM);
      return True;
    }
    
    template<class T, class FT>
    boost::shared_ptr<DeconvolverState<T> > DeconvolverBase<T,FT>::state() const
    {
      ASKAPASSERT(itsDS);
      return itsDS;
    }
    
    template<class T, class FT>
    bool DeconvolverBase<T,FT>::setState(boost::shared_ptr<DeconvolverState<T> > DS)
    {
      itsDS = DS;
      ASKAPASSERT(itsDS);
      return True;
    }
    
    template<class T, class FT>
    void DeconvolverBase<T,FT>::validateShapes()
    {
      ASKAPASSERT(this->model().shape()==this->dirty().shape());
      ASKAPASSERT(this->mask().shape()==this->dirty().shape());
      ASKAPASSERT(this->weight().shape()==this->dirty().shape());
    }
    
    template<class T, class FT>
    void DeconvolverBase<T,FT>::initialise()
    {
      this->validateShapes();
    }

    template<class T, class FT>
    void DeconvolverBase<T,FT>::finalise()
    {
    }

  } // namespace synthesis
  
} // namespace askap


