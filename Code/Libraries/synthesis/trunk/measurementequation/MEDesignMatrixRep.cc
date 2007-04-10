#include <measurementequation/MEDesignMatrixRep.tcc>
#include <measurementequation/MEDesignMatrixRep.h>
#include <measurementequation/MEimage.h>
#include <casa/aips.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Matrix.h>
// Shouldn't need this next line but we do....
#include <casa/Arrays/MaskedArray.h>

using namespace casa;

namespace conrad {
namespace synthesis
{
	template class MEDesignMatrixRep<double>;
	template class MEDesignMatrixRep<MEImage>;
}
}

#include <casa/Arrays/Array.cc>
#include <casa/Arrays/ArrayMath.cc>
namespace casa {
	template class Matrix<conrad::synthesis::MEImage>;
}