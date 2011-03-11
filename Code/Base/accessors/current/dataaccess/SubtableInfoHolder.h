/// @file 
/// @brief Implementation of ISubtableInfoHolder
/// @details This class manages and constructs handlers of
/// derived information (extracted from subtables) on demand.
/// The access to this information is via abstract classes of
/// individual  holders. Examples of derived information include:
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

#ifndef SUBTABLE_INFO_HOLDER_H
#define SUBTABLE_INFO_HOLDER_H

// boost includes
#include <boost/shared_ptr.hpp>

// own includes
#include <dataaccess/ISubtableInfoHolder.h>
#include <dataaccess/ITableHolder.h>
#include <dataaccess/ITableDataDescHolder.h>

namespace askap {

namespace accessors {

/// @brief Implementation of ISubtableInfoHolder
/// @details This class manages and constructs handlers of
/// derived information (extracted from subtables) on demand.
/// The access to this information is via abstract classes of
/// individual  holders. Examples of derived information include:
///     1. feed information,
///     2. data description indices,
///     3. spectral window ids.
///     4. Polarisation information
/// Such design allows to avoid parsing of all possible subtables and
/// building all possible derived information (which can be time consuming)
/// when the measurement set is opened.
/// @ingroup dataaccess_tm
struct SubtableInfoHolder : virtual public ISubtableInfoHolder,
                            virtual public ITableHolder
{
   /// @brief construct SubtableInfoHolder
   /// @details The idea is that this constructor is the point where one can choose
   /// how the lower level management is done (i.e. disk or memory based buffers). 
   /// In the future, more arguments can be received by this constructor. It is probably
   /// practical to provide reasonable defaults here
   /// @param memBuffers true if the buffers should be held in memory, false if they should be
   /// written back to the disk (table needs to be writable for this)
   explicit SubtableInfoHolder(bool memBuffers = false);

   /// @brief obtain data description holder
   /// @details A MemTableDataDescHolder is constructed on the first call
   /// to this method and a reference to it is always returned later
   /// @return a reference to the handler of the DATA_DESCRIPTION subtable
   virtual const ITableDataDescHolder& getDataDescription() const;

   /// @brief obtain spectral window holder
   /// @details A MemTableSpWindowHolder is constructed on the first call
   /// to this method and a reference to it is always returned later
   /// @return a reference to the handler of the SPECTRAL_WINDOW subtable
   virtual const ITableSpWindowHolder& getSpWindow() const;
   
   /// @brief obtain polarisation information holder
   /// @details A MemTablePolarisationHolder is constructed on the first call
   /// to this method and a reference to it is always returned later   
   /// @return a reference to the handler of the POLARIZATION subtable
   virtual const ITablePolarisationHolder& getPolarisation() const;
   

   /// @brief obtain a manager of buffers
   /// @details A TableBufferManager is constructed on the first call
   /// to this method, which makes BUFFERS subtable if it is not yet
   /// present.
   /// @return a reference to the manager of buffers (BUFFERS subtable)
   virtual const IBufferManager& getBufferManager() const;
     
   /// @brief obtain a feed subtable handler
   /// @details A FeedSubtableHandler is constructred on the first call to
   /// this method and a reference to it is always returne later
   /// @return a reference to the handler of the FEED subtable
   virtual const IFeedSubtableHandler& getFeed() const;
   
   /// @brief obtain a field subtable handler
   /// @details A FieldSubtableHandler is consructed on the first call to this
   /// method and a reference to it is returned thereafter.
   /// @return a reference to the handler of the FIELD subtable
   virtual const IFieldSubtableHandler& getField() const;
   
   /// @brief obtain an antenna subtable handler
   /// @details A MemAntennaSubtableHandler is constructed on the first call
   /// to this method and a reference to it is returned thereafter
   /// @return a reference to the handler of the ANTENNA subtable
   virtual const IAntennaSubtableHandler& getAntenna() const;
   
   
protected:   

   /// initialize itsBufferManager with an instance of TableBufferManager
   void initBufferManager() const;
   
private:
   /// smart pointer to the handler of the data description subtable
   mutable boost::shared_ptr<ITableDataDescHolder const> itsDataDescHandler;

   /// smart pointer to the handler of the spectral window subtable
   mutable boost::shared_ptr<ITableSpWindowHolder const> itsSpWindowHandler;

   /// smart pointer to the handler of polarisation subtable
   mutable boost::shared_ptr<ITablePolarisationHolder const> itsPolarisationHandler;

   /// smart pointer to the buffer manager
   mutable boost::shared_ptr<IBufferManager const> itsBufferManager;
   
   /// true if visibility buffers are kept in memory
   bool itsUseMemBuffers;

   /// smart pointer to the feed subtable handler
   mutable boost::shared_ptr<IFeedSubtableHandler const> itsFeedHandler;
   
   /// smart pointer to the field subtable handler
   mutable boost::shared_ptr<IFieldSubtableHandler const> itsFieldHandler;
   
   /// smart pointer to the antenna subtable handler
   mutable boost::shared_ptr<IAntennaSubtableHandler const> itsAntennaHandler;
};


} // namespace accessors

} // namespace askap

#endif // #ifndef SUBTABLE_INFO_HOLDER_H
