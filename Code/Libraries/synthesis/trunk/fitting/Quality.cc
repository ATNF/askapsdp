#include <fitting/Quality.h>

namespace conrad
{
namespace synthesis
{
	
Quality::Quality() : itsCond(0.0), itsRank(0), itsInfo("")
{
}

Quality::~Quality()
{
}

std::ostream& operator<<(std::ostream& os, const Quality& q)
{
	os << "Quality : " << q.info() << " : condition number = " 
		<< q.cond() << ", rank = " << q.rank() << std::endl;
}
	

}
}
