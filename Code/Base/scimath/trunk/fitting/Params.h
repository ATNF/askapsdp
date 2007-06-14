/// @file
///
/// Params: represent a set of parameters for an equation.
///
/// A parameter has:
///    - A name
///    - A scalar or array double precision value
///    - Some axes for the array
///    - Free or fixed status
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef SCIMATHPARAMS_H_
#define SCIMATHPARAMS_H_

#include <fitting/Axes.h>

#include <casa/aips.h>
#include <casa/Arrays/Array.h>

#include <map>
#include <vector>
#include <string>
#include <ostream>

namespace conrad
{
  namespace scimath
  {

    class Params
    {
      public:

        Params();

        Params& operator=(const Params& other);

        Params(const Params& other);

/// Add an Parameter
/// @param name Name of param to be added
        void add(const std::string& name, const double ip=0.0);

/// Add an Parameter
/// @param name Name of param to be added
/// @param ip Param to be added
        void add(const std::string& name, const casa::Array<double>& ip);

/// Add an Parameter
/// @param name Name of param to be added
/// @param ip Param to be added
        void add(const std::string& name, const casa::Array<double>& ip,
          const Axes& axes);
        void add(const std::string& name, const double ip,
          const Axes& axes);

/// Update an Parameter
/// @param name Name of param to be updated
/// @param ip Param to be updated
        void update(const std::string& name, const casa::Array<double>& ip);
        void update(const std::string& name, const double ip);

/// Is this parameter a scalar?
        bool isScalar(const std::string& name) const;

/// Fix a parameter
        void fix(const std::string& name);

/// Free a parameter
        void free(const std::string& name);

/// Is this parameter free?
        bool isFree(const std::string& name) const;

// Return number of values
        const uint size() const;

/// Return the value for the parameter with this name
/// @param name Name of param
        const casa::Array<double>& value(const std::string& name) const;
        casa::Array<double>& value(const std::string& name);

/// Return the value for the scalar parameter with this name
/// Throws invalid_argument if non-scalar
/// @param name Name of param
        const double scalarValue(const std::string& name) const;
        double scalarValue(const std::string& name);

/// Return the domain for the parameter with this name
/// @param name Name of param
        const Axes& axes(const std::string& name) const;
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

        bool has(const std::string& name) const;

/// Is this set congruent with another?
        bool isCongruent(const Params& other) const;

/// Merge parameters from other into this set
        void merge(const Params& other);

/// Reset to empty
        void reset();

/// Count -  gives the number of accesses - use
/// this as a aid in caching. Is incremented on every non-const
/// access. A cache is no longer valid if the count has increased.
        int count(const std::string& name) const;

        friend std::ostream& operator<<(std::ostream& os, const Params& params);

      private:
        mutable std::map<std::string, casa::Array<double> > itsArrays;
        mutable std::map<std::string, Axes> itsAxes;
        mutable std::map<std::string, bool> itsFree;
        mutable std::map<std::string, int> itsCounts;

    };

  }
}
#endif
