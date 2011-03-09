/// @file 
/// @brief Implementation of IDataSource in the table-based case
/// @details
/// TableDataSource: Allow read-write access to the data stored in the
/// measurement set. This class implements IConstDataSource interface.
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

// casa includes
#include <tables/Tables/TableRecord.h>

// own includes
#include <dataaccess/TableDataSource.h>
#include <dataaccess/TableDataIterator.h>
#include <dataaccess/DataAccessError.h>
#include <dataaccess/IDataConverterImpl.h>
#include <dataaccess/ITableDataSelectorImpl.h>
#include <dataaccess/SubtableInfoHolder.h>

using namespace askap;
using namespace askap::synthesis;

/// construct a read-write data source object
/// @param[in] fname file name of the measurement set to use
/// @param[in] opt options from TableDataSourceOptions, can be or'ed
/// removed, if it already exists.   
/// @param[in] dataColumn a name of the data column used by default
///                       (default is DATA)
TableDataSource::TableDataSource(const std::string &fname,
                int opt, const std::string &dataColumn) :
         TableInfoAccessor(casa::Table(fname, (opt & MEMORY_BUFFERS) && 
				  !(opt & REMOVE_BUFFERS) && !(opt & WRITE_PERMITTED) ? 
				      casa::Table::Old : casa::Table::Update),
						opt & MEMORY_BUFFERS, dataColumn)
{
  if (opt & REMOVE_BUFFERS) {
      if (table().keywordSet().isDefined("BUFFERS")) {
          try {
            table().rwKeywordSet().asTable("BUFFERS").markForDelete();
	  }
	  catch(...) {}
          table().rwKeywordSet().removeField("BUFFERS");
      }
  }
}

/// @brief obtain a read/write iterator
/// @details 
/// get a read/write iterator over a selected part of the dataset 
/// represented by this DataSource object with an explicitly 
/// specified conversion policy. This is the most general 
/// createIterator(...) call, which
/// is used as a default implementation for all less general cases
/// (although they can be overriden in the derived classes, if it 
///  will be necessary because of the performance issues)
///
/// @param[in] sel a shared pointer to the selector object defining 
///            which subset of the data is used
/// @param[in] conv a shared pointer to the converter object defining
///            reference frames and units to be used
/// @return a shared pointer to DataIterator object
/// @note
/// The method acts as a factory by creating a new DataIterator.
/// The lifetime of this iterator is the same as the lifetime of
/// the DataSource object. Therefore, it can be reused multiple times,
/// if necessary. Call init() to rewind the iterator.
boost::shared_ptr<IDataIterator> TableDataSource::createIterator(const
           IDataSelectorConstPtr &sel, const
	   IDataConverterConstPtr &conv) const
{
 // cast input selector to "implementation" interface
   boost::shared_ptr<ITableDataSelectorImpl const> implSel=
           boost::dynamic_pointer_cast<ITableDataSelectorImpl const>(sel);
   boost::shared_ptr<IDataConverterImpl const> implConv=
           boost::dynamic_pointer_cast<IDataConverterImpl const>(conv);
   	   
   if (!implSel || !implConv) {
       ASKAPTHROW(DataAccessLogicError, "Incompatible selector and/or "<<
                 "converter are received by the createIterator method");
   }
   return boost::shared_ptr<IDataIterator>(new TableDataIterator(
                getTableManager(),implSel,implConv,uvwMachineCacheSize(),
                uvwMachineCacheTolerance())); 
}
