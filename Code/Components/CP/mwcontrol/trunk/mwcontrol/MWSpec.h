//# MWSpec.h: Base component class of the MWSpec composite pattern.
//#
//# Copyright (C) 2007
//#
//# $Id$

#ifndef CONRAD_MWCONTROL_MWSPEC_H
#define CONRAD_MWCONTROL_MWSPEC_H

/// \file
/// Base component class of the MWSpec composite pattern.

//# Includes
#include <mwcontrol/MWSpecVisitor.h>
#include <mwcommon/DomainShape.h>
#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

//# Forward Declarations.
namespace LOFAR { namespace ACC { namespace APS { class ParameterSet; }}}

namespace conrad { namespace cp {

  /// This is the so-called \e component class in the MWSpec composite
  /// pattern (see Gamma, 1995). It is the base class for all MWSpec
  /// classes, both composite and leaf classes. It has data members that are
  /// common to all MWSpec classes.

  class MWSpec
  {
  public:
    /// Define a shared pointer to this object.
    typedef boost::shared_ptr<MWSpec> ShPtr;

    /// Destructor.
    virtual ~MWSpec();

    /// Create a new spec object. The new spec can either be a MWSingleSpec
    /// or a MWMultiSpec object. This is determined by examining the
    /// parameter set \a parSet. If this set contains a key
    /// <tt>Step.<em>name</em>.Steps</tt>, then \a aName is a MWMultiSpec,
    /// otherwise it is a SingleSpec. The third, optional, argument is used
    /// to pass a backreference to the parent MWSpec object.
    static MWSpec::ShPtr create (const std::string& name,
				 const LOFAR::ACC::APS::ParameterSet& parSet,
				 const MWSpec* parent = 0);

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

    /// Make \a parent the parent of this spec.
    void setParent(const MWSpec* parent)
      { itsParent = parent; }

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
    /// Default constructor. Construct an empty MWSpec object and make it a
    /// child of the MWSpec object \a parent.
    MWSpec(const MWSpec* parent = 0) : itsParent(parent) {}

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
