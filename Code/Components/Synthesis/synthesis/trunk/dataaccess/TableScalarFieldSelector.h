/// @file TableScalarFieldSelector.h
/// @brief An implementation of ITableDataSelectorImpl for simple (scalar) fields, like feed ID.
/// @details This class represents a selection of visibility
///         data according to some criterion. This is an
///         implementation of the part of the IDataSelector
///         interface, which can be done with the table selection
///         mechanism in the table based case. Only simple
///         (scalar) fields are included in this selection.
///         Epoch-based selection is done via a separate class
///         because a fully defined converter is required to
///         perform such selection.
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///
#ifndef TABLE_SCALAR_FIELD_SELECTOR_H
#define TABLE_SCALAR_FIELD_SELECTOR_H

// casa includes
#include <tables/Tables/ExprNode.h>
#include <tables/Tables/Table.h>

// own includes
#include <dataaccess/ITableDataSelectorImpl.h>
#include <dataaccess/IDataConverter.h>
#include <dataaccess/ITableHolder.h>
#include <dataaccess/ITableInfoAccessor.h>

namespace askap {

namespace synthesis {
	
/// @brief An implementation of ITableDataSelectorImpl for simple (scalar) fields, like feed ID.
/// @details This class represents a selection of visibility
///         data according to some criterion. This is an
///         implementation of the part of the IDataSelector
///         interface, which can be done with the table selection
///         mechanism in the table based case. Only simple
///         (scalar) fields are included in this selection.
///         Epoch-based selection is done via a separate class
///         because a fully defined converter is required to
///         perform such selection.
///
/// A derivative from this class is passed to a DataSource object in the
/// request for an iterator. The iterator obtained that way runs through
/// the selected part of the dataset.
/// @ingroup dataaccess_tab
class TableScalarFieldSelector : virtual public ITableDataSelectorImpl,
				 virtual protected ITableInfoAccessor
{
public:
  
  /// Choose a single feed, the same for both antennae
  /// @param[in] feedID the sequence number of feed to choose
  virtual void chooseFeed(casa::uInt feedID);

  /// Choose a single baseline
  /// @param[in] ant1 the sequence number of the first antenna
  /// @param[in] ant2 the sequence number of the second antenna
  /// Which one is the first and which is the second is not important
  virtual void chooseBaseline(casa::uInt ant1, casa::uInt ant2);

  /// Choose a single spectral window (also known as IF).
  /// @param[in] spWinID the ID of the spectral window to choose
  virtual void chooseSpectralWindow(casa::uInt spWinID);
  
  /// @brief Choose autocorrelations only
  virtual void chooseAutoCorrelations();
  
  /// @brief Choose crosscorrelations only
  virtual void chooseCrossCorrelations();
  
  /// @brief Choose samples corresponding to a uv-distance larger than threshold
  /// @details This effectively rejects the baselines giving a smaller
  /// uv-distance than the specified threshold
  /// @param[in] uvDist threshold (in metres)
  virtual void chooseMinUVDistance(casa::Double uvDist);

  /// @brief Choose samples corresponding to a uv-distance smaller than threshold
  /// @details This effectively rejects the baselines giving a larger
  /// uv-distance than the specified threshold
  /// @param[in] uvDist threshold (in metres)
  virtual void chooseMaxUVDistance(casa::Double uvDist);
      
protected:
  /// Obtain a table expression node for selection. This method is
  /// used in the implementation of the iterator to form a subtable
  /// obeying the selection criteria specified by the user via
  /// IDataSelector interface
  /// 
  /// @return a reference to the cached table expression node
  ///
  casa::TableExprNode& getTableSelector() const;

private:
  /// a current table selection expression (cache)
  mutable casa::TableExprNode  itsTableSelector;  
};
  
} // namespace synthesis
  
} // namespace askap
  
#endif // #ifndef TABLE_SCALAR_FIELD_SELECTOR_H
