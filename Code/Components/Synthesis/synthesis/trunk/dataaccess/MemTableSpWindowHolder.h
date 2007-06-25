/// @file
/// @brief Implementation of ITableSpWindowHolder
/// @details This file contains a class, which reads and stores 
/// the content of the SPECTRAL_WINDOW subtable (which provides
/// frequencies for each channel). The table is indexed with the
/// spectral window ID.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef MEM_TABLE_SP_WINDOW_HOLDER_H
#define MEM_TABLE_SP_WINDOW_HOLDER_H

// casa includes
#include <measures/Measures/MFrequency.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Array.h>
#include <tables/Tables/Table.h>
#include <casa/Quanta/Unit.h>

// own includes
#include <dataaccess/ITableSpWindowHolder.h>

namespace conrad {

namespace synthesis {

/// @brief Memory-based implementation of ITableSpWindowHolder
/// @details This class reads and stores in memory the content
/// of the SPECTRAL_WINDOW subtable (which provides
/// frequencies for each channel). The table is indexed with the
/// spectral window ID.
struct MemTableSpWindowHolder : virtual public ITableSpWindowHolder {

  /// read all required information from the SPECTRAL_WINDOW subtable
  /// @param ms an input measurement set (in fact any table which has a
  /// SPECTRAL_WINDOW subtable defined)
  explicit MemTableSpWindowHolder(const casa::Table &ms);

  /// obtain the reference frame used in the spectral window table
  /// @param[in] spWindowID an index into spectral window table
  /// @return the reference frame of the given row
  virtual casa::MFrequency::Ref
                    getReferenceFrame(casa::uInt spWindowID) const;

  /// @brief obtain the frequency units used in the spectral window table
  /// @details The frequency units depend on the measurement set only and
  /// are the same for all rows.
  /// @return a reference to the casa::Unit object
  virtual const casa::Unit& getFrequencyUnit() const throw();
  
  /// @brief obtain frequencies for each spectral channel
  /// @details All frequencies for each spectral channel are retreived as
  /// Doubles at once. The units and reference frame can be obtained
  /// via getReferenceFrame and getFrequencyUnit methods of this class.  
  /// @param[in] spWindowID an index into spectral window table
  /// @return freqs a const reference to a vector with result
  virtual const casa::Vector<casa::Double>&
                     getFrequencies(casa::uInt spWindowID) const;

  /// @brief obtain frequency for a given spectral channel
  /// @details This version of the method is intended to obtain a
  /// frequency of a given spectral channel as fully qualified measure.
  /// The intention is to use this method if the conversion is required
  /// (and, hence, element by element operations are needed anyway)
  /// @param[in] spWindowID an index into spectral window table
  /// @param[in] channel a channel number of interest
  virtual casa::MFrequency getFrequencies(casa::uInt spWindowID,
                            casa::uInt channel) const;
private:
  // reference frame ids for each row (spectral window ID)
  casa::Vector<casa::Int>  itsMeasRefIDs;
  
  // a buffer for channel frequencies
  // we use Vector of Vectors as each cell can, in principle, have
  // different shape
  casa::Vector<casa::Vector<casa::Double> > itsChanFreqs;
  
  // frequency units used in the table (and therefore used in itsChanFreqs)
  casa::Unit itsFreqUnits;
};


} // namespace synthesis

} // namespace conrad

#endif // #ifndef MEM_TABLE_SP_WINDOW_HOLDER_H
