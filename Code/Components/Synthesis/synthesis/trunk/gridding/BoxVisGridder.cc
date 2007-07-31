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
      itsC.resize(0,0,0);
    }

    void BoxVisGridder::initConvolutionFunction(IDataSharedIter& idi, 
    		casa::Vector<casa::RigidVector<double, 3> >& uvw,
    		const casa::Vector<double>& cellSize,
      const casa::IPosition& shape)
    {
      itsSupport=0;
      itsOverSample=1;
      itsCSize=2*(itsSupport+1)*itsOverSample+1; // 3
      itsCCenter=(itsCSize-1)/2; // 1
      itsC.resize(itsCSize, itsCSize, 1); // 3, 3, 1
      itsC.set(0.0);
      itsC(itsCCenter,itsCCenter,0)=1.0; // 1,1,0 = 1
    }

    int BoxVisGridder::cOffset(int row, int chan)
    {
      return 0;
    }

  }
}
