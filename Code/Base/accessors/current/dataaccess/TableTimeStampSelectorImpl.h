/// @file 
/// @brief Implementation of the time range selector
/// @details This template implements TableTimeStampSelector for the
/// various types used to specify the time range. Currently, specializations
/// exists for Double and casa::MVEpoch
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
#ifndef TABLE_TIME_STAMP_SELECTOR_IMPL_H
#define TABLE_TIME_STAMP_SELECTOR_IMPL_H

// casa includes
#include <tables/Tables/Table.h>

// own includes
#include <dataaccess/TableTimeStampSelector.h>
#include <dataaccess/TableHolder.h>

namespace askap {

namespace accessors {

/// @brief Implementation of the time range selector
/// @details This template implements TableTimeStampSelector for the
/// various types used to specify the time range. Currently, specializations
/// exists for Double and casa::MVEpoch
/// @ingroup dataaccess_tab
template<typename T>
struct TableTimeStampSelectorImpl : public TableTimeStampSelector,
                                    virtual protected TableHolder
{
   /// @brief construct time range selector
   /// @param[in] tab the table to work with
   /// @param[in] start start time of the interval
   /// @param[in] stop stop time of the interval
   TableTimeStampSelectorImpl(const casa::Table &tab, const T &start, 
                              const T &stop);
   
protected:
  
   /// @brief This method converts interval to MEpoch
   /// @details Each actual type, this template is intended to work with, 
   /// should have an explicit specialization of this method
   /// @return start and stop times of the interval to be selected (as
   ///         an std::pair, start is first, stop is second)
   virtual std::pair<casa::MEpoch, casa::MEpoch>
           getStartAndStop() const;       
               
private:
   /// @brief start time
   T itsStart;
   /// @brief stop time
   T itsStop;  
};

} // namespace accessors
} // namespace askap


#include <dataaccess/TableTimeStampSelectorImpl.tcc>

#endif // #ifndef TABLE_TIME_STAMP_SELECTOR_IMPL_H
