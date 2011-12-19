/// @file 
///
/// @brief recalculates UVWs for a given MS
/// @details This application is intended to fix UVW column. It recalculates UVWs for a given field 
/// centre and time (handy if capture is done with the wrong field direction in the configuration file).
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
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MCEpoch.h>
#include <measures/Measures/MDirection.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableError.h>
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/ArrayColumn.h>
#include <measures/TableMeasures/ScalarMeasColumn.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/MatrixMath.h>



// other 3rd party
#include <mwcommon/AskapParallel.h>
#include <CommandLineParser.h>

ASKAP_LOGGER(logger, ".fixuvw");

using namespace askap;
//using namespace askap::swcorrelator;

casa::MDirection getDirection(const casa::Table &ms)
{
   casa::Table fieldSubtable(ms.keywordSet().asTable("FIELD"));
   ASKAPCHECK(fieldSubtable.nrow() == 1, "FIELD subtable is supposed to have just one row");
   casa::ROScalarMeasColumn<casa::MDirection> dir(fieldSubtable, "PHASE_DIR");
   casa::MDirection result;
   dir.get(0, result);
   return result;
}

casa::Matrix<double> readAntennaPositions(const casa::Table &ms)
{
   casa::Table antSubtable(ms.keywordSet().asTable("ANTENNA"));
   casa::ROArrayColumn<double> pos(antSubtable, "POSITION");
   casa::Matrix<double> result(antSubtable.nrow(), 3);
   ASKAPASSERT(result.nrow() > 0);
   for (casa::uInt i = 0; i<result.nrow(); ++i) {
        result.row(i) = pos(i);
   }
   return result;
}

void process(const std::string &fname) 
{
  ASKAPLOG_INFO_STR(logger,  "Recalculate uvw's (old data will be replaced) for "<<fname);
  casa::Table ms(fname, casa::Table::Update);
  ASKAPCHECK(ms.keywordSet().asTable("FEED").nrow() == ms.keywordSet().asTable("ANTENNA").nrow(), "Only single on axis beam is currently supported");
  casa::MDirection phaseCntr = getDirection(ms);
  casa::Matrix<double> layout = readAntennaPositions(ms);
  
  casa::ROScalarMeasColumn<casa::MEpoch> epochCol(ms,"TIME_CENTROID");
  //casa::ROArrayColumn<double> uvwCol(ms, "UVW");
  casa::ArrayColumn<double> uvwCol(ms, "UVW");
  casa::ROScalarColumn<int> ant1Col(ms, "ANTENNA1");
  casa::ROScalarColumn<int> ant2Col(ms, "ANTENNA2");
  
  for (casa::uInt row = 0; row<ms.nrow(); ++row) {
       casa::MEpoch epoch;
       epochCol.get(row, epoch);
       const int antenna1 = ant1Col(row);
       const int antenna2 = ant2Col(row);       
       
       const double ra = phaseCntr.getAngle().getValue()(0);
       const double dec = phaseCntr.getAngle().getValue()(1);
       const double gmstInDays = casa::MEpoch::Convert(epoch,casa::MEpoch::Ref(casa::MEpoch::GMST1))().get("d").getValue("d");
       const double gmst = (gmstInDays - casa::Int(gmstInDays)) * casa::C::_2pi; // in radians
  
       const double H0 = gmst - ra, sH0 = sin(H0), cH0 = cos(H0), sd = sin(dec), cd = cos(dec);
       // quick and dirty calculation without taking aberration and other fine effects into account
       // it should be fine for the sort of baselines we have with BETA3
       casa::Matrix<double> trans(3, 3, 0.);
       trans(0, 0) = -sH0; trans(0, 1) = -cH0;
       trans(1, 0) = sd * cH0; trans(1, 1) = -sd * sH0; trans(1, 2) = -cd;
       trans(2, 0) = -cd * cH0; trans(2, 1) = cd * sH0; trans(2, 2) = -sd;
       const casa::Matrix<double> antUVW = casa::product(trans,casa::transpose(layout));
       casa::Vector<double> newUVW(3,0.);
       for (casa::uInt dim = 0; dim < newUVW.nelements(); ++dim) {
            newUVW[dim]= antUVW(dim, antenna2) - antUVW(dim,antenna1);
       }
       std::cout<<row<<" "<<newUVW<<" "<<uvwCol(row)<<" "<<newUVW - uvwCol(row)<<" "<<antenna1<<" "<<antenna2<<std::endl;
       uvwCol.put(row,newUVW);       
  }
}


// Main function
int main(int argc, const char** argv)
{
    // This class must have scope outside the main try/catch block
    askap::mwcommon::AskapParallel comms(argc, argv);
    
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


       