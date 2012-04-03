/// @file
///
/// @brief combine a number of images as a linear mosaic
/// @details This is a standalone utility to merge images into
/// a mosaic. Some code/functionality can later be moved into cimager, but
/// for now it is handy to have it separate. 
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
#include <CommandLineParser.h>
#include <measurementequation/SynthesisParamsHelper.h>
#include <Common/ParameterSet.h>
#include <fitting/Params.h>
#include <askap/AskapUtil.h>


ASKAP_LOGGER(logger, ".linmos");

using namespace askap;
using namespace askap::synthesis;

/// @brief do the merge
/// @param[in] parset subset with parameters
void merge(const LOFAR::ParameterSet &parset) {
   // for now just do quick and dirty job by loading all facets into memory,
   // later on it can be changed to a more memory-wise behavior
   
   const std::vector<std::string> inNames = parset.getStringVector("Names");
   const std::string outName = parset.getString("out");
   
   // check for conflicts and load components
   for (std::vector<std::string>::const_iterator ci = inNames.begin(); ci!=inNames.end();++ci) {
        ASKAPCHECK(*ci != outName, "Output image name, "<<outName<<", is present among the inputs");
   }
   scimath::Params params;
   boost::shared_ptr<scimath::Params> pParams(&params, utility::NullDeleter());
   ASKAPLOG_INFO_STR(logger,  "Loading individual facets");
   SynthesisParamsHelper::loadImages(pParams, parset);
      
   ASKAPLOG_INFO_STR(logger,  "Creating a merged image");
   SynthesisParamsHelper::add(params, inNames, outName);
   const casa::DirectionCoordinate dc = SynthesisParamsHelper::directionCoordinate(params,outName);
   
   for (std::vector<std::string>::const_iterator ci = inNames.begin(); ci!=inNames.end();++ci) {
       ASKAPLOG_INFO_STR(logger,  "Adding "<<*ci);
       const casa::Slicer slicer = SynthesisParamsHelper::facetSlicer(params,*ci, dc);
       // conformance cross-check
       const casa::IPosition blc = slicer.start();
       const casa::IPosition trc = slicer.end();
       ASKAPDEBUGASSERT(blc.nelements() >= 2);
       ASKAPDEBUGASSERT(trc.nelements() >= 2);
       ASKAPDEBUGASSERT(blc.nelements() == trc.nelements());
       casa::IPosition newShape(trc);
       for (casa::uInt dim=0; dim<newShape.nelements(); ++dim) {
            newShape(dim) -= blc(dim);
            ++newShape(dim);
       }
       casa::Array<double> inPix = params.value(*ci);
       if (inPix.shape() != newShape) {
           ASKAPLOG_WARN_STR(logger,  "There appears to be a projection mismatch, input facet with shape="<<inPix.shape()<<
                    " is mapped onto a region of the merged image with blc="<<blc<<" and trc="<<trc<< 
                    " shape="<<newShape);
           // we can do regrid here later on
           continue;         
       }
       // insert pixels
       // proper linmos math comes here after we're sure that projection stuff works fine
       params.value(outName)(slicer) = inPix; 
   }   
   SynthesisParamsHelper::saveImageParameter(params,outName,outName);
};

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

        cmdlineparser::Parser parser; // a command line parser
        
        // command line parameter
        cmdlineparser::FlaggedParameter<std::string> inputsPar("-inputs",
                 "linmos.in");
        // this parameter is optional
        parser.add(inputsPar, cmdlineparser::Parser::return_default);
                
        // Process command line options
        parser.process(argc, argv);
        
        const std::string parsetFile = inputsPar;

        LOFAR::ParameterSet parset(parsetFile);
        LOFAR::ParameterSet subset(parset.makeSubset("linmos."));
        SynthesisParamsHelper::setUpImageHandler(subset);
               
        merge(subset);
        
        ASKAPLOG_INFO_STR(logger,  "Total times - user:   " << timer.user() << " system: " << timer.system()
                << " real:   " << timer.real());
        ///==============================================================================
    } catch (const cmdlineparser::XParser &ex) {
        ASKAPLOG_FATAL_STR(logger, "Command line parser error, wrong arguments " << argv[0]);
        ASKAPLOG_FATAL_STR(logger, "Usage: " << argv[0] << " [-inputs linmos.in]");
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

        