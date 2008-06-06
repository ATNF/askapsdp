/// @file
/// @brief Specification of a solve step.
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
/// @author Ger van Diepen (diepen AT astron nl)
///
//# $Id$

#ifndef ASKAP_MWCONTROL_MWSOLVESPEC_H
#define ASKAP_MWCONTROL_MWSOLVESPEC_H

//# Includes
#include <mwcontrol/MWSingleSpec.h>


namespace askap { namespace cp {

  /// @ingroup mwcontrol
  /// @brief Specification of a solve step.

  /// This is a so-called \e leaf class in the MWSpec composite pattern (see
  /// Design Patterns, Gamma et al, 1995).
  ///
  /// It implements a solve step specification which is read from a
  /// LOFAR .parset file. The base class holds the standard data members,
  /// while this class holds the specific solve info.
  /// The class implements the required virtual functions.

  class MWSolveSpec : public MWSingleSpec
  {
  public:
    /// Construct from the given .parset file.
    /// Unspecified items are taken from the parent specification.
    MWSolveSpec(const std::string& name,
		const LOFAR::ACC::APS::ParameterSet& parset,
		const MWSpec* parent);

    virtual ~MWSolveSpec();

    /// Visit the object, so the visitor can process it.
    virtual void visit (MWSpecVisitor&) const;

    /// Print the contents in human readable form into the output stream.
    /// Indent as needed.
    virtual void print (std::ostream& os, const std::string& indent) const;

    /// @name Accessor methods
    /// @{
    unsigned getMaxIter() const
      { return itsMaxIter; }
    double getEpsilon() const
      { return itsEpsilon; }
    double getMinConverged() const
      { return itsMinConverged; }
    const std::vector<std::string>& getParms() const
      { return itsParms; }
    const std::vector<std::string>& getExclParms() const
      { return itsExclParms; }
    const DomainShape& getDomainShape() const
      { return itsDomainSize; }
    /// @}

  private:
    unsigned    itsMaxIter;          ///< Maximum number of iterations
    double      itsEpsilon;          ///< Convergence threshold
    double      itsMinConverged;     ///< Fraction that must have converged
    DomainShape itsDomainSize;       ///< Solve domain size.
    std::vector<std::string> itsParms;    ///< Names of the solvable parameters
    std::vector<std::string> itsExclParms;///< Parameters to be excluded
  };

}} /// end namespaces

#endif
