/// @file 
/// @brief interface describing context of the parallel step
/// @details The context describes parallel specifics for 
/// a particular processing step or its part. This includes
/// methods to obtain actual communicators to send data around,
/// work domain iterators and various application specific information.
/// We rely on polymorphic behaviour to substitute objects with adapters
/// when necessary (e.g. when work domain is split between a number of
/// workers).
///
/// For more information see ASKAPSDP-1605 issue. This class plays the
/// role of the IComms class in the original design presentation attached
/// to the jira ticket.
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
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef ASKAP_ASKAPPARALLEL_I_CONTEXT_H
#define ASKAP_ASKAPPARALLEL_I_CONTEXT_H

// boost includes
#include <boost/shared_ptr.hpp>

// CASA includes
#include <casa/Arrays/IPosition.h>

// own includes
#include "communicators/IComms.h"

// standard includes
#include <string>

namespace askap {

namespace askapparallel {
	
/// @brief interface describing context of the parallel step
/// @details The context describes parallel specifics for 
/// a particular processing step or its part. This includes
/// methods to obtain actual communicators to send data around,
/// work domain iterators and various application specific information.
/// We rely on polymorphic behaviour to substitute objects with adapters
/// when necessary (e.g. when work domain is split between a number of
/// workers).
class IContext
{
public:
	
   /// @brief an empty virtual destructor to make the compiler happy
   virtual ~IContext();

   // obtain communicators for message passing

   /// @brief obtain named communicator 
   /// @details
   /// @param[in] name communicator name
   /// @return shared pointer to communicator object
   /// @note The result is guaranteed to be a non-zero pointer. 
   /// An exception is thrown if a communicator with given name
   /// does not exist.
   virtual boost::shared_ptr<IComms> getComm(const std::string &name) const = 0;

   /// @brief obtain global communicator
   /// @details
   /// Global communicator can be used to broadcast across all available ranks.
   /// It is probably a good idea to try using named communicators as much 
   /// as we can for these type of operations. 
   /// @return shared pointer to communicator object
   /// @note The result is guaranteed to be a non-zero pointer. 
   /// An exception is thrown if a communicator with given name
   /// does not exist.
   virtual boost::shared_ptr<IComms> globalComm() const; 

   /// @brief obtain local communicator
   /// @details
   /// Local communicator can be used to broadcast within ranks allocated
   /// to a particular multi-rank processing step.
   /// @return shared pointer to communicator object
   /// @note The result is guaranteed to be a non-zero pointer. 
   /// An exception is thrown if a communicator with given name
   /// does not exist.
   virtual boost::shared_ptr<IComms> localComm() const; 
  
   // iteration over work domain

   /// @brief rewind the iterator
   /// @details The iterator is brought back to the origin (same 
   /// value as just after start)
   /// @note An exception is thrown, if no iteration has been setup
   virtual void origin() = 0;

   /// @brief test whether there is more work
   /// @return true if current iteration is not complete
   /// @note this method can be used in all circumstances, even
   /// if no iteration has been setup (in this case it would return
   /// false)
   virtual bool hasMore() const = 0;

   /// @brief obtain current position of the iterator
   /// @details The new framework provides a convenient way to
   /// iterate over a hypercube with the mix of physical iteration
   /// and parallelism. This method returns the current position
   /// of the iterator. The meaning of each dimension is 
   /// defined by the user.
   /// @return current position of the iterator
   /// @note An exception is thrown, if no iteration has been setup
   virtual casa::IPosition cursor() const = 0;

   /// @brief advance iterator to the next work unit
   /// @note An exception is thrown, if no iteration has been setup
   virtual void next() = 0;

};

} // end of namespace askapparallel
} // end of namespace askap
#endif // ASKAP_ASKAPPARALLEL_I_CONTEXT_H

