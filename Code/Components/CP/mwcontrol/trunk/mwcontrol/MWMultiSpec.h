/// @file
/// @brief Specification of a step containing multiple other steps.
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

#ifndef ASKAP_MWCONTROL_MWMULTISPEC_H
#define ASKAP_MWCONTROL_MWMULTISPEC_H

//# Includes
#include <mwcontrol/MWSpec.h>
#include <list>

namespace askap { namespace cp {

  /// @ingroup mwcontrol
  /// @brief Specification of a step containing multiple other steps.

  /// This is the so-called \e composite class in the composite pattern (see
  /// Design Patterns, Gamma et al, 1995). The composite class contains shared
  /// pointers to zero or more MWSpec (component) objects.
  /// This class is very useful to combine multiple steps which can be treated
  /// as a single step. They object is created from the contents of a LOFAR
  /// .parset file.
  ///
  /// The contained objects get their default values from the settings
  /// in this parent MWMultiSpec object.

  class MWMultiSpec : public MWSpec
  {
  public:
    /// Construct an empty MWMultiSpec object.
    MWMultiSpec();

    /// Construct a MWMultiSpec. \a name identifies the step name in the
    /// parameter set file. It does \e not uniquely identify the step \e
    /// object being created. The third argument is used to pass a
    /// backreference to the parent MWSpec object.
    MWMultiSpec(const std::string& name,
		const LOFAR::ACC::APS::ParameterSet& parset,
		const MWSpec* parent);

    virtual ~MWMultiSpec();

    /// Add a step.
    void push_back (const MWSpec::ShPtr& spec)
      { itsSpecs.push_back (spec); }

    /// Visit the object, so the visitor can process it.
    virtual void visit (MWSpecVisitor&) const;

    /// Print the contents in human readable form into the output stream.
    /// Indent as needed.
    virtual void print (std::ostream& os, const std::string& indent) const;

    /// Define functions to do STL style iteration.
    /// @{
    typedef std::list<MWSpec::ShPtr>::const_iterator const_iterator;
    const_iterator begin() const
      { return itsSpecs.begin(); }
    const_iterator end() const
      { return itsSpecs.end(); }
    /// @}

  private:
    /// Check to see if there's an infinite recursion present in the
    /// definition of a MWMultiSpec. This can happen when one of the specs
    /// (identified by the argument \a name) defining the MWMultiSpec refers
    /// directly or indirectly to that same MWMultiSpec. 
    void infiniteRecursionCheck (const std::string& name) const;

    /// Vector holding a sequence of MWSpecs.
    std::list<MWSpec::ShPtr> itsSpecs;
  };

}} /// end namespaces

#endif
