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

#include <gridding/WStackVisGridder.h>

#include <askap_synthesis.h>
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, ".gridding");

#include <askap/AskapError.h>
#include <askap/AskapUtil.h>

#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>

#include <casa/BasicSL/Constants.h>
#include <fft/FFTWrapper.h>
#include <measurementequation/PaddingUtils.h>

using namespace askap;

#include <cmath>

namespace askap
{
  namespace synthesis
  {

    WStackVisGridder::WStackVisGridder(const double wmax, const int nwplanes)
    {
      ASKAPCHECK(wmax>0.0, "Baseline length must be greater than zero");
      ASKAPCHECK(nwplanes>0, "Number of w planes must be greater than zero");
      ASKAPCHECK(nwplanes%2==1, "Number of w planes must be odd");

      itsNWPlanes=nwplanes;
      itsWScale=wmax;
      if (nwplanes>1) {
          itsWScale/=double((nwplanes-1)/2);
      }
    }

    WStackVisGridder::~WStackVisGridder()
    {
    }
    
    /// @brief copy constructor
    /// @details It is required to decouple internal arrays between
    /// input object and the copy
    /// @param[in] other input object
    WStackVisGridder::WStackVisGridder(const WStackVisGridder &other) :
      SphFuncVisGridder(other), itsWScale(other.itsWScale),
      itsNWPlanes(other.itsNWPlanes), itsGMap(other.itsGMap.copy()) 
    {
    }
    
    
    /// Clone a copy of this Gridder
    IVisGridder::ShPtr WStackVisGridder::clone()
    {
      return IVisGridder::ShPtr(new WStackVisGridder(*this));
    }

    /// Initialize the convolution function into the cube. If necessary this
    /// could be optimized by using symmetries.
    void WStackVisGridder::initIndices(const IConstDataAccessor& acc)
    {
      /// We have to calculate the lookup function converting from
      /// row and channel to plane of the w-dependent convolution
      /// function
      const int nSamples = acc.nRow();
      const int nChan = acc.nChannel();
      const int nPol = acc.nPol();

      itsGMap.resize(nSamples, nPol, nChan);
      int cenw=(itsNWPlanes-1)/2;
      const casa::Vector<casa::RigidVector<double, 3> > &rotatedUVW = acc.rotatedUVW(getTangentPoint());      
      for (int i=0; i<nSamples; ++i)
      {
        const double w=(rotatedUVW(i)(2))/(casa::C::c);
        for (int chan=0; chan<nChan; ++chan)
        {
          for (int pol=0; pol<nPol; pol++)
          {
            const double freq=acc.frequency()[chan];
            /// Calculate the index into the grids
            itsGMap(i, pol, chan)=cenw+nint(w*freq/itsWScale);
            if (itsGMap(i, pol, chan)<0)
            {
              ASKAPLOG_INFO_STR(logger, w << " "<< freq << " "<< itsWScale << " "<< itsGMap(i, pol, chan) );
            }
            ASKAPCHECK(itsGMap(i, pol, chan)<itsNWPlanes,
                "W scaling error: recommend allowing larger range of w, you have w="<<w*freq<<" wavelengths");
            ASKAPCHECK(itsGMap(i, pol, chan)>-1,
                "W scaling error: recommend allowing larger range of w, you have w="<<w*freq<<" wavelengths");
          }
        }
      }
    }

    void WStackVisGridder::initialiseGrid(const scimath::Axes& axes,
        const casa::IPosition& shape, const bool dopsf)
    {
      itsAxes=axes;
      itsShape=shape;
      ASKAPDEBUGASSERT(shape.nelements()>=2);
      itsShape(0) *= paddingFactor();
      itsShape(1) *= paddingFactor();
      
      configureForPSF(dopsf);

      /// We need one grid for each plane
      itsGrid.resize(itsNWPlanes);
      for (int i=0; i<itsNWPlanes; i++)
      {
        itsGrid[i].resize(itsShape);
        itsGrid[i].set(0.0);
      }
      if (isPSFGridder())
      {
        // for a proper PSF calculation
		initRepresentativeFieldAndFeed();
      }
      
      
      ASKAPCHECK(itsAxes.has("RA")&&itsAxes.has("DEC"),
		 "RA and DEC specification not present in axes");

      const double raStart=itsAxes.start("RA");
      const double raEnd=itsAxes.end("RA");

      const double decStart=itsAxes.start("DEC");
      const double decEnd=itsAxes.end("DEC");

      itsUVCellSize.resize(2);
      itsUVCellSize(0)=1.0/(raEnd-raStart)/double(paddingFactor());
      itsUVCellSize(1)=1.0/(decEnd-decStart)/double(paddingFactor());
       
      initialiseSumOfWeights();
      ASKAPCHECK(itsSumWeights.nelements()>0, "SumWeights not yet initialised");      
      
      initialiseFreqMapping();           
    }

