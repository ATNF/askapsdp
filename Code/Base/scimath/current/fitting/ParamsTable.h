/// @file
///
/// ParamsTable: Base class for storing and retrieving Params
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
#ifndef SCIMATHPARAMSTABLE_H_
#define SCIMATHPARAMSTABLE_H_

#include <fitting/Params.h>
#include <fitting/Axes.h>

namespace askap
{
  namespace scimath
  {
    /// Abstract base class for parameter persistence
    /// @ingroup fitting
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
