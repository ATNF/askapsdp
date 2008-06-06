/// @file
/// @brief Base class for BBSKernel worker proxies.
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

#ifndef ASKAP_MWCONTROL_BBSPROXY_H
#define ASKAP_MWCONTROL_BBSPROXY_H

#include <mwcommon/WorkerProxy.h>


namespace askap { namespace cp {

  /// @ingroup mwcontrol
  /// @brief Base class for BBSKernel worker proxies.

  /// This class is the base class for BBSKernel proxy workers.
  /// It interpretes a \a process command and handles the \a Init operation as a
  /// special case by calling the \a setInitInfo function, which has to be
  /// implemented in derived classes.
  /// Other commands are handled by the \a doProcess function which
  /// has to be implemented in derived classes as well.

  class BBSProxy: public WorkerProxy
  {
  public:
    BBSProxy()
    {}

    virtual ~BBSProxy();

    /// Process the command with the given operation and streamId.
    /// It handles the \a init operation by reading its data from
    /// the message and calling \a setInitInfo for it.
    /// Other operations are sent to \a doProcess in the derived class
    /// which can write the result into the output buffer.
    virtual int process (int operation, int streamId,
                         LOFAR::BlobIStream& in, LOFAR::BlobOStream& out);

    /// Initialise the proxy by telling the data is has to operate on.
    virtual void setInitInfo (const std::string& measurementSet,
			      const std::string& inputColumn,
			      const std::string& skyParameterDB,
			      const std::string& instrumentParameterDB,
			      unsigned int subBand,
			      bool calcUVW) = 0;

  private:
    /// Process any other command than \a init.
    /// Return the operation type of the reply.
    virtual int doProcess (int operation, int streamId,
                           LOFAR::BlobIStream& in,
                           LOFAR::BlobOStream& out) = 0;
  };

}} /// end namespaces

#endif
