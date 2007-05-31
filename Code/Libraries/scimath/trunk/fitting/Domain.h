/// @file
///
/// Domain: Represent a domain for imaging equation purposes.
/// 
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef SCIMATHDOMAIN_H_
#define SCIMATHDOMAIN_H_

#include <iostream>
#include <vector>
#include <map>
#include <string>

namespace conrad
{
	
namespace scimath
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
	void add(const std::string& name, const double start, const double end);
	
	/// Has this axis?
	/// @param name Name of axis
	bool has(const std::string& name) const;

	/// Order of this axis
	/// @param name Name of axis
	int order(const std::string& name) const;
		
	/// Return the possible axis names
	const std::vector<std::string>& names() const;

    /// Return start value  
    /// @param name Name of axis
    double start(const std::string& name) const;
    
    /// Return end value    
    /// @param name Name of axis
    double end(const std::string& name) const;
    
    /// Return start value  
    /// @param name Name of axis
    const std::vector<double>& start() const;
    
    /// Return end value    
    /// @param name Name of axis
    const std::vector<double>& end() const;
    
	friend std::ostream& operator<<(std::ostream& os, const Domain& domain);
	
private:
	mutable std::vector<std::string> itsNames;
	mutable std::vector<double> itsStart;
	mutable std::vector<double> itsEnd;
};

};
};
#endif /*DOMAIN_H_*/
