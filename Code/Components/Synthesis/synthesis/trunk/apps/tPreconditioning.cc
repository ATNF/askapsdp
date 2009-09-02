/// @file 
/// This is a test file intended to study timing/preformance of preconditioning 
///
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

#include <iostream>
#include <stdexcept>
#include <askap_synthesis.h>
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, "");

#include <askap/AskapError.h>
#include <casa/OS/Timer.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/IPosition.h>
#include <boost/shared_ptr.hpp>

#include <measurementequation/IImagePreconditioner.h>
#include <measurementequation/WienerPreconditioner.h>
#include <measurementequation/GaussianNoiseME.h>
#include <measurementequation/SynthesisParamsHelper.h>

#include <mwcommon/MPIConnection.h>

using namespace askap;
using namespace askap::synthesis;

/// @brief class for random number generation
/// @details We could have used casa stuff directly
struct RandomGenerator : public GaussianNoiseME 
{
  explicit RandomGenerator(double variance) :
      GaussianNoiseME(variance, casa::Int(time(0)), 
           casa::Int(askap::mwbase::MPIConnection::getRank())) {}
  using GaussianNoiseME::getRandomComplexNumber;
  float operator()() const { return casa::real(getRandomComplexNumber());}
};

/// @brief fill given array with some rather arbitrary values
void fillArray(casa::Array<float> &in, const RandomGenerator &rg)
{
  // first fill array with the noise
  casa::Vector<float> flattened(in.reform(casa::IPosition(1,in.nelements())));
  for (size_t i=0; i<flattened.nelements(); ++i) {
       flattened[i] = rg();
  }
}

int main(int argc, char **argv) {
  try {
     casa::Timer timer;

     timer.mark();
     // Initialize MPI (also succeeds if no MPI available).
     askap::mwbase::MPIConnection::initMPI(argc, (const char **&)argv);
     RandomGenerator rg(0.01);
     // hard coded parameters of the test
     const casa::Int size = 512;
     const size_t numberOfRuns = 10;
     //
     const casa::IPosition shape(2,size,size);
     
     casa::Array<float> psf(shape);
     fillArray(psf,rg);
     casa::Array<float> img(shape);
     fillArray(img,rg);
          
     std::cerr<<"Image initialization: "<<timer.real()<<std::endl;

     timer.mark();
     
     std::cerr<<"Initialization of preconditioner: "<<timer.real()<<std::endl;       
     
     timer.mark();     
     std::cerr<<"Preconditioning: "<<timer.real()<<std::endl;
     
     timer.mark();     
     SynthesisParamsHelper::saveAsCasaImage("outpsf.casa",psf);
     SynthesisParamsHelper::saveAsCasaImage("outimg.casa",img);     
     std::cerr<<"Storing results: "<<timer.real()<<std::endl;     
  }
  catch(const AskapError &ce) {
     std::cerr<<"AskapError has been caught. "<<ce.what()<<std::endl;
     return -1;
  }
  catch(const std::exception &ex) {
     std::cerr<<"std::exception has been caught. "<<ex.what()<<std::endl;
     return -1;
  }
  catch(...) {
     std::cerr<<"An unexpected exception has been caught"<<std::endl;
     return -1;
  }
  return 0;
}
     