#include <gridding/BoxVisGridder.h>

namespace conrad
{
namespace synthesis
{

BoxVisGridder::BoxVisGridder()
{
    initConvolutionFunction();
}

BoxVisGridder::~BoxVisGridder()
{
    itsC.resize(0,0,0);
}

void BoxVisGridder::initConvolutionFunction() {
    itsSupport=0;
    itsOverSample=1;
    itsCSize=2*(itsSupport+1)*itsOverSample+1;
    itsCCenter=(itsCSize-1)/2;
    itsC.resize(itsCSize, itsCSize, 1);
    itsC.set(0.0);
    itsC(itsCCenter,itsCCenter,0)=1.0;
}

int BoxVisGridder::cOffset(int row, int chan) {
    return 0;
}

}
}
