/// @file MasterControl.h
/// @brief Master control of a distributed process.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Ger van Diepen (diepen AT astron nl)
///
//# $Id$

#ifndef CONRAD_MWCOMMON_MASTERCONTROL_H
#define CONRAD_MWCOMMON_MASTERCONTROL_H

#include <mwcommon/MWStepVisitor.h>
#include <mwcommon/ObsDomain.h>
#include <mwcommon/WorkDomainSpec.h>
#include <mwcommon/MWStep.h>
#include <mwcommon/MWConnectionSet.h>
#include <string>


namespace conrad { namespace cp {

  /// Master control of a distributed process.

  /// This class does the overall control of the master/worker framework.
  /// It defines the basic operations (see the enum) that can be done by the
  /// workers.
  ///
  /// Its operations are as follows:
  /// <ol>
  ///  <li> The \a setInitInfo function sends the basic info to all
  ///       workers like the name of the VDS to be used.
  ///  <li> The \a setWorkDomain function defines the work domain info
  ///       in a WorkDomainSpec object.
  ///  <li> The processSteps function does the actual processing.
  ///       It loops over the entire observation domain in work domain chunks.
  ///       For each work domain it loops over the steps to be processed.
  ///       This is done by using the MasterControl as a visitor to an MWStep.
  ///  <li> After all steps are processed, it sends a quit command to the
  ///       workers.
  /// </ol>
  /// As said above, a step is processed by using the MasterControl as
  /// an MWStepVisitor object. Usually step maps directly to an operation
  /// and prcoessing the step simply consists of sending a single command
  /// to the workers.
  /// However, in case of a solve it is more ivolved.
  /// It consists of sending multiple operations to prediffers and solver
  /// and testing if the solver has converged. This is all handled in the
  /// \a visitSolve function.
  ///
  /// Instead of using MasterControl as the visitor, it might also be
  /// possible to pass a visitor object to the MasterControl. However,
  /// apart from processing the steps the MasterControl is doing hardly
  /// anything at all, so it might be better to have anther XXXControl
  /// class resembling this one.
  /// (It might be better to rename MasterControl to BBSControl as it is
  /// modeled after the BBSKernel functionality).

  class MasterControl: public MWStepVisitor
  {
  public:
    /// Define the possible standard operations.
    enum Operation {
      /// initialize
      Init=1,
      /// set work domain
      SetWd,
      /// process a step
      Step,
      /// solvable parm info
      ParmInfo,
      /// get equations
      GetEq,
      /// do a solve step
      Solve,
      /// end the processing of a work domain
      EndWd
    };

    /// Create the master control with the giuven prediffer and solver
    /// connections.
    MasterControl (const MWConnectionSet::ShPtr& prediffers,
		   const MWConnectionSet::ShPtr& solvers);

    virtual ~MasterControl();

    /// Set the MS name to process.
    void setInitInfo (const std::string& measurementSet,
		      const std::string& inputColumn,
		      const std::string& skyParameterDB,
		      const std::string& instrumentParameterDB,
		      unsigned int subBand,
		      bool calcUVW,
		      const ObsDomain&);

    /// Set the work domain specification.
    void setWorkDomainSpec (const WorkDomainSpec&);

    /// Process a step (which can consist of multiple steps).
    void processSteps (const MWStep&);

  private:
    /// Process the various MWStep types.
    /// @{
    virtual void visitSolve    (const MWSolveStep&);
    virtual void visitCorrect  (const MWCorrectStep&);
    virtual void visitSubtract (const MWSubtractStep&);
    virtual void visitPredict  (const MWPredictStep&);
    /// @}

    /// Read the result from all prediffers.
    /// This is merely to see if all prediffers have performed the step.
    void readAllPrediffers();

    //# Data members.
    ObsDomain              itsFullDomain;
    WorkDomainSpec         itsWds;
    MWConnectionSet::ShPtr itsPrediffers;
    MWConnectionSet::ShPtr itsSolvers;
  };

}} /// end namespaces

#endif
