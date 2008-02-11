// @file
// Generate random gains and store them in a parset file
// These gains can then be used to simulate corrupted data.
//

// casa includes
#include <casa/BasicMath/Random.h>
#include <casa/BasicSL/Complex.h>

// own includes
#include <conrad/ConradUtil.h>
#include <conrad/ConradError.h>

// std includes
#include <fstream>
#include <iostream>
#include <cmath>
#include <string>

/// @brief generator of random complex numbers 
/// @details Amplitude is confined in the given bounds
struct ComplexRandomGainGenerator {
  /// @brief initialize generator
  /// @details it generates a random phase and the amplitude
  /// within the given bounds
  /// @param[in] minAmp minimum amplitude
  /// @param[in] maxAmp maximum amplitude
  /// @param[in] reseed true to attempt reading the seed from the file 
  /// @note If reseed is true, this constructor attempts to read 
  /// file .ComplexRandomGainGenerator.seed. If found and two integer numbers
  /// can be read, the inital seed will be set to these numbers. The seed values 
  /// are written back to the file in the destructor. Therefore, the default
  /// behavior is to generate a different set of values for each run of the code
  ComplexRandomGainGenerator(casa::Double minAmp, casa::Double maxAmp, 
                             bool reseed = true);
  
  /// @brief descructor
  /// @details It saves the current seeds into .ComplexRandomGainGenerator.seed
  ~ComplexRandomGainGenerator();   
  
  /// @brief main operator 
  /// @return a random complex number with the amplitude in the given bounds
  casa::Complex operator()() const 
    {return casa::polar(casa::Float(itsAmp()),casa::Float(itsPhase()));}
    
private:
  mutable casa::MLCG itsGen;
  mutable casa::Uniform itsPhase;
  mutable casa::Uniform itsAmp;
};

/// initialize generator, it generates a random phase and the amplitude
/// within the given bounds
/// @param[in] minAmp minimum amplitude
/// @param[in] maxAmp maximum amplitude
/// @param[in] reseed true to attempt reading the seed from the file 
/// @note If reseed is true, this constructor attempts to read 
/// file .ComplexRandomGainGenerator.seed. If found and two integer numbers
/// can be read, the inital seed will be set to these numbers. The seed values 
/// are written back to the file in the destructor. Therefore, the default
/// behavior is to generate a different set of values for each run of the code
ComplexRandomGainGenerator::ComplexRandomGainGenerator(casa::Double minAmp, 
                      casa::Double maxAmp, bool reseed) : itsGen(0,10),
        itsPhase(&itsGen,0.,2.*M_PI), itsAmp(&itsGen,minAmp,maxAmp) 
{
  if (reseed) {
      std::ifstream is(".ComplexRandomGainGenerator.seed");  
      if (is) {
         int seed1=0;
         int seed2=10;
         is>>seed1>>seed2;
         if (is) {
             itsGen.reseed(seed1,seed2);
         }
      }
  }
  
  // take a few values to ensure that the algorithm is stabilized
  // and gives a proper sequence of random numbers
  for (size_t cnt=0; cnt<3; ++cnt) {
       itsGen.asuInt();
  }
}

/// @brief descructor
/// @details It saves the current seeds into .ComplexRandomGainGenerator.seed
ComplexRandomGainGenerator::~ComplexRandomGainGenerator()
{
  std::ofstream os(".ComplexRandomGainGenerator.seed");
  if (os) {
      os<<itsGen.seed1()<<" "<<itsGen.seed2()<<std::endl;
  }
}

/// @brief get the name of the parameter 
/// @details This method forms the name of the gain parameter corresponding
/// to the given feed and antenna
/// @param[in] ant antenna number
/// @param[in] pol polarisation (0 or 1 - translated to g11 and g22)
/// @param[in] feed feed number (-1 means feed-independent)
std::string gainParameterName(casa::uInt ant, casa::uInt pol,
                              casa::Int feed = -1)
{
  std::string res("gain.");
  if (!pol) {
      res+="g11.";
  } else if (pol == 1) {
      res+="g22.";
  } else {
     CONRADTHROW(conrad::ConradError, 
                 "Only parallel hand polarisations are currently supported");
  }
  res+=conrad::utility::toString<casa::uInt>(ant);
  if (feed>=0) {
      res+="."+conrad::utility::toString<casa::uInt>(casa::uInt(feed));
  }
  return res;
}

int main()
{
   const size_t nAnt = 45;
   const size_t nPol = 2;
   const int nFeed = -1;
   ComplexRandomGainGenerator gen(0.7,1.3);
   std::ofstream os("rndgains.in");
   os<<std::endl;
   os<<"# This is an automatically generated file with random complex gains"<<std::endl;
   os<<"# for "<<nAnt<<" antennae and "<<nPol<<" polarisation products"<<std::endl;
   if (nFeed>=0) {
       os<<"# "<<nFeed<<" feeds will be simulated"<<std::endl;
   }
   os<<std::endl;
   
   for (size_t ant = 0; ant<nAnt; ++ant) {
        for (size_t pol = 0; pol<nPol; ++pol) {
             for (int feed = 0; feed<(nFeed<0 ? 1 : nFeed); ++feed) { 
                  casa::Complex gain = gen();
                  os<<gainParameterName(ant,pol, nFeed<0 ? nFeed : feed)<<" = ["
                    <<real(gain)<<","<<imag(gain)<<"]"<<std::endl;
             }
        }
   }
}