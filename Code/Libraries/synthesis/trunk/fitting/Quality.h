#ifndef SYNQUALITY_H_
#define SYNQUALITY_H_

#include <iostream>
#include <string>

namespace conrad
{
namespace synthesis
{

class MEQuality
{
public:
	MEQuality();
	virtual ~MEQuality();
	void setRank(const unsigned int rank) {itsRank=rank;};
	const unsigned int rank() const {return itsRank;};
	void setCond(const double cond) {itsCond=cond;};
	const double cond() const {return itsCond;};
	void setInfo(const std::string info) {itsInfo=info;}
	const std::string& info() const {return itsInfo;};
	
	friend std::ostream& operator<<(std::ostream& os, const MEQuality& q);
	
private:
	double itsCond;
	unsigned int itsRank;
	std::string itsInfo;
};

}
}
#endif /*MEQUALITY_H_*/
