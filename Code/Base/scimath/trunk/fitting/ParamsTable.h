/// @file
///
/// ParamsTable: Base class for storing and retrieving Params
///
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
#ifndef SCIMATHPARAMSTABLE_H_
#define SCIMATHPARAMSTABLE_H_

#include <fitting/Params.h>
#include <fitting/Axes.h>

namespace conrad
{
  namespace scimath
  {
    /// Abstract base class for parameter persistence
    class ParamsTable
    {
      public:
      /// Default constructor
        ParamsTable();

        virtual ~ParamsTable();

/// Get all the parameters
/// @param ip Template of parameters - must match
        virtual void getParameters(Params& ip) const;

/// Get the parameters for a specified domain
/// @param ip Template of parameters - must match
/// @param dom domain of parameters
        virtual void getParameters(Params& ip, const Domain& dom) const;

/// Set all the parameters
/// @param ip Parameters to set
        virtual void setParameters (const Params& ip);

/// Set the parameters for a given domain
/// @param ip Parameters to set
/// @param dom domain of parameters
        virtual void setParameters (const Params& ip, const Domain& dom);

 
    };

  }
}
#endif                                            /*PARAMSTABLE_H_*/
