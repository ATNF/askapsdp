/// @file
/// @brief Class for a Fista-based deconvolver
/// @details This interface class defines a deconvolver used to estimate an
/// image from a dirty image, psf optionally using a mask and a weights image.
/// @ingroup Deconvolver
///
/// The FISTA algorithm searches for a minimum L1 solution to the
/// deconvolution problem.
///
/// @code
/// function [Model]=ASKAPdeconv_L1norm(Dirtymap,PSF,center,lambda,niter)
/// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
/// %This function implements FISTA -- a L1 norm based algorithom for solving the deconvolution problem in
/// %radio astronomy
/// %
/// % Details can be found in paper "A Fast Iterative Shrinkage-Thresholding Algorithm for Linear Inverse Problems"
/// 
/// % Dirtymap the blurred image
/// % PSF .... Point Spread Function of the Dirymap,here the psf is surposed to
/// %           be the same size of Dirtymap
/// %                            
/// % center      A vector indicating the peak coordinate of the PSF for
/// % example [129 129]
/// %                                         
/// % lambda  Regularization parameter, the larger the less iterations
/// % required, however, the smaller the finer the reconstruction. 
/// % 
/// % Model the output cleaned image
/// PSF=PSF/sum(sum(PSF)); % 
/// wh=size(Dirtymap) == size(PSF);
/// if wh(1) & wh(2)
/// else
///   error(' The dirtymap and the dirty beam have to be the same size');
/// end
/// 
/// if nargin < 5
///     niter=100;
/// end
/// 
/// [m,n]=size(Dirtymap);
/// % computng the UV mask with the psf         
/// UV=fft2(circshift(PSF,1-center));
/// Fdirty=fft2(Dirtymap);
/// 
/// %Calculate the Lipschitz constant as introduce in the paper
/// L=2*max(max(abs(UV).^2));
/// % initialization
/// X_temp=Dirtymap;
/// X=X_temp;
/// t_new=1;
/// for i=1:niter
///     X_old=X_temp;
///     t_old=t_new;
///     % Gradient
///     D=UV.*fft2(X)-Fdirty;
///     X=real(X-2/L*ifft2(conj(UV).*D));
///     % Soft thresholding 
///     D=abs(X)-lambda/(L);
///     X_temp=sign(X).*((D>0).*D);
///     %updating t and X
///     t_new=(1+sqrt(1+4*t_old^2))/2;
///     X=X_temp+(t_old-1)/t_new*(X_temp-X_old);
///     
/// end
/// Model=X_temp;
/// @endcode
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

#ifndef I_DECONVOLVERFISTA_H
#define I_DECONVOLVERFISTA_H
#include <casa/aips.h>
#include <boost/shared_ptr.hpp>

#include <casa/Arrays/Array.h>

#include <string>

#include <deconvolution/DeconvolverBase.h>

#include <deconvolution/DeconvolverState.h>
#include <deconvolution/DeconvolverControl.h>
#include <deconvolution/DeconvolverMonitor.h>
#include <deconvolution/BasisFunction.h>

namespace askap {

  namespace synthesis {

    /// @brief Class for a deconvolver using the Fista Clean algorithm
    /// @details This base class defines a deconvolver used to estimate an
    /// image from a dirty image, psf optionally using a mask and a weights image.
    /// The template argument T is the type, and FT is the transform
    /// e.g. DeconvolverFista<Double, DComplex>
    /// @ingroup Deconvolver
    template<class T, class FT> class DeconvolverFista : public DeconvolverBase<T, FT> {

    public:
      typedef boost::shared_ptr<DeconvolverFista<T, FT> > ShPtr;
  
      virtual ~DeconvolverFista();
  
      /// @brief Construct from dirty image and psf
      /// @detail Construct a deconvolver from a dirty image and
      /// the corresponding PSF
      /// @param[in] dirty Dirty image (array)
      /// @param[in] psf Point Spread Function (array)
      DeconvolverFista(Array<T>& dirty, Array<T>& psf);

      /// @brief Set the basis function to be used
      /// @details The algorithm can work with different basis functions
      /// PointBasisFunction, MultiScaleBasisFunction. 
      /// @param[in] bf Shared pointer to basisfunction instance
      void setBasisFunction(boost::shared_ptr<BasisFunction<T> > bf);

      /// @brief Return the basis function to be used
      /// @details The algorithm can work with different basis functions
      /// PointBasisFunction, MultiScaleBasisFunction 
      boost::shared_ptr<BasisFunction<T> > basisFunction();

      /// @brief Perform the deconvolution
      /// @detail This is the main deconvolution method.
      virtual bool deconvolve();

      /// @brief Initialize the deconvolution
      /// @detail Initialise e.g. set weighted mask
      virtual void initialise();

      /// @brief configure basic parameters of the solver
      /// @details This method encapsulates extraction of basic solver parameters from the parset.
      /// @param[in] parset parset
      virtual void configure(const LOFAR::ParameterSet &parset); 

    private:

      void updateAlgorithm(Array<T>& delta, const Array<T>& model,
                           const Array<T>& residual, T aFit);

      void W(Array<T>& out, const Array<T>& in);
      void WT(Array<T>& out, const Array<T>& in);

      Array<FT> itsBasisFunctionTransform;

      // We need this for the inner loop
      // Mask weighted by weight image
      Array<T> itsWeightedMask;

      /// Basis function used in the deconvolution
      boost::shared_ptr<BasisFunction<T> > itsBasisFunction;

    };

  } // namespace synthesis

} // namespace askap

#include <deconvolution/DeconvolverFista.tcc>

#endif  // #ifndef I_DECONVOLVERFISTA_H


