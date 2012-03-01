/// @file : testing ways to access Measurement Sets and related information
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
#include <askap_analysis.h>

#include <parallelanalysis/DuchampParallel.h>
#include <analysisutilities/NewArrayMath.h>
#include <analysisutilities/NewArrayPartMath.h>

#include <casa/aipstype.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayPartMath.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Slicer.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

using namespace askap;
using namespace askap::analysis;

ASKAP_LOGGER(logger, "tMedianSmooth.log");

void findSNR(float *input, float *output, float *outmed, float *outmadfm, float *outdiff, float *outin, casa::IPosition shape, casa::IPosition box, size_t loc, bool isSpatial, size_t spatSize, size_t specSize)
{
  casa::Vector<Float> base(shape, input, casa::COPY);
  ASKAPLOG_DEBUG_STR(logger, "Base: " << base);
  casa::Vector<Float> medians = slidingArrayMath(casa::Vector<Float>(shape,input), box, MedianFunc<Float>());
  ASKAPLOG_DEBUG_STR(logger, "Median: " << medians);
  ASKAPLOG_DEBUG_STR(logger, "Base: " << base);
//   casa::Vector<Float> base(shape, input, casa::COPY);
  casa::Vector<Float> madfm = slidingArrayMath(base, box, MadfmFunc<Float>()) / Statistics::correctionFactor;
  //	ASKAPLOG_DEBUG_STR(logger, loc<<":   " << base(IPosition(1,100)) << " " << median(IPosition(1,100)) << " " << madfm(IPosition(1,100)) << "    " << base(IPosition(1,101)) << " " << median(IPosition(1,101)) << " " << madfm(IPosition(1,101)));
  //  casa::Vector<Float> base(shape, input, casa::COPY);
  casa::Vector<Float> mean = slidingArrayMath(base, box, MeanFunc<Float>());
  //  casa::Vector<Float> base(shape, input, casa::COPY);
  casa::Vector<Float> stddev = slidingArrayMath(base, box, StddevFunc<Float>());
  //  casa::Vector<Float> base(shape, input, casa::COPY);
  casa::Vector<Float> sum = slidingArrayMath(base, box, SumFunc<Float>());
  casa::Vector<Float> snr = (base - medians);
  //casa::Vector<Float> snr = (base - mean);
  //   if((isSpatial && loc==100)|| (!isSpatial && loc==(100*150+100))){
  ASKAPLOG_DEBUG_STR(logger, "shape="<<shape<<", box="<<box);
  //	  for(size_t i=0;i<base.size();i++) ASKAPLOG_DEBUG_STR(logger, base.data()+i << "  " << base.data()[i]);
  ASKAPLOG_DEBUG_STR(logger, "Base: " << base);
  ASKAPLOG_DEBUG_STR(logger, "Median: " << medians);
  ASKAPLOG_DEBUG_STR(logger, "MADFM: " << madfm);
  ASKAPLOG_DEBUG_STR(logger, "Mean: " << mean);
  ASKAPLOG_DEBUG_STR(logger, "Stddev: " << stddev);
  ASKAPLOG_DEBUG_STR(logger, "Sum: " << sum);
  //   }
	
  Vector<Float>::iterator baseEnd(base.end());
  Vector<Float>::iterator iterBase(base.begin()),iterMedian(medians.begin()),iterMadfm(madfm.begin()),iterSnr(snr.begin()),iterMean(mean.begin()),iterStddev(stddev.begin());
  int i=0,pos=0;
  //	for(;iterBase!=baseEnd;iterBase++,iterMedian++,iterMadfm++,iterSnr++){
  while(iterBase != baseEnd){
    if(isSpatial) pos = i+loc*spatSize;
    else pos = loc+i*spatSize;
    output[pos] = (*iterMadfm > 0) ? (*iterSnr)/(*iterMadfm) : 0.;
    //	  output[pos] = (*iterStddev > 0) ? output[pos] = (*iterSnr)/(*iterStddev) : 0.;
    outmed[pos] = *iterMedian;
    outmadfm[pos] = *iterMadfm;
    // 	  outmed[pos] = *iterMean;
    // 	  outmadfm[pos] = *iterStddev;
    outdiff[pos] = *iterSnr;
    outin[pos] = *iterBase;
    //outin[pos] = input[i];
    i++;
    iterBase++;
    iterMedian++;
    iterMadfm++;
    iterMean++;
    iterStddev++;
    iterSnr++;
  }
	
	
}



int main(int argc, const char *argv[])
{
  // This class must have scope outside the main try/catch block
  askap::mwcommon::AskapParallel comms(argc, argv);
  try {

    int width = 1;

    const size_t spatsize=1,specsize=50;
//     float input[specsize]={-0.00876193, -0.00555512, -0.00423819, -0.00253079, -0.0017712, 
// 			   -0.00402222, -0.000307184, 5.3473e-05, -0.00039965, -0.0119562, 
// 			   0.00140414, -0.000558144, 0.00290469, -0.0147037, 0.00373708, 
// 			   -0.0118633, 0.00633639, 0.00634123, -0.00291006, 0.00357627, 
// 			   -0.00648157, -0.00849306, -0.00241731, -0.00158923, 0.00812297, 
// 			   0.00595202, -0.00693134, 0.00807557, 0.00835577, 0.00839215, 
// 			   0.00421789, -0.0102947, 0.00122658, -0.00445886, 0.00868195, 
// 			   0.00881673, 0.00537492, -0.00470855, -0.00549969, 0.00711248, 
// 			   -0.0043232, -0.00229182, -0.00771517, -0.00707709, -0.00497447, 
// 			   -0.000660667, 0.000178434, 0.00411235, 0.00283374, -9.37844e-05};

    float input[specsize];
    for(size_t i=0;i<specsize;i++) input[i]=i%5+(i/5)*0.01;
  
    casa::IPosition box(1,width);
    casa::IPosition shape(1,specsize);

    casa::Array<Float> inputAsArray(shape,input);
    ASKAPLOG_DEBUG_STR(logger, "Input as a casa::Array: " << inputAsArray);
    
    casa::Vector<Float> inputAsVector(shape,input);
    ASKAPLOG_DEBUG_STR(logger, "Input as a casa::Vector: " << inputAsVector);


    float *snrAll = new float[specsize];
    float *medAll = new float[specsize];
    float *madfmAll = new float[specsize];
    float *diffAll = new float[specsize];
    float *inputAll = new float[specsize];
  
  
    findSNR(input,snrAll,medAll,madfmAll,diffAll,inputAll,shape,box,0,false,spatsize,specsize);

  } catch (askap::AskapError& x) {
    ASKAPLOG_FATAL_STR(logger, "Askap error in " << argv[0] << ": " << x.what());
    std::cerr << "Askap error in " << argv[0] << ": " << x.what() << std::endl;
    exit(1);
  } catch (const duchamp::DuchampError& x) {
    ASKAPLOG_FATAL_STR(logger, "Duchamp error in " << argv[0] << ": " << x.what());
    std::cerr << "Duchamp error in " << argv[0] << ": " << x.what() << std::endl;
    exit(1);
  } catch (std::exception& x) {
    ASKAPLOG_FATAL_STR(logger, "Unexpected exception in " << argv[0] << ": " << x.what());
    std::cerr << "Unexpected exception in " << argv[0] << ": " << x.what() << std::endl;
    exit(1);
  }

  exit(0);
}
