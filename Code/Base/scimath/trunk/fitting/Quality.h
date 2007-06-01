#ifndef SCIMATHQUALITY_H_
#define SCIMATHQUALITY_H_

#include <iostream>
#include <string>

namespace conrad
{
namespace scimath
{

class Quality
{
public:
	Quality();
	virtual ~Quality();
	void setRank(const unsigned int rank) {itsRank=rank;};
	const unsigned int rank() const {return itsRank;};
	void setCond(const double cond) {itsCond=cond;};
	const double cond() const {return itsCond;};
	void setInfo(const std::string info) {itsInfo=info;}
	const std::string& info() const {return itsInfo;};
	void setDOF(const unsigned int DOF) {itsDOF=DOF;};
	const unsigned int DOF() const {return itsDOF;};
	
	friend std::ostream& operator<<(std::ostream& os, const Quality& q);
	
private:
	double itsCond;
	unsigned int itsRank;
	unsigned int itsDOF;
	std::string itsInfo;
};

}
}
#endif /*QUALITY_H_*/
