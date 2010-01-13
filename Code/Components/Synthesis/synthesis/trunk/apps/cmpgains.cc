/// @file
/// Experiments to find a good way to compare two gain solutions
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

// casa includes
#include <casa/BasicSL/Complex.h>

// own includes
#include <askap/AskapUtil.h>
#include <askap/AskapError.h>
#include <measurementequation/SynthesisParamsHelper.h>

// command line parser
#include <CommandLineParser.h>

int main(int argc, char **argv)
{
   using namespace askap;

   try {
      cmdlineparser::Parser parser; // a command line parser
      // command line parameters
      cmdlineparser::GenericParameter<std::string> gainsFileName1;
      cmdlineparser::GenericParameter<std::string> gainsFileName2;
      
      // required parameters
      parser.add(gainsFileName1);
      parser.add(gainsFileName2);
      
      parser.process(argc,argv);
   }
   catch (const cmdlineparser::XParser &ex) {
      std::cerr<<"Usage: "<<argv[0]<<" gains1.par gains2.par"<<std::endl;
      std::cerr<<"gains1.par and gains2.par two parset files with gains"<<std::endl;
   }
   catch (const std::exception &ex) {
      std::cerr<<ex.what()<<std::endl;
   }       
}

