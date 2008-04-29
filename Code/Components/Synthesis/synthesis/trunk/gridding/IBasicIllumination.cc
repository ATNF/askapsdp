/// @file 
/// @brief Interface to a basic illumination pattern
/// @details This class is an abstract base (i.e. an interface) to 
/// an hierarchy of classes representing illumination patterns.
/// It provides a method to obtain illumination pattern by populating a 
/// pre-defined grid supplied as a UVPattern object. 
///
/// @copyright (c) 2008 ASKAP, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#include <gridding/IBasicIllumination.h>

using namespace askap;
using namespace askap::synthesis;

/// @brief empty virtual destructor to keep the compiler happy
IBasicIllumination::~IBasicIllumination() {}

