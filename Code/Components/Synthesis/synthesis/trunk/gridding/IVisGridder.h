/// @file
///
/// IVisGridder: Interface definition for visibility gridders
///
/// Gridders derived from this base are intentionally designed to
/// work with visibility data accessed via the synthesis/dataaccess
/// classes. They are not intended to be generate purpose gridding
/// classes.
///
/// Multi-frequency synthesis and normal spectral gridding are 
/// supported by different methods.
///
/// No phase rotation is performed in the gridder.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef IVISGRIDDER_H_
#define IVISGRIDDER_H_


#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>

#include <fitting/Axes.h>

#include <boost/shared_ptr.hpp>
#include <dataaccess/IDataIterator.h>
#include <dataaccess/SharedIter.h>

namespace conrad
{
namespace synthesis
{

class IVisGridder
{
public:
	IVisGridder();
	virtual ~IVisGridder();
	
	/// Grid the visibility data onto the grid using multifrequency
	/// synthesis. Note that the weights allow complete flexibility
    /// @param idi DataIterator
    /// @param axes axes specifications
	/// @param grid Output grid: cube: u,v,pol
	/// @param weights Output weights: vector: pol
	virtual void reverse(IDataSharedIter& idi,
            const scimath::Axes& axes,
			casa::Cube<casa::Complex>& grid,
			casa::Vector<float>& weights) = 0;
			
	/// Grid the spectral visibility data onto the grid
	/// Note that the weights allow complete flexibility
    /// @param idi DataIterator
    /// @param axes axes specifications
	/// @param grid Output grid: cube: u,v,chan,pol
	/// @param weights Output weights: vector: pol
	virtual void reverse(IDataSharedIter& idi,
            const scimath::Axes& axes,
			casa::Array<casa::Complex>& grid,
			casa::Matrix<float>& weights) = 0;
			
    /// Estimate visibility data from the grid using multifrequency
    /// synthesis. 
    /// @param idi DataIterator
    /// @param axes axes specifications
    /// @param grid Input grid: cube: u,v,pol
    virtual void forward(IDataSharedIter& idi,
        const scimath::Axes& axes,
        const casa::Cube<casa::Complex>& grid) = 0; 

    /// Estimate spectral visibility data from the grid
    /// @param idi DataIterator
    /// @param axes axes specifications
    /// @param grid Output weights: cube of same shape as visibility
    virtual void forward(IDataSharedIter& idi,
        const scimath::Axes& axes,
        const casa::Array<casa::Complex>& grid) = 0; 
        
    /// Correct for gridding convolution function
    /// @param axes axes specifications
    /// @param image image to be corrected
    virtual void correctConvolution(const scimath::Axes& axes,
        casa::Cube<double>& image) = 0;
    virtual void correctConvolution(const scimath::Axes& axes,
        casa::Array<double>& image) = 0;
        
};

}
}
#endif /*IVISGRIDDER_H_*/
