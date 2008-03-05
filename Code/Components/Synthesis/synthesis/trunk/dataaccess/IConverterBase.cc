/// @file IConverterBase.cc
/// @brief A base class for all converter classes.
/// @details
/// IConverterBase: A base class for all converter classes. It doesn't
/// have any useful functionality and is used as a structural unit.
/// The only method defined is a virtual destructor to make the compiler
/// happy and reduce the number of *.cc files for the derived interfaces
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#include <dataaccess/IConverterBase.h>

using namespace askap;
using namespace askap::synthesis;

/// an empty virtual destructor to keep the compiler happy
/// for all derived interfaces
IConverterBase::~IConverterBase()
{
}
