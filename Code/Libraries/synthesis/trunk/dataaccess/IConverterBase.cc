/// @file
///
/// IConverterBase: A base class for all converter classes. It doesn't
/// have any useful functionality and is used as a structural unit.
/// The only method defined is a virtual destructor to make the compiler
/// happy and reduce the number of *.cc files for the derived interfaces
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#include "IConverterBase.h"

using namespace conrad;
using namespace synthesis;

/// an empty virtual destructor to keep the compiler happy
/// for all derived interfaces
IConverterBase::~IConverterBase()
{
}
