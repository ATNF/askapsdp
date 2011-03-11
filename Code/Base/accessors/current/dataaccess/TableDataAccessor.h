/// @file
/// @brief an implementation of IDataAccessor for original visibility
///
/// @details TableDataAccessor is an implementation of the
/// DataAccessor for original visibility working with TableDataIterator.
/// At this moment this class just throws an exception if a write is
/// attempted and mirrors all const functions.
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
#ifndef TABLE_DATA_ACCESSOR_H
#define TABLE_DATA_ACCESSOR_H

// casa includes
#include <casa/Arrays/IPosition.h>

// own includes
#include <dataaccess/TableDataIterator.h>
#include <dataaccess/MetaDataAccessor.h>
#include <dataaccess/IDataAccessor.h>

namespace askap {
	
namespace accessors {

/// forward declaration of the type returned by reference
/// @ingroup dataaccess_tab
class TableDataIterator;

/// @brief an implementation of IDataAccessor for original visibility
///
/// @details TableDataAccessor is an implementation of the
/// DataAccessor for original (non-buffered) visibilities, i.e. visibilities
/// in the DATA column of the measurement set. It is working in pair with 
/// the TableDataIterator class. If the measurement set is writable (see
/// construction options in TableDataSource), this class allows to use
/// rwVisibility() method of the interface. An exception is thrown if the shape
/// of the updated visibility cube doesn't match the shape of the DATA column
/// when write is attempted (the write operation is delayed until the iterator
/// is progressed to the next step). Simple selections (i.e. those based on the
/// feed ID, baseline, time range) can be used together with the write operation.
/// However, polarization and spectral selections and on-the-fly averaging
/// are not supported by this class. It is not trivial and probably we don't 
/// even have a use case in ASKAP to support such operations. 
///
/// @ingroup dataaccess_tab
class TableDataAccessor : virtual public MetaDataAccessor,
                          virtual public IDataAccessor
{
public:
  /// construct an object linked with the given read-write iterator
  /// @param iter a reference to the associated read-write iterator
  explicit TableDataAccessor(const TableDataIterator &iter);

  /// Read-only visibilities (a cube is nRow x nChannel x nPol; 
  /// each element is a complex visibility)
  ///
  /// @return a reference to nRow x nChannel x nPol cube, containing
  /// all visibility data
  ///
  virtual const casa::Cube<casa::Complex>& visibility() const;
  
  
  /// Read-write access to visibilities (a cube is nRow x nChannel x nPol;
  /// each element is a complex visibility)
  ///
  /// @return a reference to nRow x nChannel x nPol cube, containing
  /// all visibility data
  ///
  virtual casa::Cube<casa::Complex>& rwVisibility();
  
  /// this method flush back the data to disk if there are any changes
  void sync() const;
private:
  /// a flag showing that the visibility has been changed and needs flushing
  /// back to the table
  mutable bool itsNeedsFlushFlag;  
  
  /// @brief A reference to associated read-write iterator
  /// @details 
  /// @note We could have obtained it from the data accessor, but
  /// this approach seems to be more general and works faster.
  const TableDataIterator &itsIterator;  
};


} // namespace accessors

} // namespace askap

#endif // #ifndef TABLE_DATA_ACCESSOR_H
