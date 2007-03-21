#include "IEqParams.h"

namespace conrad
{

IEqParams::IEqParams()
{
}

IEqParams::IEqParams(casa::String& parmtable)
{
}

IEqParams::~IEqParams()
{
	itsParams.resize(0);
}

}