    void WStackVisGridder::multiply(casa::Array<casa::Complex>& scratch, int i)
    {
      if(itsWScale==0.0) return;

      /// These are the actual cell sizes used
      float cellx=1.0/(float(itsShape(0))*itsUVCellSize(0));
      float celly=1.0/(float(itsShape(1))*itsUVCellSize(1));

      int nx=itsShape(0);
      int ny=itsShape(1);

      int cenw=(itsNWPlanes-1)/2;
      if(cenw==0) return;
      if(i==cenw) return;

      float w=2.0f*casa::C::pi*float(i-cenw)*itsWScale;
      casa::ArrayIterator<casa::Complex> it(scratch, 2);
      while (!it.pastEnd())
      {
        casa::Matrix<casa::Complex> mat(it.array());

        /// @todo Optimise multiply loop
        for (int iy=0; iy<ny; iy++)
        {
          float y2=float(iy-ny/2)*celly;
          y2*=y2;
          for (int ix=0; ix<nx; ix++)
          {
            if (casa::abs(mat(ix, iy))>0.0)
            {
              float x2=float(ix-nx/2)*cellx;
              x2*=x2;
              float r2=x2+y2;
	      if(r2<1.0) {
		float phase=w*(1.0-sqrt(1.0-r2));
		mat(ix, iy)*=casa::Complex(cos(phase), -sin(phase));
	      }
            }
          }
        }
        it.next();
      }
    }

    /// This is the default implementation
    void WStackVisGridder::finaliseGrid(casa::Array<double>& out)
    {
      if (!isPSFGridder()) {
          ASKAPLOG_INFO_STR(logger, "Stacking " << itsNWPlanes
                          << " planes of W stack to get final image");
      } else {
          ASKAPLOG_INFO_STR(logger, "Stacking " << itsNWPlanes
                          << " planes of W stack to get final PSF");
      }
      ASKAPDEBUGASSERT(itsGrid.size()>0);
      // buffer for the result as doubles
      casa::Array<double> dBuffer(itsGrid[0].shape());
      ASKAPDEBUGASSERT(dBuffer.shape().nelements()>=2);
      
      /// Loop over all grids Fourier transforming and accumulating
      bool first=true;
      for (unsigned int i=0; i<itsGrid.size(); i++)
      {
        if (casa::max(casa::amplitude(itsGrid[i]))>0.0)
        {
          casa::Array<casa::Complex> scratch(itsGrid[i].copy());
          fft2d(scratch, false);
          multiply(scratch, i);

          if (first)
          {
            first=false;
            toDouble(dBuffer, scratch);
          }
          else
          {
            casa::Array<double> work(dBuffer.shape());
            toDouble(work, scratch);
            dBuffer+=work;
          }
        }
      }
      // Now we can do the convolution correction
      correctConvolution(dBuffer);
      dBuffer *= double(dBuffer.shape()(0))*double(dBuffer.shape()(1));
      out = PaddingUtils::extract(dBuffer, paddingFactor());
    }

    void WStackVisGridder::initialiseDegrid(const scimath::Axes& axes,
        const casa::Array<double>& in)
    {

      itsAxes=axes;
      itsShape = PaddingUtils::paddedShape(in.shape(),paddingFactor());
      configureForPSF(false);

      ASKAPCHECK(itsAxes.has("RA")&&itsAxes.has("DEC"),
          "RA and DEC specification not present in axes");

      double raStart=itsAxes.start("RA");
      double raEnd=itsAxes.end("RA");

      double decStart=itsAxes.start("DEC");
      double decEnd=itsAxes.end("DEC");

      itsUVCellSize.resize(2);
      itsUVCellSize(0)=1.0/(raEnd-raStart)/double(paddingFactor());
      itsUVCellSize(1)=1.0/(decEnd-decStart)/double(paddingFactor());
  
      initialiseFreqMapping();      

      itsGrid.resize(itsNWPlanes);
      if (casa::max(casa::abs(in))>0.0)
      {
        itsModelIsEmpty=false;
        ASKAPLOG_INFO_STR(logger, "Filling " << itsNWPlanes
                           << " planes of W stack with model");
        casa::Array<double> scratch(itsShape);
        PaddingUtils::extract(scratch, paddingFactor()) = in;
        correctConvolution(scratch);
        for (int i=0; i<itsNWPlanes; i++)
        {
          itsGrid[i].resize(itsShape);
          toComplex(itsGrid[i], scratch);
          multiply(itsGrid[i], i);
          /// Need to conjugate to get sense of w correction correct
          itsGrid[i]=casa::conj(itsGrid[i]);
          fft2d(itsGrid[i], true);
        }
      }
      else
      {
        itsModelIsEmpty=true;
        ASKAPLOG_INFO_STR(logger, "No need to fill W stack: model is empty");
        for (int i=0; i<itsNWPlanes; i++)
        {
          itsGrid[i].resize(casa::IPosition(1, 1));
          itsGrid[i].set(casa::Complex(0.0));
        }
      }
    }

    int WStackVisGridder::gIndex(int row, int pol, int chan)
    {
      return itsGMap(row, pol, chan);
    }

  }
}
