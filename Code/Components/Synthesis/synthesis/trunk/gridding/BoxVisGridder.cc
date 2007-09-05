#include <gridding/BoxVisGridder.h>

namespace conrad
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

		void BoxVisGridder::initIndices(IDataSharedIter& idi) 
		{
		}

    void BoxVisGridder::initConvolutionFunction(IDataSharedIter& idi)
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
