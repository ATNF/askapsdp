/// @file IPolSelector.h
/// @brief Interface to select a number of polarization products
/// @details Examples of the polarization selection are, e.g. circulars
/// with cross products, full stokes, stokes I only, etc.
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef I_POL_SELECTOR_H
#define I_POL_SELECTOR_H

namespace askap {

namespace synthesis {

/// @brief Interface to select a number of polarization products
/// @details Examples of the polarization selection are, e.g. circulars
/// with cross products, full stokes, stokes I only, etc.
/// @ingroup dataaccess_i
class IPolSelector {
public:
    /// non-explicit constructor to allow conversion from the strings like
    /// "XX,YY,XY,YX"
    /// @param[in] stokes requested polarizations, i.e. a string like
    /// "XX,YY,XY,YX"
    IPolSelector(const std::string &stokes);
    
    /// an empty virtual destructor to make the compiler happy
    virtual ~IPolSelector();

    /// @return a number of polarization products selected
    size_t nPol() const throw();

    /// @return true if the selection is a subset of circular polarization
    ///              products
    bool isCircular() const throw();

    /// @return true if the selection is a subset of linear polarization
    ///              products
    bool isLinear() const throw();

    /// @return true if the selection is a subset of stokes parameters
    bool isStokes() const throw();
};
} // namespace synthesis

} // namespace askap

#endif // #ifndef I_POL_SELECTOR_H
