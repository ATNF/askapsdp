/// @file
///
/// NormalEquations: Hold the normal equations for parameters
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef SCIMATHNORMALEQUATIONS_H_
#define SCIMATHNORMALEQUATIONS_H_

#include <fitting/Params.h>

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Slice.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/IPosition.h>

namespace conrad
{
namespace scimath
{
	
class DesignMatrix;
    
class NormalEquations 
/// Fully general normal equations
{
public:

    NormalEquations() {};
    
    /// Define the normal equations
    /// @param ip Parameters
    NormalEquations(const Params& ip);
    
    /// Copy constructor
    NormalEquations(const NormalEquations& normeq);

    /// Assignment operator
    NormalEquations& operator=(const NormalEquations& normeq);
    
    virtual ~NormalEquations();
    
    /// Construct the normal equations from the design matrix
    /// @param dm Design matrix
    /// @param approx Type of approximation to be used
    NormalEquations(const DesignMatrix& dm);
    
    /// Add the design matrix to the normal equations
    /// @param dm Design matrix
    /// @param approx Type of approximation to be used
    void add(const DesignMatrix& dm);

    /// Return the specified parameters
   const Params& parameters() const;
   Params& parameters();

    /// @param name Name of parameter
    /// @param normalmatrix Normal Matrix for this parameter
    /// @param datavector Data vector for this parameter
    /// @param shape Shape of this parameter
    void add(const string& name, const casa::Matrix<double>& normalmatrix,
        const casa::Vector<double>& datavector);

    void add(const string& name, const casa::Matrix<double>& normalmatrix,
        const casa::Vector<double>& datavector,
        const casa::IPosition& shape);

    void addSlice(const string& name, 
        const casa::Vector<double>& normalmatrixslice,
        const casa::Vector<double>& normalmatrixdiagonal,
        const casa::Vector<double>& datavector,
        const casa::IPosition& shape,
        const casa::IPosition& reference);

    void addSlice(const string& name, 
        const casa::Vector<double>& normalmatrixslice,
        const casa::Vector<double>& normalmatrixdiagonal,
        const casa::Vector<double>& datavector,
        const casa::IPosition& reference);

    void addDiagonal(const string& name, const casa::Vector<double>& normalmatrix,
        const casa::Vector<double>& datavector,
        const casa::IPosition& shape);

    void addDiagonal(const string& name, const casa::Vector<double>& normalmatrix,
        const casa::Vector<double>& datavector);

    /// Merge these normal equations with another - means that we just add
    /// @param other Other normal equations
    void merge(const NormalEquations& other);
    
    /// Return normal equations
    std::map<string, std::map<string, casa::Matrix<double> > >& normalMatrix() const;
    
    /// Return normal equations slice
    std::map<string, casa::Vector<double> >& normalMatrixSlice() const;
    
    /// Return normal equations diagonal
    std::map<string, casa::Vector<double> >& normalMatrixDiagonal() const;
    
    /// Return data vector
    std::map<string, casa::Vector<double> >& dataVector() const;
    
    /// Return shape
    std::map<string, casa::IPosition>& shape() const;
    
    /// Return references
    std::map<string, casa::IPosition >& NormalEquations::reference() const;
  
    /// Reset to empty
    void reset();
protected:
    Params itsParams;
    // Note that this is a very flexible format - it allows any of the
    // approximations to be used
    mutable std::map<string, std::map<string, casa::Matrix<double> > > itsNormalMatrix;
    mutable std::map<string, casa::Vector<double> > itsNormalMatrixSlice;
    mutable std::map<string, casa::Vector<double> > itsNormalMatrixDiagonal;
    mutable std::map<string, casa::IPosition> itsShape;
    mutable std::map<string, casa::IPosition> itsReference;
    mutable std::map<string, casa::Vector<double> > itsDataVector;
};


}
}
#endif /*NORMALEQUATIONS_H_*/
