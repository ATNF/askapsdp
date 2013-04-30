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


    }

}
