/// @file
///
/// ParamsTable: Base class for storing and retrieving Params
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
#ifndef SCIMATHPARAMSTABLE_H_
#define SCIMATHPARAMSTABLE_H_

#include <fitting/Params.h>
#include <fitting/Axes.h>

namespace conrad
{
  namespace scimath
  {

    class ParamsTable
    {
      public:
        ParamsTable();

        virtual ~ParamsTable();

/// Get the parameters for a specified domain
/// @param ip Template of parameters - must match
/// @param dom domain of parameters
        virtual void getParameters(Params& ip) const;
        virtual void getParameters(Params& ip, const Domain& domain) const;

/// Set the parameters for a given domain
/// @param ip Parameters to set
/// @param dom domain of parameters
        virtual void setParameters (const Params& ip);
        virtual void setParameters (const Params& ip, const Domain& domain);

    };

  }
}
#endif                                            /*PARAMSTABLE_H_*/
