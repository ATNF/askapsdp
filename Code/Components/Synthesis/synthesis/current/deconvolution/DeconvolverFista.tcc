/// @file
/// @brief Class for a deconvolver based on the Fista
/// @details This concrete class defines a deconvolver used to estimate an
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
#include <casa/Arrays/ArrayMath.h>
#include <fft/FFTWrapper.h>

#include <string>

#include <deconvolution/DeconvolverFista.h>

namespace askap {

  namespace synthesis {

    /// @brief Class for a deconvolver based on the Fista Clean
    /// @details This base class defines a deconvolver used to estimate an
    /// image from a dirty image, psf optionally using a mask and a weights image.
    /// The template argument T is the type, and FT is the transform
    /// e.g. DeconvolverFista<Double, DComplex>
    /// @ingroup Deconvolver

    template<class T, class FT>
    DeconvolverFista<T,FT>::~DeconvolverFista() {
    };

    template<class T, class FT>
    DeconvolverFista<T,FT>::DeconvolverFista(Array<T>& dirty, Array<T>& psf)
      : DeconvolverBase<T,FT>::DeconvolverBase(dirty, psf)
    {
      this->model() = this->dirty().copy();
      this->model().set(T(0.0));
    };

    template<class T, class FT>
    void DeconvolverFista<T,FT>::initialise()
    {
      DeconvolverBase<T, FT>::initialise();

      // First deal with the mask
      ASKAPASSERT(this->mask().shape()==this->weight().shape());

      ASKAPLOG_INFO_STR(logger, "Calculating weighted mask");
      itsWeightedMask=this->mask()*sqrt(this->weight()/max(this->weight()));

      ASKAPASSERT(itsWeightedMask.shape()==this->dirty().shape());

      // Now we need to find the peak and support of the PSF
      casa::IPosition minPos;
      casa::IPosition maxPos;
      T minVal, maxVal;
      casa::minMax(minVal, maxVal, minPos, maxPos, this->psf());
      ASKAPLOG_INFO_STR(logger, "Maximum of PSF = " << maxVal << " at " << maxPos);
      ASKAPLOG_INFO_STR(logger, "Minimum of PSF = " << minVal << " at " << minPos);

      // Check the peak of the PSF - it should be at nx/2,ny/2
      if(!(maxPos==this->psf().shape()/2)) {
	throw(AskapError("Peak of PSF is not at center"));
      };

      // Needed for FISTA

      itsXFR.resize(this->psf().shape());
      itsXFR.set(FT(0.0));
      casa::setReal(itsXFR, this->psf());
      scimath::fft2d(itsXFR, true);

      itsVis.resize(this->dirty().shape());
      itsVis.set(FT(0.0));
      casa::setReal(itsVis, this->dirty());
      scimath::fft2d(itsVis, true);

    // L=2*max(max(abs(UV).^2));
      itsLipschitz=casa::max(casa::real(itsXFR*conj(itsXFR)));
      itsLipschitz=2.0*itsLipschitz;

      ASKAPLOG_INFO_STR(logger, "Lipschitz number = " << itsLipschitz);

    }

    // This contains the heart of the Fista algorithm
    // function [Model]=ASKAPdeconv_L1norm(Dirtymap,PSF,center,lambda,niter)
    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    // %This function implements FISTA -- a L1 norm based algorithom for solving the deconvolution problem in
    // %radio astronomy
    // %
    // % Details can be found in paper "A Fast Iterative Shrinkage-Thresholding Algorithm for Linear Inverse Problems"
    // 
    // % Dirtymap the blurred image
    // % PSF .... Point Spread Function of the Dirymap,here the psf is surposed to
    // %           be the same size of Dirtymap
    // %                            
    // % center      A vector indicating the peak coordinate of the PSF for
    // % example [129 129]
    // %                                         
    // % lambda  Regularization parameter, the larger the less iterations
    // % required, however, the smaller the finer the reconstruction. 
    // % 
    // % Model the output cleaned image
    // PSF=PSF/sum(sum(PSF)); % 
    // wh=size(Dirtymap) == size(PSF);
    // if wh(1) & wh(2)
    // else
    //   error(' The dirtymap and the dirty beam have to be the same size');
    // end
    // 
    // if nargin < 5
    //     niter=100;
    // end
    // 
    // [m,n]=size(Dirtymap);
    // % computng the UV mask with the psf         
    // UV=fft2(circshift(PSF,1-center));
    // Fdirty=fft2(Dirtymap);
    // 
    // %Calculate the Lipschitz constant as introduce in the paper
    // L=2*max(max(abs(UV).^2));
    // % initialization
    // X_temp=Dirtymap;
    // X=X_temp;
    // t_new=1;
    // for i=1:niter
    //     X_old=X_temp;
    //     t_old=t_new;
    //     % Gradient
    //     D=UV.*fft2(X)-Fdirty;
    //     X=real(X-2/L*ifft2(conj(UV).*D));
    //     % Soft thresholding 
    //     D=abs(X)-lambda/(L);
    //     X_temp=sign(X).*((D>0).*D);
    //     %updating t and X
    //     t_new=(1+sqrt(1+4*t_old^2))/2;
    //     X=X_temp+(t_old-1)/t_new*(X_temp-X_old);
    //     
    // end
    // Model=X_temp;

