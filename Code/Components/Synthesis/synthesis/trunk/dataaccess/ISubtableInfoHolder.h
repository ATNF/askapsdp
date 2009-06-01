/// @file 
/// @brief A class which holds derived information
/// @details An implementation of this interface constructs holders of
/// derived information (extracted from subtables) on demand. This
/// interface is intended to provide  access to this data stored in the
/// subtable via abstract classes of individual  holders. Examples of
/// derived information include:
///     1. feed information,
///     2. data description indices,
///     3. spectral window ids.
///     4. Polarisation information
/// Such design allows to avoid parsing of all possible subtables and
/// building all possible derived information (which can be time consuming)
/// when the measurement set is opened.
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

#ifndef I_SUBTABLE_INFO_HOLDER_H
#define I_SUBTABLE_INFO_HOLDER_H

#include <dataaccess/IHolder.h>
#include <dataaccess/ITableDataDescHolder.h>
#include <dataaccess/ITableSpWindowHolder.h>
#include <dataaccess/IBufferManager.h>
#include <dataaccess/IFeedSubtableHandler.h>
#include <dataaccess/IFieldSubtableHandler.h>
#include <dataaccess/IAntennaSubtableHandler.h>
#include <dataaccess/ITablePolarisationHolder.h>

namespace askap {

namespace synthesis {

/// @brief A class which holds derived information
/// @details An implementation of this interface constructs holders of
/// derived information (extracted from subtables) on demand. This
/// interface is intended to provide  access to this data stored in the
/// subtable via abstract classes of individual  holders. Examples of
/// derived information include:
///     1. feed information,
///     2. data description indices,
///     3. spectral window ids.
///     4. Polarisation information
/// Such design allows to avoid parsing of all possible subtables and
/// building all possible derived information (which can be time consuming)
/// when the measurement set is opened.
/// @ingroup dataaccess_tm
struct ISubtableInfoHolder : virtual public IHolder {

   /// @return a reference to the handler of the DATA_DESCRIPTION subtable
   virtual const ITableDataDescHolder& getDataDescription() const = 0;

   /// @return a reference to the handler of the SPECTRAL_WINDOW subtable
   virtual const ITableSpWindowHolder& getSpWindow() const = 0;
   
   /// @return a reference to the handler of the POLARIZATION subtable
   virtual const ITablePolarisationHolder& getPolarisation() const = 0;

   /// @return a reference to the manager of buffers (BUFFERS subtable)
   virtual const IBufferManager& getBufferManager() const = 0;
   
   /// @return a reference to the handler of the FEED subtable
   virtual const IFeedSubtableHandler& getFeed() const = 0;
   
   /// @return a reference to the handler of the FIELD subtable
   virtual const IFieldSubtableHandler& getField() const = 0;

   /// @return a reference to the handler of the ANTENNA subtable
   virtual const IAntennaSubtableHandler& getAntenna() const = 0;
};


} // namespace synthesis

} // namespace askap

#endif // #ifndef I_SUBTABLE_INFO_HOLDER_H
