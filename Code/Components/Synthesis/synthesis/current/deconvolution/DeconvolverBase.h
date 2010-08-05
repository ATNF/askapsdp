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

#include <Common/ParameterSet.h>

#include <Common/ParameterSet.h>

namespace askap {

  namespace synthesis {

    /// @brief Base class for a deconvolver
    /// @details This base class defines a deconvolver used to estimate an
    /// image from a dirty image, psf optionally using a mask and a weights image.
    /// The template argument T is the type, and FT is the transform
    /// e.g. Deconvolver<Double, DComplex>
    /// The interface is by Array<T>'s holding the various arrays
    /// Usually the arrays are 2-D. However, in the case of e.g. MSMFS the
    /// third axis will be the taylor terms.
    /// @ingroup Deconvolver
    template<class T, class FT> class DeconvolverBase {

    public:
      typedef boost::shared_ptr<DeconvolverBase<T, FT> > ShPtr;
  
      virtual ~DeconvolverBase();
  
      /// @brief Construct from dirty image and psf
      /// @detail Construct a deconvolver from a dirty image and
      /// the corresponding PSF. Note that both dirty image
      /// and psf can have more than 2 dimensions
      /// @param[in] dirty Dirty image (array)
      /// @param[in] psf Point Spread Function (array)
      DeconvolverBase(Array<T> dirty, Array<T> psf);

      /// @brief Set the initial model
      /// @detail Set the model from which iteration will start
      /// @param[out] model Model image (array)
      void setModel(const Array<T> model);

      /// @brief Get the current model
      /// @detail Get the current model
      /// @param[out] model Model image (array)
      Array<T>& model() { return itsModel;};

      /// @brief Set the initial background
      /// @detail Set the background from which iteration will start
      /// @param[out] background Background image (array)
      void setBackground(const Array<T> background);

      /// @brief Get the current background
      /// @detail Get the current background
      /// @param[out] background Background image (array)
      Array<T>& background() { return itsBackground;};

      /// @brief Get the current dirty
      /// @detail Get the current dirty
      /// @param[out] dirty Dirty image (array)
      Array<T>& dirty() { return itsDirty;};

      /// @brief Get the current PSF
      /// @detail Get the current PSF
      /// @param[out] PSF image (array)
      Array<T>& psf() { return itsPSF;};

      /// @brief Get the current XFR
      /// @detail Get the current XFR
      /// @param[out] XFR image (array)
      Array<FT>& xfr() { return itsXFR;};

      /// @brief Get the current residual
      /// @detail Get the current residual
      /// @param[out] residual image (array)
      Array<T>& residual() { return itsResidual;};

      /// @brief Update only the dirty image
      /// @detail Update an existing deconvolver for a changed dirty image
      /// @param[in] dirty Dirty image (array)
      void updateDirty(Array<T> dirty);

      /// @brief Set the mask image otherwise there is no mask
      /// @detail The mask image is used to limit where flux is allowed in
      /// the image
      /// @param[in] mask Mask (array)
      void setMask(Array<T> mask);

      /// @brief Set the weight image
      /// @detail The weights image (actually the sqrt) is used to 
      /// aid the deconvolution. The weights image is proportional
      /// to 1/sigma**2
      /// @param[in] weights Weights (array)
      void setWeight(Array<T> weight);

      Array<T> & mask();

      Array<T> & weight();

      boost::shared_ptr<DeconvolverControl<T> > control() const;

      bool setControl(boost::shared_ptr<DeconvolverControl<T> > control);

      boost::shared_ptr<DeconvolverMonitor<T> > monitor() const;

      bool setMonitor(boost::shared_ptr<DeconvolverMonitor<T> > monitor);

      boost::shared_ptr<DeconvolverState<T> > state() const;

      bool setState(boost::shared_ptr<DeconvolverState<T> > state);

      // @brief Perform the deconvolution
      // @detail This is the main deconvolution method.
      virtual bool deconvolve();

      /// @brief Update the residuals
      /// @detail Update the residuals for this model
      virtual void updateResiduals(Array<T>& model);

      /// @brief Initialize the deconvolution
      /// @detail Initialise e.g. set weighted mask
      virtual void initialise();

      /// @brief Finalise the deconvolution
      /// @detail Finalise any calculations needed at the end
      /// of iteration
      virtual void finalise();

      /// @brief configure basic parameters of the solver
      /// @details This method encapsulates extraction of basic solver parameters from the parset.
      /// @param[in] parset parset
      virtual void configure(const LOFAR::ParameterSet &parset); 
      
      protected:

      // Validate the various shapes to ensure consistency
      void validateShapes();

      Array<T> itsDirty;

      Array<T> itsResidual;

      Array<T> itsPSF;

      Array<T> itsModel;

      Array<T> itsBackground;

      Array<FT> itsXFR;

      Array<T> itsMask;

      Array<T> itsWeight;

      /// The state of the deconvolver
      boost::shared_ptr<DeconvolverState<T> > itsDS;

      /// The control used for the deconvolver
      boost::shared_ptr<DeconvolverControl<T> > itsDC;

      /// The monitor used for the deconvolver
      boost::shared_ptr<DeconvolverMonitor<T> > itsDM;

      // Peak and location of peak of PSF
      casa::IPosition itsPeakPSFPos;
      T itsPeakPSFVal;

      // We need this for the inner loop
      // Mask weighted by weight image
      Array<T> itsWeightedMask;

    };

  } // namespace synthesis

} // namespace askap

#include <deconvolution/DeconvolverBase.tcc>

#endif  // #ifndef I_DECONVOLVERBASE_H


