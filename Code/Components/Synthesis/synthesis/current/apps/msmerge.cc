/// @file 
///
/// @brief Experiments with the measurement set
/// @details This is not a general purpose program. 
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
#include <askap_synthesis.h>
#include <askap/AskapLogging.h>



// casa includes
#include <casa/OS/Timer.h>
#include <casa/OS/RegularFile.h>
#include <casa/OS/Directory.h>
#include <casa/OS/File.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableError.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/TiledShapeStMan.h>


// other 3rd party
#include <mwcommon/AskapParallel.h>
#include <Common/ParameterSet.h>
#include <CommandLineParser.h>

ASKAP_LOGGER(logger, ".msmerge");

using namespace askap;

// process one column of the spectral window table
void insert1D(const std::string &name, casa::Table &in, const casa::uInt where, casa::Table &out)
{
  casa::ROArrayColumn<casa::Double> inCol(in, name);
  casa::ArrayColumn<casa::Double> outCol(out, name);
  ASKAPDEBUGASSERT(in.nrow()==1);
  ASKAPDEBUGASSERT(out.nrow()==1);
  casa::Vector<casa::Double> outVal = outCol(0);
  casa::Vector<casa::Double> inVal = inCol(0);
  for (casa::uInt i=0; i<inVal.nelements(); ++i) {
      const casa::uInt targetPos = where*inVal.nelements() + i;
      ASKAPDEBUGASSERT(targetPos < outVal.nelements());
      outVal[targetPos] = inVal[i];
  }
  outCol.put(0,outVal);
}

