/// @file
/// @brief Base component class of the MWSpec composite pattern.
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

#ifndef ASKAP_MWCONTROL_MWSPEC_H
#define ASKAP_MWCONTROL_MWSPEC_H

//# Includes
#include <mwcontrol/MWSpecVisitor.h>
#include <mwcommon/DomainShape.h>
#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

//# Forward Declarations.
namespace LOFAR { namespace ACC { namespace APS { class ParameterSet; }}}

namespace askap { namespace cp {

  /// @ingroup mwcontrol
  /// @brief Base component class of the MWSpec composite pattern.

  /// This is the so-called \e component class in the MWSpec composite
  /// pattern (see Gamma, 1995). It is the base class for all MWSpec
  /// classes, both composite and leaf classes. It has data members that are
  /// common to all MWSpec classes.
  ///
  /// The MWSpec objects contain the specification of the BBS steps to
  /// perform in the MW framework. A step can be part of a composite
  /// MWMultiSpec object and the specification in there acts as the default
  /// value of a step. In that way it is possible to create a composite
  /// step object, that can be used with various sky source models.
  ///
  /// The specification is given in a LOFAR .parset file. In there each
  /// step has a name, say XX. Then the \a parset variables <tt>Step.XX.*</tt>
  /// contain the specification of XX.
  /// A composite object is made by specifying the names of the steps it
  /// consists of as <tt>Step.COMP.Steps=["XX", "YY", "ZZ"]</tt>.
  ///
  /// An MWSpec hierarchy needs to be transformed to an MWStep hierarchy
  /// to be able to process the steps. This is done by the derived MWSpecVisitor
  /// class MWSpec2Step.
 
  class MWSpec
  {
  public:
    /// Define a shared pointer to this object.
    typedef boost::shared_ptr<MWSpec> ShPtr;

    /// Destructor.
    virtual ~MWSpec();

    /// Create a new spec object. This is a factory method.
    /// The new spec can either be a MWSingleSpec or a MWMultiSpec object.
    /// This is determined by examining the parameter set \a parSet.
    /// If this set contains a key <tt>Step.<em>name</em>.Steps</tt>,
    /// then \a aName is a MWMultiSpec, otherwise it is an object derived
    /// from SingleSpec.
    /// The third argument is used to pass a backreference to the parent
    /// MWSpec object for the default values.
    /// The exact type of the MWSingleSpec is determined by the \a Operation.
    static MWSpec::ShPtr create (const std::string& name,
				 const LOFAR::ACC::APS::ParameterSet& parSet,
				 const MWSpec* parent);

    /// Print the contents of \c *this in human readable form into the output
    /// stream \a os.
    virtual void print (std::ostream& os, const std::string& indent) const = 0;

    /// @name Accessor methods
    /// @{

    /// Return the name of this spec.
    std::string getName() const
      { return itsName; }

    /// Return the full name of this spec. The full name consists of the name
    /// of this spec, preceeded by that of its parent, etc., separated by
    /// dots.
    std::string fullName() const;

    /// Return a pointer to the parent of this spec.
    const MWSpec* getParent() const
      { return itsParent; }

    /// Visit the object, so the visitor can process it.
    virtual void visit (MWSpecVisitor&) const = 0;

    /// Return the selection of baselines for this spec.
    /// @{
    const std::vector<std::string>& getStation1() const
      { return itsStation1; }
    const std::vector<std::string>& getStation2() const
      { return itsStation2; }
    /// @}

    /// Return which correlation products should be used for this spec.
    const std::vector<std::string>& getCorrType() const
      { return itsCorrType; }

    /// Return which antenna correlation should be used for this spec.
    const std::string& getCorrSelection() const
      { return itsCorrSelection; }

     /// Return the amount of integration that must be applied to the data.
    const DomainShape& getIntegration() const
      { return itsIntegration; }

    /// Return the sources in the source model for the current patch.
    const std::vector<std::string>& getSources() const
      { return itsSources; }

    /// Return the extra sources outside the current patch.
    const std::vector<std::string>& getExtraSources() const
      { return itsExtraSources; }

    /// Return a list of instrument model parts to be used for this spec.
    const std::vector<std::string>& getInstrumentModel() const
      { return itsInstrumentModel; }

    /// @}

  protected:
    /// Construct an empty object.
    MWSpec();

    /// Construct a MWSpec. \a name identifies the spec name in the
    /// parameter set file. It does \e not uniquely identify the spec \e
    /// object being created. The third argument is used to pass a
    /// backreference to the parent MWSpec object.
    MWSpec(const std::string& name, 
	   const LOFAR::ACC::APS::ParameterSet& parSet,
	   const MWSpec* parent);

    /// Print the info for a given object type.
    void printSpec (std::ostream& os, const std::string& indent,
		    const std::string& type) const;

  private:
    /// Override the default values, "inherited" from the parent spec object,
    /// for those members that are specified in \a parSet.
    void setParms(const LOFAR::ACC::APS::ParameterSet& parSet);

    /// Write the contents of a MWSpec to an output stream.
    friend std::ostream& operator<< (std::ostream&, const MWSpec&);


    /// Name of this spec.
    std::string              itsName;

    /// Pointer to the parent of \c *this. All MWSpec objects have a parent,
    /// except the top-level MWSpec object. The parent reference is used,
    /// among other things, to initialize the data members of the child
    /// object with those of its parent.
    const MWSpec*            itsParent;

    /// Selection of baselines for this spec.
    std::vector<std::string> itsStation1;
    std::vector<std::string> itsStation2;

    /// Parameters describing the amount of integration that must be applied
    /// to the data. Integration can be useful to decrease the amount of data.
    DomainShape              itsIntegration;

    //// Correlation types. E.g., ["XX", "XY", "YX", "YY"].
    std::vector<std::string> itsCorrType;

    //// Antenna correlation types.
    //// Valid values: "NONE", "AUTO", "CROSS", "ALL"
    std::string              itsCorrSelection;

    /// The sources in the source model for the current patch.
    std::vector<std::string> itsSources;

    /// Extra sources outside the current patch that may contribute to the
    /// current patch. They should be taken into account in order to improve
    /// the predictions of source parameters for the current patch.
    std::vector<std::string> itsExtraSources;

    /// A list of instrument model parts to be used for this spec.
    std::vector<std::string> itsInstrumentModel;
  };
  
}} /// end namespaces

#endif
