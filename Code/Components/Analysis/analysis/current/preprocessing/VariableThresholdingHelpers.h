#ifndef ASKAP_ANALYSIS_VAR_THRESH_HELP_H_
#define ASKAP_ANALYSIS_VAR_THRESH_HELP_H_

#include <casa/Arrays/Array.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/ArrayPartMath.h>

#include <mathsutils/NewArrayPartMath.h>

namespace askap {

    namespace analysis {

	void slidingBoxStats(casa::Array<Float> &input, casa::Array<Float> &middle, casa::Array<Float> &spread, casa::IPosition &box, bool useRobust);
	casa::Array<Float> calcSNR(casa::Array<Float> &input, casa::Array<Float> &middle, casa::Array<Float> &spread);

	void findVariableThreshold_OLD(float *input, float *output, float SNRthreshold, casa::IPosition shape, casa::IPosition box, size_t loc, size_t spatSize, size_t specSize, bool isSpatial, bool useRobust);
	void findNoiseMap_OLD(float *input, float *output, casa::IPosition shape, casa::IPosition box, size_t loc, size_t spatSize, size_t specSize, bool isSpatial, bool useRobust);
	void findSNR_OLD(float *input, float *output, casa::IPosition shape, casa::IPosition box, size_t loc, size_t spatSize, size_t specSize, bool isSpatial, bool useRobust);

    }

}

#endif
