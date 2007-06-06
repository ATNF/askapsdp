/// @file Quality.h
///
/// Quality: Encapsulate quality of a solution 
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
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
    /// Only constructor
	Quality();
    
	virtual ~Quality();
    
    // Set and get rank of equations
	void setRank(const unsigned int rank) {itsRank=rank;};
	const unsigned int rank() const {return itsRank;};

    // Set and get condition number of equations
	void setCond(const double cond) {itsCond=cond;};
	const double cond() const {return itsCond;};

    // Set and get caller-defined info string 
	void setInfo(const std::string info) {itsInfo=info;}
	const std::string& info() const {return itsInfo;};
    
    // Set and get degrees of freedom of equations
	void setDOF(const unsigned int DOF) {itsDOF=DOF;};
	const unsigned int DOF() const {return itsDOF;};
	
    // Output quality
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
