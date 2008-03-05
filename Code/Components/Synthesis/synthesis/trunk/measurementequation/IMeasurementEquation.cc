/// @file
/// 
/// @brief An abstract measurement equation.
/// @details To be able to use common code regardless on the time of the
/// measurement equaiton used (i.e. ComponentEquation, ImageFFTEquation, etc)
/// we need a common ancestor of the measurement equation classes.
/// askap::scimath::Equation is not specialised enough for this purpose.
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>


#include <measurementequation/IMeasurementEquation.h>

using namespace askap;
using namespace askap::synthesis;

/// @brief empty virtual descrtuctor to make the compiler happy
IMeasurementEquation::~IMeasurementEquation() {}

