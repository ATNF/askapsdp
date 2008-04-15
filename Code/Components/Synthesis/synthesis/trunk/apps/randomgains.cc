// @file
// Generate random gains and store them in a parset file
// These gains can then be used to simulate corrupted data.
//

// casa includes
#include <casa/BasicMath/Random.h>
#include <casa/BasicSL/Complex.h>

// own includes
#include <askap/AskapUtil.h>
#include <askap/AskapError.h>

// command line parser
#include <CommandLineParser.h>

// std includes
#include <fstream>
#include <iostream>
#include <cmath>
#include <string>
#include <stdexcept>

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
     ASKAPTHROW(askap::AskapError, 
                 "Only parallel hand polarisations are currently supported");
  }
  res+=askap::utility::toString<casa::uInt>(ant);
  if (feed>=0) {
      res+="."+askap::utility::toString<casa::uInt>(casa::uInt(feed));
  }
  return res;
}


int main(int argc, char **argv)
{
   using namespace askap;

   try {
      cmdlineparser::Parser parser; // a command line parser
      // command line parameters
      cmdlineparser::GenericParameter<std::string> outputName;
      cmdlineparser::FlaggedParameter<int> nFeedPar("-f",-1);
      cmdlineparser::FlaggedParameter<size_t> nAntPar("-a",45);
      cmdlineparser::FlaggedParameter<size_t> nPolPar("-p",2);
      cmdlineparser::FlaggedParameter<double> minPar("-min",0.7);
      cmdlineparser::FlaggedParameter<double> maxPar("-max",1.3);
   
      // optional parameters
      parser.add(nFeedPar,cmdlineparser::Parser::return_default);
      parser.add(nAntPar,cmdlineparser::Parser::return_default);
      parser.add(nPolPar,cmdlineparser::Parser::return_default);
      parser.add(minPar,cmdlineparser::Parser::return_default);
      parser.add(maxPar,cmdlineparser::Parser::return_default);
      // required parameters
      parser.add(outputName);
      
      parser.process(argc,argv);
   
      const size_t nAnt = nAntPar;
      const size_t nPol = nPolPar;
      const int nFeed = nFeedPar;
      ASKAPCHECK(minPar<maxPar,"Minimum amplitude should be less than maximum amplitude");
      ComplexRandomGainGenerator gen(minPar,maxPar);
      std::ofstream os(outputName.getValue().c_str());
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
   catch (const cmdlineparser::XParser &ex) {
      std::cerr<<"Usage: "<<argv[0]<<" [-f nFeed] [-a nAnt] [-p nPol] [-min minAmp] [-max maxAmp] outputName"<<std::endl;
      std::cerr<<"-f nFeed    number of feeds, default is feed independent output"<<std::endl;
      std::cerr<<"-a nAnt     number of antennae, default is 45"<<std::endl;
      std::cerr<<"-p nPol     number of polarisations, default is 2"<<std::endl;
      std::cerr<<"-min minAmp minimum amplitude of simulated gains, default is 0.7"<<std::endl;
      std::cerr<<"-max maxAmp maximum amplitude of simulated gains, default is 1.3"<<std::endl;
      std::cerr<<"outputName  output parset file name"<<std::endl;
   }
   catch (const std::exception &ex) {
      std::cerr<<ex.what()<<std::endl;
   }
}
