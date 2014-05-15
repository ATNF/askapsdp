 /// @file
///
/// Holds the parameters for a set of linear equations
///
/// @copyright (c) 2007 CSIRO
/// Australia Telescope National Facility (ATNF)
/// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
/// PO Box 76, Epping NSW 1710, Australia
/// atnf-enquiries@csiro.au
///
/// This file is part of the ASKAP software distribution.
///
/// The ASKAP software distribution is free software: you can redistribute it
/// and/or modify it under the terms of the GNU General Public License as
/// published by the Free Software Foundation; either version 2 of the License,
/// or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
///
/// @author Tim Cornwell <tim.cornwell@csiro.au>
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

#include <askap/AskapUtil.h>
#include <askap/AskapError.h>
#include <utils/DeepCopyUtils.h>

#include <iostream>
#include <map>
#include <string>

using std::map;
using std::string;
using std::ostream;

namespace askap
{
	namespace scimath
	{

		Params::Params()
		{
		}

		Params::Params(const Params& other) :  itsAxes(other.itsAxes),
		      itsFree(other.itsFree)
		{
            deepCopyOfSTDMap(other.itsArrays, itsArrays);
 
			// itsChangeMonitors is not copied deliberately
			ASKAPDEBUGASSERT(itsChangeMonitors.size() == 0);
		}

		Params& Params::operator=(const Params& other)
		{
			if(this!=&other)
			{
                deepCopyOfSTDMap(other.itsArrays, itsArrays);
				itsAxes=other.itsAxes;
				itsFree=other.itsFree;
				// change monitor map is reset deliberately
				itsChangeMonitors.clear();
			}
			return *this;
		}
		
		/// @brief make a slice of another params class
        /// @details This method extracts one or more parameters 
        /// from the given Params object and stores them in the 
        /// current object. The current content of this object is lost.
        /// @note This method assumes reference semantics for data values
        /// as the use case is to split parameters and act as an adapter
        /// supporting the general interface. In the case of image parameters
        /// the size is considerable to do an unnecessary copy. Use the clone() 
        /// method explicitly or a copy constructor if a proper copy is required.
        /// We do not currently expect to access the resulting class for writing.
        /// @param[in] other other Params class to take the data from
        /// @param[in] names2copy list of parameters to include into the slice
        void Params::makeSlice(const Params &other, const std::vector<std::string> &names2copy) {
            reset();
            for (std::vector<std::string>::const_iterator ci=names2copy.begin(); ci!=names2copy.end(); ++ci) {
                 itsArrays[*ci] = other.value(*ci);
                 itsAxes[*ci] = other.axes(*ci);
                 itsFree[*ci] = other.isFree(*ci);
            }
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
            ASKAPCHECK(has(name), "Parameter " + name + " does not exist");
			return itsFree.find(name)->second;
		}

		void Params::free(const std::string& name)
		{
            ASKAPCHECK(has(name), "Parameter " + name + " does not exist");
            itsFree[name]=true;
		}

		void Params::fix(const std::string& name)
		{
            ASKAPCHECK(has(name), "Parameter " + name + " does not exist");
            itsFree[name]=false;
		}

		void Params::add(const std::string& name, const double ip)
		{
			ASKAPCHECK(!has(name), "Parameter " + name + " already exists");
			casa::Array<double> ipArray(casa::IPosition(1,1));
			ipArray(casa::IPosition(1,0))=ip;
			itsArrays[name]=ipArray.copy();
			itsFree[name]=true;
			itsAxes[name]=Axes();
            notifyAboutChange(name);
		}

		void Params::add(const std::string& name, const casa::Array<double>& ip)
		{
			ASKAPCHECK(!has(name), "Parameter " + name + " already exists");
			itsArrays[name]=ip.copy();
			itsFree[name]=true;
			itsAxes[name]=Axes();
            notifyAboutChange(name);
		}

