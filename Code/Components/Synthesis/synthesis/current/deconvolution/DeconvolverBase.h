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

#ifndef I_DECONVOLVERBASE_H
#define I_DECONVOLVERBASE_H
#include <casa/aips.h>
#include <boost/shared_ptr.hpp>

#include <casa/Arrays/Array.h>

#include <string>

#include <deconvolution/DeconvolverState.h>
#include <deconvolution/DeconvolverControl.h>
#include <deconvolution/DeconvolverMonitor.h>

namespace askap {

  namespace synthesis {

    /// @brief Base class for a deconvolver
    /// @details This base class defines a deconvolver used to estimate an
    /// image from a dirty image, psf optionally using a mask and a weights image.
    /// The template argument T is the type, and FT is the transform
    /// e.g. Deconvolver<Double, DComplex>
    /// @ingroup Deconvolver
    template<class T, class FT> class DeconvolverBase {

    public:
      typedef boost::shared_ptr<DeconvolverBase<T, FT> > ShPtr;
  
      virtual ~DeconvolverBase();
  
      /// @brief Construct from dirty image and psf
      /// @detailed Construct a deconvolver from a dirty image and
      /// the corresponding PSF
      /// @param[in] dirty Dirty image (array)
      /// @param[in] psf Point Spread Function (array)
      DeconvolverBase(Array<T>& dirty, Array<T>& psf);

      /// @brief Set the initial model
      /// @detailed Set the model from which iteration will start
      /// @param[in] model Model image (array)
      void setModel(const Array<T>& model);

      /// @brief Get the current model
      /// @detailed Get the current model
      /// @param[in] model Model image (array)
      Array<T>& model() { return itsModel;};

      /// @brief Get the current dirty
      /// @detailed Get the current dirty
      /// @param[in] dirty Dirty image (array)
      Array<T>& dirty() { return itsDirty;};

      /// @brief Update only the dirty image
      /// @detailed Update an existing deconvolver for a changed dirty image
      /// @param[in] dirty Dirty image (array)
      void updateDirty(Array<T>& dirty);

      /// @brief Set the mask image otherwise there is no mask
      /// @detailed The mask image is used to limit where flux is allowed in
      /// the image
      /// @param[in] mask Mask (array)
      void setMask(Array<T> & mask);

      /// @brief Set the weight image
      /// @detailed The weights image (actually the sqrt) is used to 
      /// aid the deconvolution. The weights image is proportional
      /// to 1/sigma**2
      /// @param[in] weights Weights (array)
      void setWeight(Array<T> & weight);

      Array<T> & mask();

      Array<T> & weight();

      boost::shared_ptr<DeconvolverControl<T> > control() const;

      bool setControl(boost::shared_ptr<DeconvolverControl<T> > control);

      boost::shared_ptr<DeconvolverMonitor<T> > monitor() const;

      bool setMonitor(boost::shared_ptr<DeconvolverMonitor<T> > monitor);

      boost::shared_ptr<DeconvolverState<T> > state() const;

      bool setState(boost::shared_ptr<DeconvolverState<T> > state);

      // @brief Perform the deconvolution
      // @detailed This is the main deconvolution method.
      virtual bool deconvolve();

      // @brief Perform the deconvolution
      // @detailed This is the main deconvolution method.
      virtual bool oneIteration();

      /// @brief Initialize the deconvolution
      /// @detailed Initialise e.g. set weighted mask
      virtual void initialise();

      /// @brief Finalise the deconvolution
      /// @detailed Finalise any calculations needed at the end
      /// of iteration
      virtual void finalise();

    protected:
      void validateShapes();

    private:

      Array<T> itsDirty;

      Array<T> itsPSF;

      Array<T> itsModel;

      Array<FT> itsXFR;

      Array<T> itsMask;

      Array<T> itsWeight;

      /// The state of the deconvolver
      boost::shared_ptr<DeconvolverState<T> > itsDS;

      /// The control used for the deconvolver
      boost::shared_ptr<DeconvolverControl<T> > itsDC;

      /// The monitor used for the deconvolver
      boost::shared_ptr<DeconvolverMonitor<T> > itsDM;

    };

  } // namespace synthesis

} // namespace askap

#include <deconvolution/DeconvolverBase.tcc>

#endif  // #ifndef I_DECONVOLVERBASE_H


