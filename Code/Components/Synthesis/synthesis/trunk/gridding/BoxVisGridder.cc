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

#include <gridding/BoxVisGridder.h>

namespace askap
{
  namespace synthesis
  {

    BoxVisGridder::BoxVisGridder()
    {
    }

    BoxVisGridder::~BoxVisGridder()
    {
    }

		/// Clone a copy of this Gridder
		IVisGridder::ShPtr BoxVisGridder::clone() 
		{
			return IVisGridder::ShPtr(new BoxVisGridder(*this));
		}

		void BoxVisGridder::initIndices(const IConstDataAccessor&) 
		{
		}

    void BoxVisGridder::initConvolutionFunction(const IConstDataAccessor&)
    {
      itsSupport=0;
      itsOverSample=1;
      itsCSize=2*(itsSupport+1)*itsOverSample+1; // 3
      itsCCenter=(itsCSize-1)/2; // 1
      itsConvFunc.resize(1);
      itsConvFunc[0].resize(itsCSize, itsCSize); // 3, 3, 1
      itsConvFunc[0].set(0.0);
      itsConvFunc[0](itsCCenter,itsCCenter)=1.0; // 1,1,0 = 1
    }
    
		void BoxVisGridder::correctConvolution(casa::Array<double>& image)
		{
		}

  }
}
