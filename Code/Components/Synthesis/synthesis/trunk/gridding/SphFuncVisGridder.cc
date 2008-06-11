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

#include <gridding/SphFuncVisGridder.h>
#include <casa/Arrays/ArrayIter.h>

namespace askap
{
  namespace synthesis
  {

    SphFuncVisGridder::SphFuncVisGridder()
    {
      itsSupport=3;
      itsOverSample=128;
      itsConvFunc.resize(itsOverSample*itsOverSample);

      itsCSize=2*itsSupport+1; // 7;
      itsCCenter=itsSupport; // 3

      /// This must be changed for non-MFS

      for (int fracv=0; fracv<itsOverSample; fracv++)
      {
        for (int fracu=0; fracu<itsOverSample; fracu++)
        {
          const int plane=fracu+itsOverSample*fracv;
          ASKAPDEBUGASSERT(plane>=0 && plane<int(itsConvFunc.size()));
          itsConvFunc[plane].resize(itsCSize, itsCSize);
          itsConvFunc[plane].set(0.0);
          for (int ix=0; ix<itsCSize; ix++)
          {
            double nux=std::abs(double(itsOverSample*(ix-itsCCenter)+fracu))/double(itsSupport*itsOverSample);
            double fx=grdsf(nux)*(1.0-std::pow(nux, 2));
            for (int iy=0; iy<itsCSize; iy++)
            {
              double nuy=std::abs(double(itsOverSample*(iy-itsCCenter)+fracv))/double(itsSupport*itsOverSample);
              double fy=grdsf(nuy)*(1.0-std::pow(nuy, 2));
              itsConvFunc[plane](ix, iy)=fx*fy;
            }
          }
        }
      }
      float volume=casa::sum(casa::real(itsConvFunc[0]));
      ASKAPCHECK(volume>0.0, "Integral of convolution function is zero");
      for (int fracv=0; fracv<int(itsOverSample); fracv++)
      {
        for (int fracu=0; fracu<int(itsOverSample); fracu++)
        {
          const int plane=fracu+itsOverSample*fracv;
          ASKAPDEBUGASSERT(plane>=0 && plane<int(itsConvFunc.size()));
          itsConvFunc[plane]*=casa::Complex(1.0/volume);
        }
      }
    }

    SphFuncVisGridder::~SphFuncVisGridder()
    {
    }

    /// Clone a copy of this Gridder
    IVisGridder::ShPtr SphFuncVisGridder::clone()
    {
      return IVisGridder::ShPtr(new SphFuncVisGridder(*this));
    }

    void SphFuncVisGridder::initIndices(const IConstDataAccessor&)
    {
    }

    /// Initialize the convolution function into the cube. If necessary this
    /// could be optimized by using symmetries.
    void SphFuncVisGridder::initConvolutionFunction(const IConstDataAccessor&)
    {
    }

    void SphFuncVisGridder::correctConvolution(casa::Array<double>& grid)
    {
      casa::Vector<double> ccfx(itsShape(0));
      casa::Vector<double> ccfy(itsShape(1));
      for (int ix=0; ix<itsShape(0); ix++)
      {
        double nux=std::abs(double(ix-itsShape(0)/2))/double(itsShape(0)/2);
        ccfx(ix)=1.0/grdsf(nux);
      }
      for (int iy=0; iy<itsShape(1); iy++)
      {
        double nuy=std::abs(double(iy-itsShape(1)/2))/double(itsShape(1)/2);
        ccfy(iy)=1.0/grdsf(nuy);
      }

      casa::ArrayIterator<double> it(grid, 2);
      while (!it.pastEnd())
      {
        casa::Matrix<double> mat(it.array());
        for (int ix=0; ix<itsShape(0); ix++)
        {
          for (int iy=0; iy<itsShape(1); iy++)
          {
            mat(ix, iy)*=ccfx(ix)*ccfy(iy);
          }
        }
        it.next();
      }
    }

    // find spheroidal function with m = 6, alpha = 1 using the rational
                  // approximations discussed by fred schwab in 'indirect imaging'.
                  // this routine was checked against fred's sphfn routine, and agreed
                  // to about the 7th significant digit.
                  // the gridding function is (1-nu**2)*grdsf(nu) where nu is the distance
                  // to the edge. the grid correction function is just 1/grdsf(nu) where nu
                  // is now the distance to the edge of the image.
                  double SphFuncVisGridder::grdsf(double nu)
                  {

                    double top, bot, delnusq, nuend;
                    int k, part;
                    int np, nq;
                    np=4;
                    nq=2;
                    double p[2][5] =
                      { {8.203343e-2, -3.644705e-1, 6.278660e-1, -5.335581e-1, 2.312756e-1},
                        {4.028559e-3, -3.697768e-2, 1.021332e-1, -1.201436e-1, 6.412774e-2}};
                    double q[2][3]=
                    { {1.0000000, 8.212018e-1, 2.078043e-1}, {1.0000000, 9.599102e-1,
                      2.918724e-1}};
                    double value = 0.0;

                    if ((nu>=0.0)&&(nu<0.75))
                    {
                      part = 0;
                      nuend = 0.75;
                    }
                    else if ((nu>=0.75)&&(nu<=1.00))
                    {
                      part = 1;
                      nuend = 1.00;
                    }
                    else
                    {
                      value = 0.0;
                      return value;
                    }

                    top = p[part][0];
                    bot = q[part][0];
                    delnusq = std::pow(nu, 2) - std::pow(nuend, 2);
                    for (k = 1; k<= np; k++)
                    {
                      double factor=std::pow(delnusq, k);
                      top += p[part][k] * factor;
                    }
                    for (k = 1; k<= nq; k++)
                    {
                      double factor=std::pow(delnusq, k);
                      bot += q[part][k] * factor;
                    }
                    if (bot!=0.0)
                    {
                      value = top/bot;
                    }
                    else
                    {
                      value = 0.0;
                    }
                    if (value<0.0)
                    value=0.0;
                    return value;
                  }

                }
              }
