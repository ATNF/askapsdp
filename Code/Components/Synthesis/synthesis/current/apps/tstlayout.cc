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
#include <casa/Quanta/MVDirection.h>
#include <casa/Quanta/MVAngle.h>
#include <measures/Measures/MPosition.h>
#include <casa/Arrays/Vector.h>
#include <measurementequation/MEParsetInterface.h>

// std
#include <stdexcept>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>

using namespace askap;
using namespace askap::synthesis;
using namespace askap::mwbase;
using namespace LOFAR;

/// @brief load layout
/// @details
/// @param[in] fname input file name
/// @param[out] x coordinate X
/// @param[out] y coordinate Y
/// @param[out] z coordinate Z
void loadLayout(const std::string &fname, casa::Vector<double> &x, casa::Vector<double> &y, casa::Vector<double> &z)
{
  ParameterSet parset(fname);
  
  const std::string telName = parset.getString("antennas.telescope");
  ASKAPLOG_INFO_STR(logger, "Loading " << telName);
  std::ostringstream oos;
  oos << "antennas." << telName << ".";
  ParameterSet antParset(parset.makeSubset(oos.str()));
  
  ASKAPCHECK(antParset.isDefined("names"), "Subset (antennas."<<telName<<") of the antenna definition parset does not have 'names' keyword.");
  std::vector<string> antNames(antParset.getStringVector("names"));
  const int nAnt = antNames.size();
  ASKAPCHECK(nAnt > 0, "No antennas defined in parset file");
  
  std::string coordinates = antParset.getString("coordinates", "local");
  ASKAPCHECK((coordinates == "local") || (coordinates == "global"), "Coordinates type unknown");

  /// Csimulator.ASKAP.scale=0.333
  const float scale = antParset.getFloat("scale", 1.0);

  /// Now we get the coordinates for each antenna in turn
  casa::Vector<double> xBuf(nAnt);
  casa::Vector<double> yBuf(nAnt);
  casa::Vector<double> zBuf(nAnt);
  
  /// Antenna information in the form:
  /// antennas.ASKAP.antenna0=[x,y,z]
  /// ...
  for (int iant = 0; iant < nAnt; ++iant) {
       std::vector<float> xyz = antParset.getFloatVector(antNames[iant]);
       ASKAPCHECK(xyz.size()>=3, "Error loading ant="<<iant+1<<", xyz.size()="<<xyz.size());
       xBuf[iant] = xyz[0] * scale;
       yBuf[iant] = xyz[1] * scale;
       zBuf[iant] = xyz[2] * scale;
  }

  /// Csimulator.ASKAP.location=[+115deg, -26deg, 192km, WGS84]
  if((coordinates == "local") || (coordinates == "longlat") ) {
     const casa::MPosition location = MEParsetInterface::asMPosition(antParset.getStringVector("location"));
      
     casa::MVAngle mvLong = location.getAngle().getValue()(0);
     casa::MVAngle mvLat = location.getAngle().getValue()(1);

     ASKAPLOG_INFO_STR(logger, "Using local coordinates for the antennas: Reference position = "
                              << mvLong.string(casa::MVAngle::ANGLE, 7) << " "
                              << mvLat.string(casa::MVAngle::DIG2, 7));
     x.resize(xBuf.nelements());                              
     y.resize(yBuf.nelements());                              
     z.resize(zBuf.nelements());                            
     if (coordinates == "local") {  
         Simulator::local2global(x, y, z, location, xBuf, yBuf, zBuf);      
     } else {
      Simulator::longlat2global(x, y, z, location, xBuf, yBuf, zBuf);
     }
  } else if (coordinates == "global") {
      x.assign(xBuf);
      y.assign(yBuf);
      z.assign(zBuf);
      ASKAPLOG_INFO_STR(logger, "Using global coordinates for the antennas");
  } else {
        ASKAPLOG_INFO_STR(logger, "Unknown coordinate system type: " << coordinates);
  }

  ASKAPLOG_INFO_STR(logger, "Successfully defined " << nAnt
          << " antennas of " << telName);
  
}

/// @brief read layout, form baselines
/// @details
/// @param[in] fname input file name
/// @param[out] x coordinate X
/// @param[out] y coordinate Y
/// @param[out] z coordinate Z
void getBaselines(const std::string &fname, casa::Vector<double> &x, casa::Vector<double> &y, casa::Vector<double> &z)
{
  casa::Vector<double> xAnt,yAnt,zAnt;
  loadLayout(fname,xAnt,yAnt,zAnt);
  ASKAPCHECK((xAnt.nelements() == yAnt.nelements()) && (xAnt.nelements() == zAnt.nelements()), 
             "Expect the same number of elements in xAnt, yAnt, zAnt");
  const casa::uInt nBaselines = (xAnt.nelements()*(xAnt.nelements()-1)) / 2;
  x.resize(nBaselines);
  y.resize(nBaselines);
  z.resize(nBaselines);
  for (casa::uInt ant1=0,baseline=0;ant1<xAnt.nelements(); ++ant1) {
       for (casa::uInt ant2=0;ant2<ant1; ++ant2,++baseline) {
            ASKAPASSERT(baseline < nBaselines);
            x[baseline] = xAnt[ant1] - xAnt[ant2];
            y[baseline] = yAnt[ant1] - yAnt[ant2];
            z[baseline] = zAnt[ant1] - zAnt[ant2];            
       }
  }
}

/// @brief analyse the layout
/// @details
/// @param[in] x baseline coordinate X
/// @param[in] y baseline coordinate Y
/// @param[in] z baseline coordinate Z
void analyseBaselines(casa::Vector<double> &x, casa::Vector<double> &y, casa::Vector<double> &z)
{
  casa::Matrix<double> normalMatr(3,3,0.);
  const casa::uInt nBaselines = x.nelements();
  for (casa::uInt b = 0; b<nBaselines; ++b) {
       normalMatr(0,0) += casa::square(x[b]);
       normalMatr(1,1) += casa::square(y[b]);
       normalMatr(2,2) += casa::square(z[b]);
       normalMatr(0,1) += x[b]*y[b];
       normalMatr(0,2) += x[b]*z[b];
       normalMatr(1,2) += y[b]*z[b];
  }
  normalMatr(1,0) = normalMatr(0,1);
  normalMatr(2,0) = normalMatr(0,2);
  normalMatr(2,1) = normalMatr(1,2);
 
  ASKAPLOG_INFO_STR(logger, "Normal matrix: "<<normalMatr);
}

/// @brief main
/// @param[in] argc number of arguments
/// @param[in] argv vector of arguments
/// @retirm status
int main(int argc, char **argv) {
  try {
     cmdlineparser::Parser parser; // a command line parser
     
     // command line parameter
     cmdlineparser::GenericParameter<std::string> cfgName;
     parser.add(cfgName, cmdlineparser::Parser::throw_exception);
     parser.process(argc, argv);
     
     // Initialize MPI (also succeeds if no MPI available).
     askap::mwbase::MPIConnection::initMPI(argc, (const char **&)argv);
     ASKAPLOG_INIT("askap.log_cfg");
     
     casa::Vector<double> x,y,z;
     getBaselines(cfgName,x,y,z);
     analyseBaselines(x,y,z);
     
     askap::mwbase::MPIConnection::endMPI();
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

  