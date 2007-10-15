 /// @file
///
/// Holds the parameters for a set of linear equations
///
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell tim.cornwel@csiro.au
///

 #include <fitting/Params.h>
#include <fitting/Axes.h>
#include <casa/aips.h>
#include <casa/Utilities/Regex.h>
#include <casa/BasicSL/String.h>

#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobArray.h>
#include <Blob/BlobSTL.h>

#include <conrad/ConradUtil.h>
#include <conrad/ConradError.h>

#include <iostream>
#include <map>
#include <string>

using std::map;
using std::string;
using std::ostream;

namespace conrad
{
	namespace scimath
	{

		Params::Params()
		{
		}

		Params::Params(const Params& other)
		{
			operator=(other);
		}

		Params& Params::operator=(const Params& other)
		{
			if(this!=&other)
			{
				itsArrays=other.itsArrays;
				itsAxes=other.itsAxes;
				itsFree=other.itsFree;
				itsCounts=other.itsCounts;
			}
			return *this;
		}

		Params::ShPtr Params::clone() const
		{
			return Params::ShPtr(new Params(*this));
		}

		Params::~Params()
		{
		}

		bool Params::isFree(const std::string& name) const
		{
			return itsFree.find(name)->second;
		}

		void Params::free(const std::string& name)
		{
			itsFree[name]=true;
		}

		void Params::fix(const std::string& name)
		{
			itsFree[name]=false;
		}

		void Params::add(const std::string& name, const double ip)
		{
			CONRADCHECK(!has(name), "Parameter " + name + " already exists");
			casa::Array<double> ipArray(casa::IPosition(1,1));
			ipArray(casa::IPosition(1,0))=ip;
			itsArrays[name]=ipArray.copy();
			itsFree[name]=true;
			itsAxes[name]=Axes();
			itsCounts[name]=0;
		}

		void Params::add(const std::string& name, const casa::Array<double>& ip)
		{
			CONRADCHECK(!has(name), "Parameter " + name + " already exists");
			itsArrays[name]=ip.copy();
			itsFree[name]=true;
			itsAxes[name]=Axes();
			itsCounts[name]=0;
		}

		void Params::add(const std::string& name, const casa::Array<double>& ip,
				const Axes& axes)
		{
			CONRADCHECK(!has(name), "Parameter " + name + " already exists");
			itsArrays[name]=ip.copy();
			itsFree[name]=true;
			itsAxes[name]=axes;
			itsCounts[name]=0;
		}
		
		/// @brief add a complex-valued parameter
        /// @details This method is a convenient way to add parameters, which
        /// are complex numbers. It is equivalent to adding of an array of size
        /// 2 filled with real and imaginary part.
        /// @param[in] name parameter name
        /// @param[in] value a value of the parameter to be added
        void Params::add(const std::string &name, const casa::Complex &value)
        {
            casa::Array<double> buf(casa::IPosition(1,2));
            buf(casa::IPosition(1,0)) = real(value);
            buf(casa::IPosition(1,1)) = imag(value);
            add(name,buf);
        }
		

		void Params::add(const std::string& name, const double ip, const Axes& axes)
		{
			CONRADCHECK(!has(name), "Parameter " + name + " already exists");
			casa::Array<double> ipArray(casa::IPosition(1,1));
			ipArray(casa::IPosition(1,0))=ip;
			itsArrays[name]=ipArray.copy();
			itsFree[name]=true;
			itsAxes[name]=axes;
			itsCounts[name]=0;
		}

		void Params::update(const std::string& name, const casa::Array<double>& ip)
		{
			CONRADCHECK(has(name), "Parameter " + name + " does not already exist");
			itsArrays[name]=ip.copy();
			itsFree[name]=true;
			itsAxes[name]=Axes();
			itsCounts[name]++;
		}

		void Params::update(const std::string& name, const double ip)
		{
			CONRADCHECK(has(name), "Parameter " + name + " does not already exist");
			casa::Array<double> ipArray(casa::IPosition(1,1));
			ipArray(casa::IPosition(1,0))=ip;
			itsArrays[name]=ipArray.copy();
			itsFree[name]=true;
			itsAxes[name]=Axes();
			itsCounts[name]++;
		}

		const uint Params::size() const
		{
			return static_cast<uint>(itsFree.size());
		}

		bool Params::has(const std::string& name) const
		{
			return itsArrays.count(name)>0;
		}

		bool Params::isScalar(const std::string& name) const
		{
			CONRADCHECK(has(name), "Parameter " + name + " does not already exist");
			return value(name).nelements()==1;
		}

		const casa::Array<double>& Params::value(const std::string& name) const
		{
			CONRADCHECK(has(name), "Parameter " + name + " does not already exist");
			return itsArrays.find(name)->second;
		}

		casa::Array<double>& Params::value(const std::string& name)
		{
			CONRADCHECK(has(name), "Parameter " + name + " does not already exist");
			itsCounts[name]++;
			return itsArrays.find(name)->second;
		}

		double Params::scalarValue(const std::string& name) const
		{
			CONRADCHECK(has(name), "Parameter " + name + " does not already exist");
			CONRADCHECK(isScalar(name), "Parameter " + name + " is not scalar");
			return value(name)(casa::IPosition(1,0));
		}
		
