#include <measurementequation/MEQuality.h>

namespace conrad
{
namespace synthesis
{
	
MEQuality::MEQuality() : itsCond(0.0), itsRank(0), itsInfo("")
{
}

MEQuality::~MEQuality()
{
}

std::ostream& operator<<(std::ostream& os, const MEQuality& q)
{
	os << "Solution quality:" << q.info() << " condition number = " 
		<< q.cond() << ", rank = " << q.rank() << std::endl;
}
	

}
}