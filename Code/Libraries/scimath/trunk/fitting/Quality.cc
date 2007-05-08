#include <fitting/Quality.h>

namespace conrad
{
namespace scimath
{
	
Quality::Quality() : itsCond(0.0), itsRank(0), itsDOF(0), itsInfo("")
{
}

Quality::~Quality()
{
}

std::ostream& operator<<(std::ostream& os, const Quality& q)
{
	os << "Quality : " << q.info()
		<< " : degrees of freedom " << q.DOF() 
		<< ", rank = " << q.rank() 
		<< ", condition number = " << q.cond();
}
	

}
}