    template<class T, class FT>
    bool DeconvolverFista<T,FT>::deconvolve()
    {

      this->initialise();

      bool isMasked(itsWeightedMask.shape()==this->dirty().shape());

      Array<T> X, Xold, XTemp;
      Array<FT> Xfft, XResidual;
      Array<FT> D;
      Array<T> Dfft;

      XTemp=this->dirty().copy();
      X=XTemp;
      T tnew, told;

      tnew=T(1.0);

      ASKAPLOG_INFO_STR(logger, "Performing Fista for " << this->control()->targetIter() << " iterations");
      do {
        Xold=XTemp.copy();
        told=tnew;
        Xfft.resize(X.shape());
        Xfft.set(FT(0.0));

        // Find residuals for current model X
        casa::setReal(Xfft, X);
        scimath::fft2d(Xfft, true);
        D=itsXFR*Xfft-itsVis;
        XResidual.resize(X.shape());
        XResidual=(conj(itsXFR)*D);
        scimath::fft2d(XResidual, false);

        // Find peak in residual image
        casa::IPosition minPos;
        casa::IPosition maxPos;
        T minVal, maxVal;
        if (isMasked) {
          casa::minMaxMasked(minVal, maxVal, minPos, maxPos, casa::real(XResidual),
                             itsWeightedMask);
        }
        else {
          casa::minMax(minVal, maxVal, minPos, maxPos, casa::real(XResidual));
        }
        //
        ASKAPLOG_INFO_STR(logger, "Maximum = " << maxVal << " at location " << maxPos);
        ASKAPLOG_INFO_STR(logger, "Minimum = " << minVal << " at location " << minPos);
        T absPeakVal;
        casa::IPosition absPeakPos;
        if(abs(minVal)<abs(maxVal)) {
          absPeakVal=abs(maxVal);
          absPeakPos=maxPos;
        }
        else {
          absPeakVal=abs(minVal);
          absPeakPos=minPos;
        }

        // Now update the current model: this is the Fista update algorithm
        X=X-T(2.0/itsLipschitz)*real(XResidual);
        Dfft=abs(X)-this->control()->lambda()/itsLipschitz;
        XTemp=(Dfft(Dfft>T(0)));
        XTemp*=casa::sign(X);
        tnew=(1.0+sqrt(1.0+4.0*pow(told, 2)))/2.0;
        X=XTemp+T((told-1.0)/tnew)*(XTemp-Xold);
        
        this->state()->setPeakResidual(absPeakVal);
        this->state()->setObjectiveFunction(absPeakVal);
        this->state()->setTotalFlux(sum(X));
        
        this->monitor()->monitor(*(this->state()));
        this->state()->incIter();
      }
      while (!this->control()->terminate(*(this->state())));
      this->model()=XTemp.copy();
      this->dirty()=casa::real(XResidual.copy());
      
      ASKAPLOG_INFO_STR(logger, "Performed Fista for " << this->state()->currentIter() << " iterations");
      
      ASKAPLOG_INFO_STR(logger, this->control()->terminationString());
      
      this->finalise();
      
      return True;
    }
    
    template<class T, class FT>
    bool DeconvolverFista<T,FT>::oneIteration()
    {
      throw(AskapError("oneIteration not implemented"));
    }
  } // namespace synthesis
  
} // namespace askap


