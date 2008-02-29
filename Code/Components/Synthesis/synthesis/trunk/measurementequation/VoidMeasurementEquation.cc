/// @file
/// 
/// @brief A measurement equation, which does nothing.
/// @details The current calibration class requires a perfect measurement
/// equation. This class has been written to be able to use the same code 
/// for both applying a calibration and solving for parameters. It is 
/// a void measurement equation in the sense that it does nothing to the 
/// data or normal equations given to it.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>


#include <measurementequation/VoidMeasurementEquation.h>

using namespace conrad;
using namespace conrad::synthesis;

/// @brief Predict model visibilities for one accessor (chunk).
/// @details This prediction is done for single chunk of data only. 
/// It seems that all measurement equations should work with accessors 
/// rather than iterators (i.e. the iteration over chunks should be 
/// moved to the higher level, outside this class). 
void VoidMeasurementEquation::predict(IDataAccessor &) const {}

/// @brief Calculate the normal equation for one accessor (chunk).
/// @details This calculation is done for a single chunk of
/// data only (one iteration).It seems that all measurement
/// equations should work with accessors rather than iterators
/// (i.e. the iteration over chunks should be moved to the higher
/// level, outside this class). 
void VoidMeasurementEquation::calcEquations(const IConstDataAccessor &,
                          conrad::scimath::INormalEquations&) const {}
