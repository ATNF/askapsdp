/// @file
///
/// @brief helper application to support MRO experiments
/// @details In future, linmos would replace this utility.
/// 
///
/// @copyright (c) 2012 CSIRO
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

// Package level header file
#include "askap_synthesis.h"

// System includes
#include <sstream>
#include <boost/shared_ptr.hpp>

// casa includes
#include "casa/OS/Timer.h"
#include <casa/Arrays/Array.h>

// ASKAPsoft includes
#include <askap/AskapError.h>
#include <askap/AskapLogging.h>
#include <askap/Log4cxxLogSink.h>
#include <measurementequation/SynthesisParamsHelper.h>
#include <Common/ParameterSet.h>
#include <fitting/Params.h>
#include <askap/AskapUtil.h>
#include <imageaccess/IImageAccess.h>
#include <boost/shared_ptr.hpp>

ASKAP_LOGGER(logger, ".testlinmos");

using namespace askap;
using namespace askap::synthesis;

casa::MVDirection convertDir(const std::string &ra, const std::string &dec) {
  casa::Quantity tmpra,tmpdec;
  casa::Quantity::read(tmpra, ra);
  casa::Quantity::read(tmpdec,dec);
  return casa::MVDirection(tmpra,tmpdec);  
}

void process() {
  const casa::MVDirection beam0centre = convertDir("19:41:21.77","-62.11.21.06");
  //const casa::MVDirection beam1centre = convertDir("19:39:25.03","-63.42.45.6");
  const casa::MVDirection beam1centre = convertDir("19:36:17.78","-63.07.50.87");
  const double cutoff = 1e-2;
  const double fwhm = 1.22*3e8/930e6/12;
  

  accessors::IImageAccess& iacc = SynthesisParamsHelper::imageHandler();
  const casa::IPosition shape = iacc.shape("image.beam1");
  const casa::Vector<casa::Quantum<double> > beamInfo = iacc.beamInfo("image.beam1");
  ASKAPCHECK(beamInfo.nelements()>=3, "beamInfo is supposed to have at least 3 elements");
  const casa::CoordinateSystem cs = iacc.coordSys("image.beam1");
  casa::Array<float> pix1 = iacc.read("image.beam1");
  // read the second one
  casa::Array<float> pix2 = iacc.read("image.beam0");
  ASKAPASSERT(pix1.shape() == pix2.shape());
  ASKAPASSERT(pix1.shape().nonDegenerate().nelements() == 2);
  casa::IPosition curpos(pix1.shape());
  for (casa::uInt dim=0; dim<curpos.nelements(); ++dim) {
       curpos[dim] = 0;
  }
  ASKAPASSERT(curpos.nelements()>=2);
  const casa::DirectionCoordinate &dc = cs.directionCoordinate(0);
  casa::Vector<casa::Double> pixel(2,0.);
  casa::MVDirection world;
  for (int x=0; x<pix1.shape()[0];++x) {
       for (int y=0; y<pix2.shape()[1];++y) {
            pixel[0] = double(x);
            pixel[1] = double(y);
            dc.toWorld(world,pixel);
            const double offsetBeam0 = world.separation(beam0centre);
            const double offsetBeam1 = world.separation(beam1centre);
            const double wt0 = exp(-offsetBeam0*offsetBeam0*4.*log(2.)/fwhm/fwhm);
            const double wt1 = exp(-offsetBeam1*offsetBeam1*4.*log(2.)/fwhm/fwhm);
            const double sumsqwt = wt0*wt0 + wt1*wt1;
            curpos[0] = x;
            curpos[1] = y;
            if (sqrt(sumsqwt)<cutoff) {
                pix1(curpos) = 0.;
            } else {
                pix1(curpos) = (pix1(curpos)*wt1 + pix2(curpos)*wt0)/sqrt(sumsqwt);
            }
       }
  }
 
  // write result
  iacc.create("image.result", shape, cs);
  iacc.write("image.result",pix1);
  iacc.setBeamInfo("image.result",beamInfo[0].getValue("rad"), beamInfo[1].getValue("rad"),
                   beamInfo[2].getValue("rad"));
}

// Main function
int main(int argc, const char** argv)
{
    // Now we have to initialize the logger before we use it
    // If a log configuration exists in the current directory then
    // use it, otherwise try to use the programs default one
    std::ifstream config("askap.log_cfg", std::ifstream::in);
    if (config) {
        ASKAPLOG_INIT("askap.log_cfg");
    } else {
        std::ostringstream ss;
        ss << argv[0] << ".log_cfg";
        ASKAPLOG_INIT(ss.str().c_str());
    }

    // Ensure that CASA log messages are captured
    casa::LogSinkInterface* globalSink = new Log4cxxLogSink();
    casa::LogSink::globalSink(globalSink);

    try {
        casa::Timer timer;
        timer.mark();

        SynthesisParamsHelper::setUpImageHandler(LOFAR::ParameterSet());
        
        process();       
        
        ASKAPLOG_INFO_STR(logger,  "Total times - user:   " << timer.user() << " system: " << timer.system()
                << " real:   " << timer.real());
        ///==============================================================================
    } catch (const askap::AskapError& x) {
        ASKAPLOG_FATAL_STR(logger, "Askap error in " << argv[0] << ": " << x.what());
        return 1;
    } catch (const std::exception& x) {
        ASKAPLOG_FATAL_STR(logger, "Unexpected exception in " << argv[0] << ": " << x.what());
        return 1;
    }

    return 0;
}

        