		/// @brief Return the value for the complex-valued parameter (const)
        /// @details Any scalar parameter or generic array-valued parameter with the
        /// shape [2] can be retreived into a complex number. Two numbers
        /// are interpreted as real and imaginary part of the complex value. If only
        /// one number is available, it is assumed to be a real part, with the imaginary
        /// part being zero.
        /// @note This method throws invalud_argument if the parameter shape is 
        /// incompatible.
        /// @param[in] name Name of the parameter
        /// @return value of the parameter
        casa::Complex Params::complexValue(const std::string &name) const
		{
		    CONRADCHECK(has(name), "Parameter " + name + " does not already exist");
			const casa::Array<double> &arrVal = value(name);
			CONRADCHECK(arrVal.nelements() != 0 && arrVal.nelements()<3 &&
			            arrVal.ndim() ==1, "Parameter " + name + 
			            " cannot be converted to a complex number");
			if (arrVal.nelements() == 1) {
			    return arrVal(casa::IPosition(1,0));
			} 
			return casa::Complex(arrVal(casa::IPosition(1,0)), 
			                     arrVal(casa::IPosition(1,1))); 
		}

		const Axes& Params::axes(const std::string& name) const
		{
			CONRADCHECK(has(name), "Parameter " + name + " does not already exist");
			return itsAxes.find(name)->second;
		}

		bool Params::isCongruent(const Params& other) const
		{
			for(std::map<string,bool>::const_iterator iter = itsFree.begin(); iter != itsFree.end(); iter++)
			{
				if(other.itsFree.count(iter->first)==0)
				{
					return false;
				}
			}
			return true;
		}

		void Params::merge(const Params& other)
		{
			std::vector<string> names(other.names());
			for(std::vector<string>::const_iterator iter = names.begin(); iter != names.end(); iter++)
			{
				/// @todo Improve merging logic for Params
				if(!has(*iter))
				{
					itsArrays[*iter]=other.itsArrays.find(*iter)->second;
					itsFree[*iter]=other.itsFree.find(*iter)->second;
					itsAxes[*iter]=other.itsAxes.find(*iter)->second;
					itsCounts[*iter]++;
				}
			}
		}

		vector<string> Params::names() const
		{
			vector<string> names;
			for(std::map<string,bool>::const_iterator iter = itsFree.begin();
					iter != itsFree.end(); iter++)
			{
				names.push_back(iter->first);
			}
			return names;
		}

		vector<string> Params::freeNames() const
		{
			vector<string> names;
			for(std::map<string,bool>::const_iterator iter = itsFree.begin(); iter != itsFree.end(); iter++)
			{
				if(isFree(iter->first)) names.push_back(iter->first);
			}
			return names;
		}

		vector<string> Params::fixedNames() const
		{
			vector<string> names;
			for(std::map<string,bool>::const_iterator iter = itsFree.begin(); iter != itsFree.end(); iter++)
			{
				if(!isFree(iter->first)) names.push_back(iter->first);
			}
			return names;
		}

		vector<string> Params::completions(const std::string& pattern) const
		{
			casa::Regex regex(casa::Regex::fromPattern(pattern+"*"));
			casa::Regex sub(casa::Regex::fromPattern(pattern));
			vector<string> completions;
			uint ncomplete=0;
			for(std::map<string,bool>::const_iterator iter = itsFree.begin(); iter != itsFree.end(); iter++)
			{
				if(casa::String(iter->first).matches(regex))
				{
					casa::String complete(iter->first);
					complete.gsub(sub, "");
					completions.push_back(complete);
					ncomplete++;
				}
			}
			return completions;
		}
		
		/// @brief remove a parameter
        /// @details One needs to be able to remove a given parameter to avoid passing
        /// unused parameters to design matrix.
        /// @param[in] name parameter name
        void Params::remove(const std::string &name)
        {
          CONRADDEBUGASSERT(has(name));
          itsArrays.erase(name);
          itsAxes.erase(name);
          itsFree.erase(name);
          itsCounts.erase(name);
        }
		

		void Params::reset()
		{
			itsArrays.clear();
			itsAxes.clear();
			itsFree.clear();
			itsCounts.clear();
		}

		std::ostream& operator<<(std::ostream& os, const Params& params)
		{

			vector<string> names(params.names());
			for(vector<string>::const_iterator it = names.begin(); it != names.end(); it++)
			{
				os << *it << " : ";
				if(params.isScalar(*it))
				{
					os << " (scalar) " << params.scalarValue(*it);
				}
				else
				{
					os << " (array : shape " << params.value(*it).shape() << ") ";
				}
				if(params.isFree(*it))
				{
					os << " (free)" << std::endl;
				}
				else
				{
					os << " (fixed)" << std::endl;
				}
			}
			return os;
		}

		int Params::count(const std::string& name) const
		{
			CONRADCHECK(has(name), "Parameter " + name + " does not already exist");
			return itsCounts[name];
		}

		// These are the items that we need to write to and read from a blob stream
		// std::map<std::string, casa::Array<double> > itsArrays;
		// std::map<std::string, Axes> itsAxes;
		// std::map<std::string, bool> itsFree;
		// std::map<std::string, int> itsCounts;

		LOFAR::BlobOStream& operator<<(LOFAR::BlobOStream& os, const Params& par)
		{
			os << par.itsArrays << par.itsAxes << par.itsFree << par.itsCounts;
		}

		LOFAR::BlobIStream& operator>>(LOFAR::BlobIStream& os, Params& par)
		{
			os >> par.itsArrays >> par.itsAxes >> par.itsFree >> par.itsCounts;
		}

	}
}
