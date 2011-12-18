// test application to generate dummy beamformer weights and store them 
// in the required binary format
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <complex>
#include <inttypes.h>

using namespace std;

int main()
{
 try {
   /*{
   ifstream is("apps/tstweight.bin");
   //ifstream is("apps/weight_x_01.bin");
   for (size_t i = 0; i<192; ++i) {
      int32_t buf = 0;
      float reBuf = 0., imBuf = 0.;
      is.read((char*)&buf,sizeof(int32_t));
      reBuf = float(buf & 0x3fff);
      imBuf = float((buf & 0xfffc) >> 14);
      if (is) {
          //std::cout<<i+1<<" "<<buf<<std::endl;
          std::cout<<i+1<<" "<<std::complex<float>(reBuf,imBuf)<<std::endl;
      }
   }
   }
   */
   ofstream os("apps/tstweight.bin");
   const size_t nBeams = 4;
   for (size_t beam=0; beam<nBeams; ++beam) {
        for (size_t port=0; port<192; ++port) {
             int32_t wt = 0;
             if ((port == 46) && (beam == 0)) {
                 wt = 1;
             } else if ((port == 190) && (beam == 1)) {
                 wt = 1;
             } else if ((port == 46) && (beam == 2)) {
                 wt = 1;
             } else if ((port == 191) && (beam == 3)) {
                 wt = 1;
             }
             os.write((char*)&wt, sizeof(int32_t));
        }
   }
 }
 catch(const std::exception &ex) {
   std::cerr<<ex.what()<<std::endl;
   return -1;
 }
 return 0;
}