// change shape of a single array column
template<typename T>
void reShapeColumn(const std::string &name, casa::Table &tab, const casa::uInt factor) 
{
  ASKAPDEBUGASSERT(tab.nrow()>=1);
  ASKAPCHECK(tab.actualTableDesc().isColumn(name), "Column "<<name<<" doesn't appear to exist");
  std::string origName;
  if ((name == "FLAG") || (name == "DATA")) {
      casa::ColumnDesc cd = tab.actualTableDesc().columnDesc(name);
      origName = "OLD_" + name;
      ASKAPLOG_INFO_STR(logger,  "Renaming column "<<name<<" into "<<origName);
      tab.renameColumn(origName,name);
      tab.addColumn(cd);
  }
  typename casa::ArrayColumn<T> col(tab, name);
  
  for (casa::uInt row=0; row<tab.nrow(); ++row) {
       casa::IPosition newShape = (origName.size() ? casa::ROArrayColumn<T>(tab,origName).shape(row) : col.shape(row));
       if (newShape.nelements() == 1) {
           newShape(0) *= factor;
           ASKAPCHECK(tab.nrow() == 1, "Spectral window subtable is supposed to have just one row, you have "<<tab.nrow());
           col.setShape(0,newShape);
           break;
       } else {
          ASKAPCHECK(newShape.nelements() == 2, "Shape for column "<<name<<" is "<<newShape);
          newShape(1) *= factor;
          col.setShape(row,newShape);
       }
  }
  ASKAPLOG_INFO_STR(logger,  "Changed shape of the "<<name<<
     " column in the output dataset/spectral window subtable, factor="<<factor);
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
       cmdlineparser::FlaggedParameter<std::string> outName("-o", "output.ms");
       // this parameter is required
       parser.add(outName, cmdlineparser::Parser::throw_exception);
       if (argc < 4) {
           throw cmdlineparser::XParser();
       }
       std::vector<cmdlineparser::GenericParameter<std::string> > inNames(argc-3);
       for (std::vector<cmdlineparser::GenericParameter<std::string> >::iterator it = inNames.begin(); 
            it<inNames.end(); ++it) {
            parser.add(*it);
       }
       parser.process(argc, argv);
       if (!inNames.size()) {
           throw cmdlineparser::XParser();
       } 
       ASKAPLOG_INFO_STR(logger,  "This program merges given measurement sets and writes the output into `"<<outName.getValue()<<"`");
       ASKAPCHECK(!casa::File(outName.getValue()).exists(), "File or table "<<outName.getValue()<<" already exists!");
       ASKAPLOG_INFO_STR(logger,  "First copy "<<inNames[0].getValue()<<" into "<<outName.getValue());
       {
         casa::Table inTab(inNames[0].getValue());
         inTab.deepCopy(outName.getValue(),casa::Table::New);
       }
       if (inNames.size()>1) {
           casa::Table outTab(outName.getValue(), casa::Table::Update);
           casa::Table outSpWin(outTab.keywordSet().asTable("SPECTRAL_WINDOW"));
           ASKAPCHECK(outSpWin.nrow() == 1, "Spectral window subtable is supposed to have just one row");
           reShapeColumn<casa::Bool>("FLAG",outTab,inNames.size());
           reShapeColumn<casa::Complex>("DATA",outTab,inNames.size());
           reShapeColumn<casa::Double>("CHAN_FREQ",outSpWin,inNames.size());
           reShapeColumn<casa::Double>("CHAN_WIDTH",outSpWin,inNames.size());
           reShapeColumn<casa::Double>("EFFECTIVE_BW",outSpWin,inNames.size());
           reShapeColumn<casa::Double>("RESOLUTION",outSpWin,inNames.size());
           outTab.flush();
           outSpWin.flush();
           casa::ArrayColumn<casa::Bool> flag(outTab,"FLAG");
           casa::ArrayColumn<casa::Complex> data(outTab,"DATA");
           
           for (size_t i=0; i<inNames.size(); ++i) {
                ASKAPLOG_INFO_STR(logger,  "Processing "<<inNames[i].getValue());
                casa::Table inTab(inNames[i].getValue());
                casa::ROArrayColumn<casa::Bool> inFlag(inTab,"FLAG");
                casa::ROArrayColumn<casa::Complex> inData(inTab,"DATA");
                ASKAPCHECK(outTab.nrow() == inTab.nrow(), "Number of rows differ, input table has "<<inTab.nrow()<<
                           " rows, we need "<<outTab.nrow());
                for (casa::uInt row=0; row<outTab.nrow(); ++row) {
                     casa::Matrix<casa::Bool> flagVal = flag(row);
                     casa::Matrix<casa::Bool> inFlagVal = inFlag(row);
                     ASKAPDEBUGASSERT(inFlagVal.nrow() == flagVal.nrow());
                     casa::Matrix<casa::Complex> dataVal = data(row);
                     casa::Matrix<casa::Complex> inDataVal = inData(row);
                     ASKAPDEBUGASSERT(inDataVal.nrow() == dataVal.nrow());                     
                     for (casa::uInt x = 0; x<inFlagVal.nrow(); ++x) {
                          for (casa::uInt y = 0; y<inFlagVal.ncolumn(); ++y) {
                               const casa::uInt targetCol = i*inFlagVal.ncolumn()+y;
                               ASKAPCHECK(targetCol < flagVal.ncolumn(), "targetCol = "<<targetCol<<
                                  " is outside shape="<<flagVal.shape()<<" for flags, row="<<row);
                               ASKAPCHECK(targetCol < dataVal.ncolumn(), "targetCol = "<<targetCol<<
                                  " is outside shape="<<dataVal.shape()<<" for data, row="<<row);
                               flagVal(x,targetCol) = inFlagVal(x,y);
                               dataVal(x,targetCol) = inDataVal(x,y);
                          }
                     }
                     flag.put(row,flagVal);
                     data.put(row,dataVal);
                } 
                // update spectral window subtable
                casa::Table inSpWin(inTab.keywordSet().asTable("SPECTRAL_WINDOW"));
                ASKAPCHECK(inSpWin.nrow() == 1, "Spectral window subtable is supposed to have just one row, check "<<inNames[i].getValue());
                insert1D("CHAN_FREQ", inSpWin, i, outSpWin);
                insert1D("CHAN_WIDTH", inSpWin, i, outSpWin);
                insert1D("EFFECTIVE_BW", inSpWin, i, outSpWin);
                insert1D("RESOLUTION", inSpWin, i, outSpWin);                                
           }
       }
       
       ASKAPLOG_INFO_STR(logger,  "Total times - user:   " << timer.user() << " system: " << timer.system()
                          << " real:   " << timer.real());
        ///==============================================================================
    } catch (const cmdlineparser::XParser &ex) {
        ASKAPLOG_FATAL_STR(logger, "Command line parser error, wrong arguments " << argv[0]);
        ASKAPLOG_FATAL_STR(logger, "Usage: " << argv[0] << " -o output.ms inMS1 ... inMSn");
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
