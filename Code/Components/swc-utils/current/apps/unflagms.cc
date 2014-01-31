/// @file 
///
/// @brief unflags all visibilities for a given MS
/// @details This application is intended to fix flag column. It unflags all records 
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
/// @author Max Voronkov <maxim.voronkov@csiro.au>

// own includes
#include <askap/AskapError.h>
#include <askap_swcorrelator.h>
#include <askap/AskapLogging.h>

// casa includes
#include <casa/OS/Timer.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableError.h>
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ScalarColumn.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/Arrays/ArrayMath.h>



// other 3rd party
#include <askapparallel/AskapParallel.h>
#include <CommandLineParser.h>

ASKAP_LOGGER(logger, ".unflag");

using namespace askap;
//using namespace askap::swcorrelator;

void process(const std::string &fname) 
{
  ASKAPLOG_INFO_STR(logger,  "Unflagging all data for "<<fname);
  casa::Table ms(fname, casa::Table::Update);
  
  casa::ArrayColumn<casa::Bool> flagCol(ms, "FLAG");
  casa::ScalarColumn<casa::Int> ant1(ms, "ANTENNA1");
  casa::ScalarColumn<casa::Int> ant2(ms, "ANTENNA2");

  
  // to load channel list from a file
  std::vector<int> channels;
  {
     std::ifstream is("flags.dat"); 
     ASKAPCHECK(is, "Unable to open flags.dat");
     while (is) {
        int buf;
        is >> buf;
        if (is) {
           channels.push_back(buf);
        }
     }
  }
  
  ASKAPLOG_INFO_STR(logger,"Total number of rows in the measurement set: "<<ms.nrow());

  for (casa::uInt row = 0; row<ms.nrow(); ++row) {
           
       if ((ant1.get(row) != 0) || (ant2.get(row) != 1)) {
           //continue;
       }
       casa::Array<casa::Bool> buf;
       flagCol.get(row,buf);

       // to unflag
       //buf.set(false);

       /*
       // to flag certain rows
       if (row >= 75006) {
           buf.set(true);
           continue;
       }
       //
       */
       
       
       // to flag channels based on a file
       for (size_t i = 0; i<channels.size(); ++i) {
            casa::Matrix<casa::Bool> thisRow(buf);
            // order reversed w.r.t. the accessor
            ASKAPASSERT(channels[i] < int(thisRow.ncolumn()));
            thisRow.column(channels[i]).set(true);
       }
       //
       

       flagCol.put(row,buf);
  }
}


// Main function
int main(int argc, const char** argv)
{
    // This class must have scope outside the main try/catch block
    askap::askapparallel::AskapParallel comms(argc, argv);
    
    try {
       casa::Timer timer;
       timer.mark();
       
       cmdlineparser::Parser parser; // a command line parser
       // command line parameter
       cmdlineparser::GenericParameter<std::string> msFileName;
       // this parameter is optional
       parser.add(msFileName, cmdlineparser::Parser::throw_exception);

       parser.process(argc, argv);
       
       process(msFileName.getValue());       
       
       ASKAPLOG_INFO_STR(logger,  "Total times - user:   " << timer.user() << " system: " << timer.system()
                          << " real:   " << timer.real());
        ///==============================================================================
    } catch (const cmdlineparser::XParser &ex) {
        ASKAPLOG_FATAL_STR(logger, "Command line parser error, wrong arguments " << argv[0]);
        ASKAPLOG_FATAL_STR(logger, "Usage: " << argv[0] << " measurement_set_to_change");
        return 1;
    } catch (const askap::AskapError& x) {
        ASKAPLOG_FATAL_STR(logger, "Askap error in " << argv[0] << ": " << x.what());
        return 1;
    } catch (const std::exception& x) {
        ASKAPLOG_FATAL_STR(logger, "Unexpected exception in " << argv[0] << ": " << x.what());
        return 1;
    }

    return 0;
    
}


       
