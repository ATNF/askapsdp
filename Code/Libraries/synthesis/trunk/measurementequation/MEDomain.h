/// @file
///
/// MEDomain: Represent a domain for imaging equation purposes.
/// 
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef MEDOMAIN_H_
#define MEDOMAIN_H_

#include <iostream>
#include <vector>
#include <map>
#include <string>

namespace conrad
{
	
namespace synthesis
{

class MEDomain
{
public:
	/// Make an empty domain
	MEDomain();
	
	/// Assignment operator
	MEDomain& operator=(const MEDomain& other);
	
	/// Copy constructor
	MEDomain(const MEDomain& other);

	~MEDomain();
	
	/// Add an axis 
	/// @param name Name of axis
	/// @param start Start value
	/// @param end End value
	/// @param cells Optional number of cells	
	void add(const std::string& name, const double start, const double end, const int cells=1);
	
	/// Has this axis?
	/// @param name Name of axis
	bool has(const std::string& name) const;
	
	/// Return the possible axis names
	std::vector<std::string> names() const;

	/// Return start value	
	/// @param name Name of axis
	double start(const std::string& name) const;
	
	/// Return end value	
	/// @param name Name of axis
	double end(const std::string& name) const;

	/// Return number of cells
	/// @param name Name of axis
	int cells(const std::string& name) const;
	
	friend std::ostream& operator<<(std::ostream& os, const MEDomain& domain);
	
private:
	mutable std::map<std::string, double> itsStart;
	mutable std::map<std::string, double> itsEnd;
	mutable std::map<std::string, int> itsCells;
};

};
};
#endif /*MEDOMAIN_H_*/