#include <preprocessing/VariableThresholdingHelpers.h>

#include <casa/Arrays/Array.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/ArrayPartMath.h>

#include <mathsutils/NewArrayPartMath.h>

#include <duchamp/Utils/Statistics.hh>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".varthreshhelp");

namespace askap {

    namespace analysis {

	void findVariableThreshold_OLD(float *input, float *output, float SNRthreshold, casa::IPosition shape, casa::IPosition box, size_t loc, size_t spatSize, size_t specSize, bool isSpatial, bool useRobust)
	{
	    casa::Array<Float> base(shape, input, casa::COPY);
	    casa::Array<Float> middle, spread;
	    if(useRobust){
		ASKAPLOG_DEBUG_STR(logger, "findVariableThreshold: Finding 'middle' array'");
		middle = slidingArrayMath(base, box, MedianFunc<Float>());
		ASKAPLOG_DEBUG_STR(logger, "findVariableThreshold: Finding 'spread' array'");
		spread = slidingArrayMath(base, box, MadfmFunc<Float>()) / Statistics::correctionFactor;
	    }
	    else{
		middle = slidingArrayMath(base, box, MeanFunc<Float>());
		spread = slidingArrayMath(base, box, StddevFunc<Float>());
	    }
	    ASKAPLOG_DEBUG_STR(logger, "findVariableThreshold: Calculating threshold array using SNRthreshold="<<SNRthreshold);
	    casa::Array<Float> threshold = middle + SNRthreshold * spread;
	    casa::Array<Float>::iterator threshEnd(threshold.end()),iterThresh(threshold.begin());
	    int pos=0,i=0;
	    ASKAPLOG_DEBUG_STR(logger, "findVariableThreshold: Preparing to return");
	    ASKAPLOG_DEBUG_STR(logger, "spatSize="<<spatSize <<", loc="<<loc);
	    while (iterThresh != threshEnd){
		pos = isSpatial ?  i+loc*spatSize :  loc+i*spatSize;
		output[pos] = *iterThresh;
		if(i%100==0) ASKAPLOG_DEBUG_STR(logger, i<< " " << pos << " " << output[pos]);
		i++;
		iterThresh++;
	    }
	    ASKAPLOG_DEBUG_STR(logger, "findVariableThreshold: All done");
	
	
	}

	void findNoiseMap_OLD(float *input, float *output, casa::IPosition shape, casa::IPosition box, size_t loc, size_t spatSize, size_t specSize, bool isSpatial, bool useRobust)
	{
	    casa::Array<Float> base(shape, input, casa::COPY);
	    casa::Array<Float> spread;
	    if(useRobust) spread = slidingArrayMath(base, box, MadfmFunc<Float>()) / Statistics::correctionFactor;
	    else spread = slidingArrayMath(base, box, StddevFunc<Float>());
	    Array<Float>::iterator baseEnd(base.end()),iterBase(base.begin()),iterSpread(spread.begin());
	    int i=0,pos=0;
	    while(iterBase != baseEnd){
		pos = isSpatial ?  i+loc*spatSize :  loc+i*spatSize;
		output[pos] = *iterSpread;
		i++;
		iterBase++;
		iterSpread++;
	    }
	}

	void slidingBoxStats(casa::Array<Float> &input, casa::Array<Float> &middle, casa::Array<Float> &spread, casa::IPosition &box, bool useRobust)
	{
	    ASKAPASSERT(input.shape()==middle.shape());
	    ASKAPASSERT(input.shape()==spread.shape());

	    if(useRobust) {
		middle = slidingArrayMath(input, box, MedianFunc<Float>());
		spread = slidingArrayMath(input, box, MadfmFunc<Float>()) / Statistics::correctionFactor;
	    }
	    else {
		middle = slidingArrayMath(input, box, MeanFunc<Float>());
		spread = slidingArrayMath(input, box, StddevFunc<Float>());
	    }
	}

	casa::Array<Float> calcSNR(casa::Array<Float> &input, casa::Array<Float> &middle, casa::Array<Float> &spread)
	{
	    ASKAPASSERT(input.shape()==middle.shape());
	    ASKAPASSERT(input.shape()==spread.shape());
	    casa::Array<Float> snr(input.shape(), 0.);
	    // Make sure we don't divide by the zeros around the edge of madfm. Need to set those values to S/N=0.
	    Array<Float>::iterator inputEnd(input.end());
	    Array<Float>::iterator iterInput(input.begin()),iterMiddle(middle.begin()),iterSpread(spread.begin()),iterSnr(snr.begin());
	    while(iterInput != inputEnd){
		*iterSnr = (*iterSpread > 0) ? (*iterInput - *iterMiddle)/(*iterSpread) : 0.;
		iterInput++;
		iterMiddle++;
		iterSpread++;
		iterSnr++;
	    }
	    return snr;
	}

	void findSNR_OLD(float *input, float *output, casa::IPosition shape, casa::IPosition box, size_t loc, size_t spatSize, size_t specSize, bool isSpatial, bool useRobust)
	{
	    casa::Array<Float> base(shape, input, casa::COPY);
	    casa::Array<Float> middle, spread;
	    if(useRobust){
		middle = slidingArrayMath(base, box, MedianFunc<Float>());
		spread = slidingArrayMath(base, box, MadfmFunc<Float>()) / Statistics::correctionFactor;
	    }
	    else{
		middle = slidingArrayMath(base, box, MeanFunc<Float>());
		spread = slidingArrayMath(base, box, StddevFunc<Float>());
	    }
	    casa::Array<Float> snr = (base - middle);
	
	    // Make sure we don't divide by the zeros around the edge of madfm. Need to set those values to S/N=0.
	    Array<Float>::iterator baseEnd(base.end());
	    Array<Float>::iterator iterBase(base.begin()),iterMiddle(middle.begin()),iterSpread(spread.begin()),iterSnr(snr.begin());
	    int i=0,pos=0;
	    while(iterBase != baseEnd){
		pos = isSpatial ?  i+loc*spatSize :  loc+i*spatSize;
		output[pos] = (*iterSpread > 0) ? (*iterSnr)/(*iterSpread) : 0.;
		i++;
		iterBase++;
		iterMiddle++;
		iterSpread++;
		iterSnr++;
	    }
	
	
	}


    }

}
