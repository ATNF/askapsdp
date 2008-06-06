/// @file
/// @brief Base class for a leaf class in the MWSpec composite pattern.
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

#ifndef ASKAP_MWCONTROL_MWSINGLESPEC_H
#define ASKAP_MWCONTROL_MWSINGLESPEC_H

//# Includes
#include <mwcontrol/MWSpec.h>

namespace askap { namespace cp {

  /// @ingroup mwcontrol
  /// @brief Base class for a leaf class in the MWSpec composite pattern.

  /// This is the base class for a so-called \e leaf class in the MWSpec composite
  /// pattern (see Design Patterns, Gamma et al, 1995).
  ///
  /// It contains some information common to the derived classes. Most common
  /// common information is held in the base MWSpec class. This class only contains
  /// the name of the output column in the VDS. Its name is read from the given
  /// LOFAR .parset file.

  class MWSingleSpec : public MWSpec
  {
  public:
    virtual ~MWSingleSpec();

    /// Print the contents and type in human readable form into the output stream.
    /// Indent as needed.
    void printSpec (std::ostream& os, const std::string& indent,
		    const std::string& type) const;

    /// Return the name of the data column to write data to.
    const std::string& getOutputData() const
      { return itsOutputData; }

  protected:
    /// Construct from the given .parset file.
    /// Unspecified items are taken from the parent specification.
    MWSingleSpec(const std::string& name,
		 const LOFAR::ACC::APS::ParameterSet& parset,
		 const MWSpec* parent);

  private:
    /// Name of the data column to write data to.
    std::string itsOutputData;
  };

}} /// end namespaces

#endif
