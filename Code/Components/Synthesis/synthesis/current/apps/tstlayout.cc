///
/// @file
///
/// This program analyses antenna layout (written to investigate snap-shot imaging limitations)
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

#include <askap_synthesis.h>
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, "");

#include <askap/AskapError.h>
#include <CommandLineParser.h>
#include <simulation/Simulator.h>
#include <mwcommon/MPIConnection.h>

// std
#include <stdexcept>
#include <iostream>

using namespace askap;
using namespace askap::synthesis;

int main(int argc, char **argv) {
  try {
     cmdlineparser::Parser parser; // a command line parser
     
     // command line parameter
     cmdlineparser::GenericParameter<std::string> cfgName;
     parser.add(cfgName, cmdlineparser::Parser::throw_exception);
     parser.process(argc, argv);
     
     // Initialize MPI (also succeeds if no MPI available).
     askap::mwbase::MPIConnection::initMPI(argc, (const char **&)argv);
  }
  catch(const cmdlineparser::XParser &) {
     std::cerr<<"Usage "<<argv[0]<<" cfg_name"<<std::endl;
	 return -2;    
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

  