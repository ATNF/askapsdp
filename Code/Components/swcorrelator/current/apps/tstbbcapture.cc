#include <swcorrelator/CaptureWorker.h>
#include <string>
#include <iostream>
#include <vector>
#include <complex>

using namespace askap;
using namespace askap::swcorrelator;

int main() {
   const std::string fname = "apps/BB/ant0.beam0.chan0.bat4830973926000000.dat";
   std::vector<std::complex<float> > data = CaptureWorker::read(fname);
   for (size_t i = 0; i<10; ++i) {
        std::cout<<int(std::real(data[i]))<<" "<<int(std::imag(data[i]))<<std::endl;
   }
}
