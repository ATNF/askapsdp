/// @file
/// @brief An interface to SPECTRAL_WINDOW subtable
/// @details A class derived from this interface provides access to
/// the content of the SPECTRAL_WINDOW subtable (which provides
/// frequencies for each channel). The table is indexed with the
/// spectral window ID.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef I_TABLE_SP_WINDOW_HOLDER_H
#define I_TABLE_SP_WINDOW_HOLDER_H

// casa includes
#include <measures/Measures/MFrequency.h>
#include <casa/Arrays/Vector.h>

// own includes
#include <dataaccess/IHolder.h>

namespace conrad {

namespace synthesis {

/// @brief An interface to SPECTRAL_WINDOW subtable
/// @details A class derived from this interface provides access to
/// the content of the SPECTRAL_WINDOW subtable (which provides
/// frequencies for each channel). The table is indexed with the
/// spectral window ID.
struct ITableSpWindowHolder : virtual public IHolder {

  /// obtain the reference frame used in the spectral window table
  /// @param[in] spWindowID an index into spectral window table
  /// @return the reference frame of the given row
  virtual casa::MFrequency::Ref
                    getReferenceFrame(casa::uInt spWindowID) const = 0;

  /// @brief obtain the frequency units used in the spectral window table
  /// @details The frequency units depend on the measurement set only and
  /// are the same for all rows.
  /// @return a reference to the casa::Unit object
  virtual const casa::Unit& getFrequencyUnit() const throw() = 0;
  
  /// @brief obtain frequencies for each spectral channel
  /// @details All frequencies for each spectral channel are retreived as
  /// Doubles at once. The units and reference frame can be obtained
  /// via getReferenceFrame and getFrequencyUnit methods of this class.  
  /// @param[in] spWindowID an index into spectral window table
  /// @param[out] freqs a reference to a vector to store results to
  virtual void getFrequencies(casa::uInt spWindowID,
                   casa::Vector<Double> &freq) const = 0;

  /// @brief obtain frequency for a given spectral channel
  /// @details This version of the method is intended to obtain a
  /// frequency of a given spectral channel as fully qualified measure.
  /// The intention is to use this method if the conversion is required
  /// (and, hence, element by element operations are needed anyway)
  /// @param[in] spWindowID an index into spectral window table
  /// @param[in] channel a channel number of interest
  virtual casa::MFrequency getFrequencies(casa::uInt spWindowID,
                            casa::uInt channel) const = 0;
};


} // namespace synthesis

} // namespace conrad

#endif // #ifndef I_TABLE_SP_WINDOW_HOLDER_H
