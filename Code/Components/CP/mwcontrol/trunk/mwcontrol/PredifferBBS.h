/// @file
/// @brief A WorkerProxy to handle BBSKernel prediffer commands.
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

#ifndef ASKAP_MWCONTROL_PREDIFFERBBS_H
#define ASKAP_MWCONTROL_PREDIFFERBBS_H

#include <mwcontrol/PredifferProxy.h>

//# Forward Declarations.
namespace LOFAR { namespace BBS {
  class Prediffer;
}}


namespace askap { namespace cp {

  /// @ingroup mwcontrol
  /// @brief A WorkerProxy to handle BBSKernel prediffer commands.

  /// This class handles the commands the WorkerControl has received.
  /// The first command is a call of the \a setInitInfo function.
  /// Thereafter \a doProcess is called which reads the message data from
  /// the blob and calls the correct BBS Prediffer function.
  ///
  /// Note that a similar class is made as a test class, which only prints the
  /// command. The \a create function registered in the WorkerFactory determines
  /// which proxy prediffer object is actually used.

class PredifferBBS: public PredifferProxy
  {
  public:
    /// Create this object and its BBS Prediffer object.
    PredifferBBS();

    ~PredifferBBS();

    /// Create a new object (to be registered in WorkerFactory).
    static WorkerProxy::ShPtr create();

    /// Set the initial Prediffer info.
    virtual void setInitInfo (const std::string& measurementSet,
			      const std::string& inputColumn,
			      const std::string& skyParameterDB,
			      const std::string& instrumentParameterDB,
			      unsigned int subBand,
			      bool calcUVW);

    /// Process the given operation. The associated data is read from the blob.
    /// An optional result can be written into the output blob.
    /// @todo Currently only one streamId is supported.
    /// A map<streamId, Prediffer*> should be used and a new Prediffer
    /// created for a new streamId.
    virtual int doProcess (int operation, int streamId,
                           LOFAR::BlobIStream& in,
                           LOFAR::BlobOStream& out);

  private:
    LOFAR::BBS::Prediffer* itsPrediffer;
  };

}} /// end namespaces

#endif
