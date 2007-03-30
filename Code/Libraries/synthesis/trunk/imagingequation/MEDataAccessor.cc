#include "MEDataAccessor.h"

namespace conrad {

//MEDataAccessor::MEDataAccessor()
//{
//}

/// an empty virtual destructor is required to make the
/// compiler happy for all derived classes, which don't 
/// require any special destructor (we have virtual 
/// functions, while a default destructor is not virtual)
MEDataAccessor::~MEDataAccessor()
{
}

//void MEDataAccessor::initmodel() {
//}

}
