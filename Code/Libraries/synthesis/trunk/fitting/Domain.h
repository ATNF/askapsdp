/// @file
///
/// Domain: Represent a domain for imaging equation purposes.
/// 
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef SYNDOMAIN_H_
#define SYNDOMAIN_H_

#include <iostream>
#include <vector>
#include <map>
#include <string>

namespace conrad
{
	
namespace synthesis
{

class Domain
{
public:
	/// Make an empty domain
	Domain();
	
	/// Assignment operator
	Domain& operator=(const Domain& other);
	
	/// Copy constructor
	Domain(const Domain& other);

	~Domain();
	
	/// Add an axis 
	/// @param name Name of axis
	/// @param start Start value
	/// @param end End value
	/// @param cells Optional number of cells	
	void add(const std::string& name, const double start, const double end, const int cells=1);
	
	/// Has this axis?
	/// @param name Name of axis
	bool has(const std::string& name) const;

	/// Order of this axis
	/// @param name Name of axis
	int order(const std::string& name) const;
		
	/// Return the possible axis names
	const std::vector<std::string>& names() const;

	/// Return the shape
	const std::vector<int>& shape() const;

	/// Return start value	
	/// @param name Name of axis
	double start(const std::string& name) const;
	
	/// Return end value	
	/// @param name Name of axis
	double end(const std::string& name) const;

	/// Return number of cells
	/// @param name Name of axis
	int cells(const std::string& name) const;
	
	friend std::ostream& operator<<(std::ostream& os, const Domain& domain);
	
private:
	mutable std::vector<std::string> itsNames;
	mutable std::vector<double> itsStart;
	mutable std::vector<double> itsEnd;
	mutable std::vector<int> itsCells;
};

};
};
#endif /*DOMAIN_H_*/
