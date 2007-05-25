#include <gridding/SphFuncVisGridder.h>

namespace conrad
{
namespace synthesis
{

SphFuncVisGridder::SphFuncVisGridder(IDataSharedIter& idi) : TableVisGridder(idi)
{
    initConvolutionFunction();
}

SphFuncVisGridder::~SphFuncVisGridder()
{
    itsC.resize(0,0,0);
}

void SphFuncVisGridder::initConvolutionFunction() {
    itsSupport=3;
    itsOverSample=128;
    itsCSize=2*(itsSupport+1)*itsOverSample+1;
    itsCCenter=(itsCSize-1)/2;
    itsC.resize(itsCSize, itsCSize, 1);
    for (int ix=0;ix<itsCSize;ix++) {
        double fx=grdsf(double(ix-itsCCenter)/double(itsOverSample));
        for (int iy=0;iy<itsCSize;iy++) {
            double fy=grdsf(double(iy-itsCCenter)/double(itsOverSample));
            itsC(ix,iy,0)=fx*fy;
        }
    }
}

// find spheroidal function with m = 6, alpha = 1 using the rational
// approximations discussed by fred schwab in 'indirect imaging'.
// this routine was checked against fred's sphfn routine, and agreed
// to about the 7th significant digit.
// the gridding function is (1-nu**2)*grdsf(nu) where nu is the distance
// to the edge. the grid correction function is just 1/grdsf(nu) where nu
// is now the distance to the edge of the image.
double SphFuncVisGridder::grdsf (double nu) {
  
  
  double top, bot, delnusq, nuend;
  int k, part;
  int np, nq;
  np=4;
  nq=2;
  double p[2][5]    = {8.203343e-2, -3.644705e-1, 6.278660e-1,
               -5.335581e-1, 2.312756e-1,
               4.028559e-3, -3.697768e-2, 1.021332e-1,
               -1.201436e-1, 6.412774e-2};
  double q[2][3]= {1.0000000, 8.212018e-1, 2.078043e-1,
             1.0000000, 9.599102e-1, 2.918724e-1};
  double value = 0.0;
  
  if ((nu>=0.0)&&(nu<0.75)) {
    part = 1;
    nuend = 0.75;
  }
  else if ((nu>=0.75)&&(nu<=1.00)) {
    part = 2;
    nuend = 1.00;
  }
  else {
    value = 0.0;
    return value;
  }

  top = p[part][0];
  delnusq = std::pow(nu, 2) - std::pow(nuend, 2);
  for (k = 0;k< np;k++) {
    top += p[part][k] * std::pow(delnusq, (k+1));
  }
  bot = q[part][0];
  for (k = 0;k< nq;k++) {
    bot += q[part][k] * std::pow(delnusq, (k+1));
  }
  if (bot!=0.0) {
    value = top/bot;
  }
  else {
    value = 0.0;
  }
  return value;
}

int SphFuncVisGridder::cOffset(int row, int chan) {
    return 0;
}

}
}
