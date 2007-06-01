/// @file
///
/// IPolSelector: Interface to select a number of polarization products
///               Examples are, e.g. circulars with cross products,
///               full stokes, stokes I only, etc.
///
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef I_POL_SELECTOR_H
#define I_POL_SELECTOR_H

namespace conrad {

namespace synthesis {

class IPolSelector {
public:
    /// non-explicit constructor to allow conversion from the strings like
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

} // namespace conrad

#endif // #ifndef I_POL_SELECTOR_H
