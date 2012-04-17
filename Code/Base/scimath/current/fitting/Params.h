/// @file fitting/Params.h
///
/// Params: represent a set of parameters for an equation.
///
/// A parameter has:
///    - A name
///    - A scalar or array double precision value
///    - Some axes for the array
///    - Free or fixed status
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
#ifndef SCIMATHPARAMS_H_
#define SCIMATHPARAMS_H_

#include <fitting/Axes.h>

#include <casa/aips.h>
#include <casa/Arrays/Array.h>

#include <boost/shared_ptr.hpp>

#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>

#include <utils/ChangeMonitor.h>

#include <map>
#include <vector>
#include <string>
#include <ostream>

#include <Common/ParameterSet.h>

namespace askap
{
  namespace scimath
  {
    /// @brief Represent parameters for an Equation
    /// @ingroup fitting
    class Params
    {
      public:

         /// Default constructor
        Params();

        /// Assignment operator
        Params& operator=(const Params& other);
        
        /// Copy constructor
        Params(const Params& other);
        
        /// Destructor
        virtual ~Params();

/// Add a scalar parameter
/// @param name Name of param to be added
/// @param value Value
        void add(const std::string& name, const double value=0.0);

/// Add an array parameter
/// @param name Name of param to be added
/// @param value Value
        void add(const std::string& name, const casa::Array<double>& value);

/// @brief Add an empty array parameter        
/// @details This version of the method creates a new array parameter with the
/// given shape. It is largely intended to be used together with the partial slice
/// access using the appropriate version of the update method. 
/// @param[in] name name of the parameter to be added
/// @param[in] shape required shape of the parameter
/// @param[in] axes optional axes of the parameter
void add(const std::string &name, const casa::IPosition &shape, const Axes &axes = Axes());

/// Add an array parameter with specified axes
/// @param name Name of param to be added
/// @param value Param to be added
/// @param axes Axes definition
        void add(const std::string& name, const casa::Array<double>& value,
          const Axes& axes);

/// Add a scalar parameter with specified axes
/// @param name Name of param to be added
/// @param value Value
/// @param axes Axes definition
        void add(const std::string& name, const double value,
          const Axes& axes);

        /// @brief add a complex-valued parameter
        /// @details This method is a convenient way to add parameters, which
        /// are complex numbers. It is equivalent to adding of an array of size
        /// 2 filled with real and imaginary part.
        /// @param[in] name parameter name
        /// @param[in] value a value of the parameter to be added
        void add(const std::string &name, const casa::Complex &value);

        /// @brief remove a parameter
        /// @details One needs to be able to remove a given parameter to avoid passing
        /// unused parameters to design matrix.
        /// @param[in] name parameter name
        void remove(const std::string &name);

/// Update an Array parameter
/// @param name Name of param to be updated
/// @param value New value
        void update(const std::string& name, const casa::Array<double>& value);

/// @brief Update a slice of an array parameter        
/// @details This version of the method updates a part of the array given by the IPosition object,
/// representing the bottom left corner (blc). The top right corner (trc) is obtained by adding 
/// the shape of the given value (i.e. give blc = IPosition(4,0,0,1,0) to update only channel 0, 
/// polarisation 1 plane)
/// @param[in] name name of the parameter to be added
/// const casa::Array<double>& value
/// @param[in] value an array with replacement values
/// @param[in] blc where to insert the new values to
void update(const std::string &name, const casa::Array<double> &value, const casa::IPosition &blc);

/// Update a scalar parameter
/// @param name Name of param to be updated
/// @param value New value
        void update(const std::string& name, const double value);

        /// @brief update a complex-valued parameter
        /// @details This method is a convenient way to update parameters, which
        /// are complex numbers. It is equivalent to updating of an array of size
        /// 2 filled with real and imaginary part.
        /// @param[in] name parameter name
        /// @param[in] value a value of the parameter to be added
        void update(const std::string &name, const casa::Complex &value);


/// Is this parameter a scalar?
/// @param name Name of param
        bool isScalar(const std::string& name) const;

/// Fix a parameter
/// @param name Name of param
        void fix(const std::string& name);

/// Free a parameter
/// @param name Name of param
        void free(const std::string& name);

/// Is this parameter free?
/// @param name Name of param
/// @return True if free to vary
        bool isFree(const std::string& name) const;

/// Return number of values in the parameter
        uint size() const;

/// Return array value for the parameter with this name (const)
/// @param name Name of param
        const casa::Array<double>& value(const std::string& name) const;

/// Return array value for the parameter with this name (non-const)
/// @param name Name of param
        casa::Array<double>& value(const std::string& name);

/// Return the value for the scalar parameter with this name (const)
/// Throws invalid_argument if non-scalar
/// @param name Name of param
/// @return Value of parameter
        double scalarValue(const std::string& name) const;

