/// @file
/// @brief Step to process the MW solve command using BBSKernel.
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

#ifndef ASKAP_MWCONTROL_MWSOLVESTEP_H
#define ASKAP_MWCONTROL_MWSOLVESTEP_H

#include <mwcommon/MWSolveStep.h>
#include <mwcontrol/MWStepBBSProp.h>
#include <mwcommon/DomainShape.h>
#include <vector>
#include <string>

namespace askap { namespace cp {

  /// @ingroup mwcommon
  /// @brief Step to process the MW solve command using BBSKernel.

  /// This class defines a step that solves parameters by comparing a
  /// parameterized model to data in the VDS.
  /// The following data are defined for a solve:
  /// <ul>
  ///  <li> The names of parameters to solve for. This is done by means of
  ///       a vector of file name like patterns, so all parameters matching
  ///       the pattern are used.
  ///       A parameter name consist of multiple parts separated by colons.
  ///  <li> The names of parameters to be excluded from above (also using
  ///       a vector of patterns).
  ///  <li> The shape of the solve domain. It cannot exceed a work domain
  ///       defined in WorkDomainSpec, but it can be smaller. If smaller,
  ///       independent solutions will be determined for each solve domain.
  ///  <li> Convergence criteria:
  ///       <ul>
  ///        <li> Maximum number of iterations.
  ///             Typically 10.
  ///        <li> Epsilon. A solve domain has converged if the
  ///             fractional improvement of a solution < epsilon.
  ///             Typically 1e-7.
  ///        <li> The fraction of solve domains to be converged before
  ///             the entire solve is treated as being converged.
  ///             Probably non-converged solve domains contain bad data.
  ///             Typically 0.95.
  ///       </ul>
  /// </ul>
  ///
  /// It uses the standard MWStep functionality (factory and visitor) to
  /// create and process the object.
  /// The object can be converted to/from blob, so it can be sent to workers.

  class MWSolveStepBBS: public MWSolveStep
  {
  public:
    MWSolveStepBBS();

    virtual ~MWSolveStepBBS();

    /// Clone the step object.
    virtual MWSolveStepBBS* clone() const;

    /// Create a new object of this type.
    static MWStep::ShPtr create();

    /// Register the create function in the MWStepFactory.
    static void registerCreate();

    /// Set/get the parameter name patterns.
    /// @{
    void setParmPatterns (const std::vector<std::string>& parms)
      { itsParmPatterns = parms; }
    const std::vector<std::string>& getParmPatterns() const
      { return itsParmPatterns; }
    /// @}
    
    /// Set/get the parameter name patterns to be excluded.
    /// @{
    void setExclPatterns (const std::vector<std::string>& parms)
      { itsExclPatterns = parms; }
    const std::vector<std::string>& getExclPatterns() const
      { return itsExclPatterns; }
    /// @}
    
    /// Set/get the solve domain shape.
    /// @{
    void setDomainShape (const DomainShape& ds)
      { itsShape = ds; }
    DomainShape getDomainShape() const
      { return itsShape; }
    /// @}
    
    /// Set/get the max nr of iterations.
    /// By default it is 10.
    /// @{
    void setMaxIter (int maxIter)
      { itsMaxIter = maxIter; }
    int getMaxIter() const
      { return itsMaxIter; }
    /// @}

    /// Set/get the convergence epsilon.
    /// A fitter has converged if
    ///    abs(sol - lastsol) / max(abs(lastsol), abs(sol)) < epsilon.
    /// By default it is 1e-5.
    /// @{
    void setEpsilon (double epsilon)
      { itsEpsilon = epsilon; }
    double getEpsilon() const
      { return itsEpsilon; }
    /// @}

    /// Set/get the fraction of fitters that have to converge.
    /// By default it is 0.95.
    /// @{
    void setFraction (double fraction)
      { itsFraction = fraction; }
    double getFraction() const
      { return itsFraction; }
    /// @}

    /// Give the (unique) class name of the MWStep.
    virtual std::string className() const;

    /// Visit the object, so the visitor can process it.
    virtual void visit (MWStepVisitor&) const;

    /// Get access to the properties.
    /// @{
    const MWStepBBSProp& getProp() const
      { return itsProp; }
    MWStepBBSProp& getProp()
      { return itsProp; }
    /// @}

    /// Convert to/from blob.
    /// @{
    virtual void toBlob (LOFAR::BlobOStream&) const;
    virtual void fromBlob (LOFAR::BlobIStream&);
    /// @}

  private:
    std::vector<std::string> itsParmPatterns;
    std::vector<std::string> itsExclPatterns;
    DomainShape   itsShape;
    int           itsMaxIter;
    double        itsEpsilon;
    double        itsFraction;    /// fraction of fitters to be converged
    MWStepBBSProp itsProp;
  };

}} /// end namespaces

#endif