		void Params::add(const std::string& name, const casa::Array<double>& ip,
				const Axes& axes)
		{
			ASKAPCHECK(!has(name), "Parameter " + name + " already exists");
			itsArrays[name]=ip.copy();
			itsFree[name]=true;
			itsAxes[name]=axes;
            notifyAboutChange(name);
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
		
        /// @brief add a complex vector
        /// @details This method is a convenient way to update parameter which is a complex
        /// vector (translated to vector of real numbers twice the size)
        /// @param[in] name parameter name
        /// @param[in] value a value of the paramter to be added
        void Params::addComplexVector(const std::string &name, const casa::Vector<casa::Complex> &value)
        {
           casa::Array<double> buf(casa::IPosition(1,2*value.nelements()));
           casa::IPosition index(1,0);           
           for (casa::uInt elem = 0; elem < value.nelements(); ++elem,++index(0)) {
                const casa::Complex val = value[elem];
                buf(index) = real(val);
                ++index(0);
                buf(index) = imag(val);                
           }
           add(name,buf);
        }
                

		void Params::add(const std::string& name, const double ip, const Axes& axes)
		{
			ASKAPCHECK(!has(name), "Parameter " + name + " already exists");
			casa::Array<double> ipArray(casa::IPosition(1,1));
			ipArray(casa::IPosition(1,0))=ip;
			itsArrays[name]=ipArray.copy();
			itsFree[name]=true;
			itsAxes[name]=axes;
			notifyAboutChange(name);
		}

		void Params::update(const std::string& name, const casa::Array<double>& ip)
		{
			ASKAPCHECK(has(name), "Parameter " + name + " does not already exist");
			itsArrays[name]=ip.copy();
			itsFree[name]=true;
            notifyAboutChange(name);	
		}
		
        /// @brief Update a slice of an array parameter        
        /// @details This version of the method updates a part of the array given by the IPosition object,
        /// representing the bottom left corner (blc). The top right corner (trc) is obtained by adding 
        /// the shape of the given value (i.e. give blc = IPosition(4,0,0,1,0) to update only channel 0, 
        /// polarisation 1 plane)
        /// @param[in] name name of the parameter to be added
        /// const casa::Array<double>& value
        /// @param[in] value an array with replacement values
        /// @param[in] blc where to insert the new values to
        void Params::update(const std::string &name, const casa::Array<double> &value, 
                            const casa::IPosition &blc)
        {
           ASKAPCHECK(has(name), "Parameter " + name + " does not already exist");
           ASKAPDEBUGASSERT(value.shape().nelements() == blc.nelements());
           casa::Array<double> &arr = itsArrays[name];
           casa::IPosition trc(value.shape());
           trc += blc;
           for (casa::uInt i=0; i<trc.nelements(); ++i) {
                ASKAPDEBUGASSERT(trc[i]>0);
                trc[i]--;
                ASKAPDEBUGASSERT(trc[i]<arr.shape()[i]);
                ASKAPDEBUGASSERT(blc[i]>=0);                
                ASKAPDEBUGASSERT(blc[i]<=trc[i]);
           }
           arr(blc,trc) = value.copy();
           itsFree[name]=true;
           notifyAboutChange(name);
        }		
		
		/// @brief Add an empty array parameter        
        /// @details This version of the method creates a new array parameter with the
        /// given shape. It is largely intended to be used together with the partial slice
        /// access using the appropriate version of the update method. 
        /// @param[in] name name of the parameter to be added
        /// @param[in] shape required shape of the parameter
        /// @param[in] axes optional axes of the parameter
        void Params::add(const std::string &name, const casa::IPosition &shape, const Axes &axes)
        {
         	ASKAPCHECK(!has(name), "Parameter " + name + " already exists");
			itsArrays[name].resize(shape);
			itsFree[name]=true;
			itsAxes[name]=axes;
			notifyAboutChange(name);
		}
		
		
		/// @brief update a complex-valued parameter
        /// @details This method is a convenient way to update parameters, which
        /// are complex numbers. It is equivalent to updating of an array of size
        /// 2 filled with real and imaginary part.
        /// @param[in] name parameter name
        /// @param[in] value a value of the parameter to be added
        void Params::update(const std::string &name, const casa::Complex &value)
        {
            casa::Array<double> buf(casa::IPosition(1,2));
            buf(casa::IPosition(1,0)) = real(value);
            buf(casa::IPosition(1,1)) = imag(value);
            update(name,buf);
        }
		
		/// @brief update a complex vector
        /// @details This method is a convenient way to update parameter which is a complex
        /// vector (translated to vector of real numbers twice the size)
        /// @param[in] name parameter name
        /// @param[in] value a value of the paramter to be updated
        void Params::updateComplexVector(const std::string &name, const casa::Vector<casa::Complex> &value) 
        {
           casa::Array<double> buf(casa::IPosition(1,2*value.nelements()));
           casa::IPosition index(1,0);           
           for (casa::uInt elem = 0; elem < value.nelements(); ++elem,++index(0)) {
                const casa::Complex val = value[elem];
                buf(index) = real(val);
                ++index(0);
                buf(index) = imag(val);                
           }
           update(name,buf);
        }		

		void Params::update(const std::string& name, const double ip)
		{
			ASKAPCHECK(has(name), "Parameter " + name + " does not already exist");
			casa::Array<double> ipArray(casa::IPosition(1,1));
			ipArray(casa::IPosition(1,0))=ip;
			itsArrays[name]=ipArray.copy();
			itsFree[name]=true;
			notifyAboutChange(name);
		}

		uint Params::size() const
		{
			return static_cast<uint>(itsFree.size());
		}

		bool Params::has(const std::string& name) const
		{
			return itsArrays.find(name) != itsArrays.end();
		}

		bool Params::isScalar(const std::string& name) const
		{
			ASKAPCHECK(has(name), "Parameter " + name + " does not already exist");
			return value(name).nelements()==1;
		}

		const casa::Array<double>& Params::value(const std::string& name) const
		{
			ASKAPCHECK(has(name), "Parameter " + name + " does not already exist");
			return itsArrays.find(name)->second;
		}

		casa::Array<double>& Params::value(const std::string& name)
		{
			ASKAPCHECK(has(name), "Parameter " + name + " does not already exist");
			notifyAboutChange(name);
			return itsArrays.find(name)->second;
		}

		double Params::scalarValue(const std::string& name) const
		{
			ASKAPCHECK(has(name), "Parameter " + name + " does not already exist");
			ASKAPCHECK(isScalar(name), "Parameter " + name + " is not scalar");
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
		    ASKAPCHECK(has(name), "Parameter " + name + " does not already exist");
			const casa::Array<double> &arrVal = value(name);
			ASKAPCHECK(arrVal.nelements() != 0 && arrVal.nelements()<3 &&
			            arrVal.ndim() ==1, "Parameter " + name + 
			            " cannot be converted to a complex number");
			if (arrVal.nelements() == 1) {
			    return arrVal(casa::IPosition(1,0));
			} 
			return casa::Complex(arrVal(casa::IPosition(1,0)), 
			                     arrVal(casa::IPosition(1,1))); 
		}
		
		/// @brief obtain parameter as a complex vector
        /// @details Complex vectors are represented as real vectors with twice the size.
        /// @param[in] name parameter name
        /// @return complex vector
        casa::Vector<casa::Complex> Params::complexVectorValue(const std::string &name) const
        {
		    ASKAPCHECK(has(name), "Parameter " + name + " does not already exist");
			const casa::Array<double> &arrVal = value(name);
			ASKAPCHECK(arrVal.nelements() % 2 == 0, "Parameter "<<name<<
			           " has an odd number of elements, unable to convert to complex vector");
			casa::Vector<casa::Complex> result(arrVal.nelements() / 2);
			// just to have vector interface, casa array copy has reference semantics
			casa::Vector<double> vecVal(arrVal);
			for (casa::uInt elem = 0, outElem = 0; outElem < result.nelements(); ++outElem,elem+=2) {
			     const casa::Complex val(static_cast<float>(vecVal[elem]), static_cast<float>(vecVal[elem+1]));
			     result[outElem] = val;
			}
            return result;
        } 
		

		const Axes& Params::axes(const std::string& name) const
		{
			ASKAPCHECK(has(name), "Parameter " + name + " does not already exist");
			return itsAxes.find(name)->second;
		}

		Axes& Params::axes(const std::string& name) 
		{
			ASKAPCHECK(has(name), "Parameter " + name + " does not already exist");
			notifyAboutChange(name);
			return itsAxes.find(name)->second;
		}

        bool Params::isCongruent(const Params& other) const
        {
            for(std::map<string,bool>::const_iterator iter = itsFree.begin(); iter != itsFree.end(); iter++)
            {
                if (other.itsFree.find(iter->first) == other.itsFree.end()) {
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
					// we deliberately don't copy itsChangeMonitors map here as 
					// otherwise we would need some kind of global counter and a more
					// complicated logic. The working model is that change monitor should always
					// be first obtained from the same instance of the class.
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
                                   if (iter->second) {
				       casa::String complete(iter->first);
				       complete.gsub(sub, "");
				       completions.push_back(complete);
				       ncomplete++;
                                   }
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
          ASKAPDEBUGASSERT(has(name));
          itsArrays.erase(name);
          itsAxes.erase(name);
          itsFree.erase(name);
          // change monitor map doesn't need to contain all parameters
          std::map<std::string, ChangeMonitor>::iterator it = itsChangeMonitors.find(name);
          if (it != itsChangeMonitors.end()) {          
              itsChangeMonitors.erase(it);
          }
        }
		

		void Params::reset()
		{
			itsArrays.clear();
			itsAxes.clear();
			itsFree.clear();
			itsChangeMonitors.clear();
		}

		std::ostream& operator<<(std::ostream& os, const Params& params)
		{
		    // for very long lists of parameters it is inconvenient to show all
		    // elements. The following two parameters control how many parameters
		    // are shown. If the total number of parameters is less than lengthLimit,
		    // all parameters are shown. Otherwise, only first lengthLimit-showAtEnd and the
		    // last showAtEnd are shown
            const size_t lengthLimit = 20;
            const size_t showAtEnd = 5;
            ASKAPDEBUGASSERT(lengthLimit > showAtEnd);
            
			vector<string> names(params.names());
			
			size_t counter = 1;			
			for(vector<string>::const_iterator it = names.begin(); it != names.end(); ++it,++counter)
			{
				os << *it << " : ";
				if(params.isScalar(*it))
				{
					os << " (scalar) " << params.scalarValue(*it);
				}
				else
				{
					const casa::Array<double> &arrVal = params.value(*it);
					os << " (array : shape " << arrVal.shape()<<" max abs. value: "<<
                                           casa::max(casa::abs(arrVal));
					if (arrVal.nelements() == 2 && arrVal.ndim() == 1) {
					    os<<" or complex: "<<params.complexValue(*it);
					}
					os<< ") ";
				}
				if(params.isFree(*it))
				{
					os << " (free)" << std::endl;
				}
				else
				{
					os << " (fixed)" << std::endl;
				}
				if ((counter == lengthLimit) && (counter + showAtEnd  + 1 < names.size())) {
				    // need to advance iterator to skip some elements
				    const size_t elementsToSkip = names.size() - lengthLimit - showAtEnd;
				    ASKAPDEBUGASSERT(elementsToSkip > 0);
				    os<<" .... skipped "<<elementsToSkip<<" parameter"<<(elementsToSkip > 1 ? "s" : "")<<" ....."<<std::endl;
				    // another increment happens in the for statement, so advance the iterator by one less step
				    it += elementsToSkip - 1; 
				}
			}
			return os;
		}

/// @brief obtain change monitor for a parameter
/// @details A call to this method logically remembers 
/// the current value of a given parameter in order to keep
/// track whether it changes. This method is a companion
/// to isChanged method. The parameter with the given name should
/// already exist, otherwise an exception is thrown
/// @param[in] name name of the parameter
/// @return a change monitor object
ChangeMonitor Params::monitorChanges(const std::string& name) const
{
  ASKAPDEBUGASSERT(has(name));
  std::map<std::string, ChangeMonitor>::const_iterator cit = itsChangeMonitors.find(name);
  if (cit != itsChangeMonitors.end()) {
      return cit->second;
  } 
  // the following line will call the default constructor behind the scene.
  return itsChangeMonitors[name];
}

/// @brief notify change monitors about parameter update
/// @details Change monitors are used to track updates of some
/// parameters. This method first searches whether a particular
/// parameter is monitored. If yes, it notifies the appropriate
/// change monitor object (stored in itsChangeMonitors map).  
/// Nothing happens if the given parameter is not monitored.
/// @note Althoguh this method could have been made const because
/// it works with a mutable data member only, it is conceptually
/// non-const and is supposed to be used only inside methods changing
/// the parameter.
/// @param[in] name  name of the parameter
void Params::notifyAboutChange(const std::string &name)
{
  std::map<std::string, ChangeMonitor>::iterator it = itsChangeMonitors.find(name);
  if (it != itsChangeMonitors.end()) {
      // parameter is monitored
      it->second.notifyOfChanges();
  }   
}

    
/// @brief verify that the parameter has been changed
/// @details This method checks the status of a tracked
/// parameter. An exception is thrown if monitorChanges has
/// not been called for this particular Params object.
/// @param[in] name name of the parameter
/// @param[in] cm change monitor object (obtained with monitorChanges)
/// @return true, if the given parameter has been changed
bool Params::isChanged(const std::string &name, const ChangeMonitor &cm) const
{
  std::map<std::string, ChangeMonitor>::const_iterator cit = itsChangeMonitors.find(name);
  ASKAPCHECK(cit != itsChangeMonitors.end(), "Value change for parameter "<<name<<
             " is not tracked, run monitorChanges first");
  return cm != cit->second;
}

/// @brief increment this if there is any change to the stuff written into blob
#define BLOBVERSION 2

		// These are the items that we need to write to and read from a blob stream
		// note itsChangeMonitors is not written to blob deliberately
		// std::map<std::string, casa::Array<double> > itsArrays;
		// std::map<std::string, Axes> itsAxes;
		// std::map<std::string, bool> itsFree;

		LOFAR::BlobOStream& operator<<(LOFAR::BlobOStream& os, const Params& par)
		{
		    os.putStart("Params",BLOBVERSION);		
			os << par.itsArrays << par.itsAxes << par.itsFree;
			os.putEnd();			
            return os;
		}

		LOFAR::BlobIStream& operator>>(LOFAR::BlobIStream& is, Params& par)
		{
		    const int version = is.getStart("Params");
		    ASKAPCHECK(version == BLOBVERSION, 
		        "Attempting to read from a blob stream a Params object of the wrong version, expect "<<
		        BLOBVERSION<<" got "<<version);		
			is >> par.itsArrays >> par.itsAxes >> par.itsFree;
            is.getEnd();
            // as the object has been updated one needs to obtain new change monitor
            par.itsChangeMonitors.clear();			
            return is;
		}

	} // namespace scimath
	
    /// @brief populate scimath parameters from a LOFAR Parset object
    /// @details One often needs a possibility to populate 
    /// scimath::Params class from a Parset file (e.g. to load 
    /// initial gains from an external file). A number of add methods
    /// collected in this class happen to be image-specific. This is
    /// a generic method, which just copies all numeric fields
    /// @param[in] params a reference to scimath parameter object, where the
    /// parameters from parset file will be added
    /// @param[in] parset a const reference to a parset object
    /// @return a reference to params passed as an input (for chaining)
    /// @note This operator is declared outside of scimath namespace quite deliberately.
    /// Definining it in askap namespace will allow us to use it anywhere throughout our
    /// code without worrying about using statements. 
    scimath::Params& operator<<(scimath::Params &params, const LOFAR::ParameterSet &parset)
    {
       for (LOFAR::ParameterSet::const_iterator ci = parset.begin();
            ci != parset.end();++ci) {
            try {
               vector<double> vec = parset.getDoubleVector(ci->first);
               casa::Vector<double> arr(vec.size());
               std::copy(vec.begin(),vec.end(),arr.cbegin());
               params.add(ci->first, arr);
            }
            catch (const LOFAR::Exception &) {
               // ignore non-numeric parameters
            }
       }
       return params;
    }
	
} // namespace askap