        /// @brief Return the value for the complex-valued parameter (const)
        /// @details Any scalar parameter or generic array-valued parameter with
        /// the shape [2] can be retreived into a complex number. Two numbers
        /// are interpreted as real and imaginary part of the complex value. If
        /// only one number is available, it is assumed to be a real part, with 
        /// the imaginary part being zero.
        /// @note This method throws invalud_argument if the parameter shape is 
        /// incompatible.
        /// @param[in] name Name of the parameter
        /// @return value of the parameter
        casa::Complex complexValue(const std::string &name) const;

/// Return the axes for the parameter with this name (const)
/// @param name Name of param
        const Axes& axes(const std::string& name) const;

        /// Return the axes for the parameter with this name (non-const)
        /// @param name Name of param
        Axes& axes(const std::string& name);

/// Return all the completions for this name
/// @param match Match e.g. "flux.i.*"
        std::vector<std::string> completions(const std::string& match) const;

/// Return the key names
        std::vector<std::string> names() const;

/// Return the key names of free items
        std::vector<std::string> freeNames() const;

/// Return the key names of fixed items
        std::vector<std::string> fixedNames() const;

/// Does this name exist?
/// @param name Name of parameters
/// @return True if named parameter exists
        bool has(const std::string& name) const;

/// Is this set congruent with another? Means that this must be a subset
/// @param other Other params to be test against
/// @return True if congruent
        bool isCongruent(const Params& other) const;

/// Merge parameters from other into this set
        void merge(const Params& other);

/// Reset to empty
        void reset();

    /// @brief obtain change monitor for a parameter
    /// @details A call to this method logically remembers 
    /// the current value of a given parameter in order to keep
    /// track whether it changes. This method is a companion
    /// to isChanged method. The parameter with the given name should
    /// already exist, otherwise an exception is thrown
    /// @param[in] name name of the parameter
    /// @return a change monitor object
    ChangeMonitor monitorChanges(const std::string& name) const;
    
    /// @brief verify that the parameter has been changed
    /// @details This method checks the status of a tracked
    /// parameter. An exception is thrown if monitorChanges has
    /// not been called for this particular Params object.
    /// @param[in] name name of the parameter
    /// @param[in] cm change monitor object (obtained with monitorChanges)
    /// @return true, if the given parameter has been changed
    bool isChanged(const std::string &name, const ChangeMonitor &cm) const;

/// Shared pointer definition
        typedef boost::shared_ptr<Params> ShPtr;

/// Clone this into a shared pointer
        Params::ShPtr clone() const;

        /// Shift operator for Params
        /// @param os Output ostream
        /// @param params Parameters to be output
        friend std::ostream& operator<<(std::ostream& os, const Params& params);

        /// Output shift operator for Params
        /// @param os Output ostream
        /// @param par Parameters to be processed
        friend LOFAR::BlobOStream& operator<<(LOFAR::BlobOStream& os, 
                                              const Params& par);
        /// Input shift operator for Params
        /// @param[in] is Input stream
        /// @param[in] par Parameters to be processed
        friend LOFAR::BlobIStream& operator>>(LOFAR::BlobIStream& is, 
                                              Params& par); 
         
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
        void makeSlice(const Params &other, const std::vector<std::string> &names2copy);
             
     protected:
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
        void notifyAboutChange(const std::string &name); 
        
     private:
        /// @todo Use single map map<string, struct>
        /// The value arrays, ordered as a map
        std::map<std::string, casa::Array<double> > itsArrays;
        /// The axes, ordered as a map
        std::map<std::string, Axes> itsAxes;
        /// The free/fixed status, ordered as a map
        std::map<std::string, bool> itsFree;
        /// The update count, ordered as a map. This is logically a cache 

        /// @brief change monitors for all tracked parameters
        /// @details This map contains change monitors for
        /// parameters which are tracked (i.e. for which the value
        /// change is monitored by some other code). It may not have
        /// all parameters. It is intentional, that this map is not
        /// copied when the object is cloned or restored from a Blob.
        mutable std::map<std::string, ChangeMonitor> itsChangeMonitors;
    };

  } // namespace scimath
  
  /// @brief populate scimath parameters from a LOFAR Parset object
  /// @details One needs often needs a possibility to populate 
  /// scimath::Params class from a Parset file (e.g. to load 
  /// initial gains from an external file).  This is
  /// a generic method, which just copies all numeric fields
  /// @param[in] params a reference to scimath parameter object, where the
  /// parameters from parset file will be added
  /// @param[in] parset a const reference to a parset object
  /// @return a reference to params passed as an input (for chaining)
  /// @note This operator is declared outside of scimath namespace quite deliberately.
  /// Definining it in askap namespace will allow us to use it anywhere throughout our
  /// code without worrying about using statements.
  scimath::Params& operator<<(scimath::Params &params, const LOFAR::ParameterSet &parset);
  
} // namespace askap
#endif
