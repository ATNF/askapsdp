#include "MESolver.h"
#include <casa/aips.h>
#include <scimath/Fitting/LSQFit.h>
#include <iostream>

namespace conrad
{

MESolver::MESolver(const MEParams& ip) {
	itsParams=ip;
};


}
