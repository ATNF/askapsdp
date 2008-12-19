///
/// @file : Match output list eg. from cduchamp with known input list
///
/// Control parameters are passed in from a LOFAR ParameterSet file.
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
/// @author Matthew Whiting <matthew.whiting@csiro.au>
#include <askap_simulations.h>

#include <FITS/FITSfile.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <APS/ParameterSet.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <stdlib.h>
#include <time.h>

using namespace askap;
using namespace askap::simulations;
using namespace askap::simulations::FITS;
using namespace LOFAR::ACC::APS;


ASKAP_LOGGER(logger, "imageQualTest.log");

// Move to Askap Util?
std::string getInputs(const std::string& key, const std::string& def, int argc,
    const char** argv)
{
  if (argc>2)
  {
    for (int arg=0; arg<(argc-1); arg++)
    {
      std::string argument=std::string(argv[arg]);
      if (argument==key)
      {
        return std::string(argv[arg+1]);
      }
    }
  }
  return def;
}

// Main function
int main(int argc, const char** argv)
{
  ASKAPLOG_INIT("cduchamp.log_cfg");

  try
  {

    //    casa::Timer timer;

    //    timer.mark();

    srandomdev();

    std::string parsetFile(getInputs("-inputs", "createFITS.in", argc, argv));

    ParameterSet parset(parsetFile);
    ParameterSet subset(parset.makeSubset("createFITS."));

    bool doNoise = subset.getBool("addNoise",true);
    bool noiseBeforeConvolve = subset.getBool("noiseBeforeConvolve",true);
    bool doConvolution = subset.getBool("doConvolution",true);

    FITSfile file(subset);

    file.addSources();
 
    if(doNoise && (noiseBeforeConvolve || !doConvolution)) 
      file.addNoise();

    if(doConvolution) 
      file.convolveWithBeam();

    if(doNoise && (!noiseBeforeConvolve && doConvolution)) 
      file.addNoise();

    file.saveFile();

  }
  catch (askap::AskapError& x)
  {
    ASKAPLOG_FATAL_STR(logger, "Askap error in " << argv[0] << ": " << x.what());
    std::cerr << "Askap error in " << argv[0] << ": " << x.what() << std::endl;
    exit(1);
  }
  catch (std::exception& x)
  {
    ASKAPLOG_FATAL_STR(logger, "Unexpected exception in " << argv[0] << ": " << x.what());
    std::cerr << "Unexpected exception in " << argv[0] << ": " << x.what() << std::endl;
    exit(1);
  }
  exit(0);
}

