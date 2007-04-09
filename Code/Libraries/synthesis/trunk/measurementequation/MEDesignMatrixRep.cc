#include <measurementequation/MEDesignMatrixRep.tcc>
#include <measurementequation/MEDesignMatrixRep.h>
#include <measurementequation/MEimage.h>

namespace conrad {
namespace synthesis
{
	template class MEDesignMatrixRep<double>;
	template class MEDesignMatrixRep<MEImage>;
	
}
}