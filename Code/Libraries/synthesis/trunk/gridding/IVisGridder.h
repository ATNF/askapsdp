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
	/// @param cellSize Input Cell sizes (wavelengths)
	/// @param grid Output grid: cube: u,v,pol
	/// @param weights Output weights: vector: pol
	virtual void forward(const casa::Vector<double>& cellSize,
			casa::Cube<casa::Complex>& grid,
			casa::Vector<float>& weights) = 0;
			
	/// Grid the spectral visibility data onto the grid
	/// Note that the weights allow complete flexibility
	/// @param cellSize Input Cell sizes (wavelengths)
	/// @param grid Output grid: cube: u,v,chan,pol
	/// @param weights Output weights: vector: pol
	virtual void forward(const casa::Vector<double>& cellSize,
			casa::Array<casa::Complex>& grid,
			casa::Matrix<float>& weights) = 0;
			
    /// Estimate visibility data from the grid using multifrequency
    /// synthesis. 
    /// @param cellSize Input Cell sizes (wavelengths)
    /// @param grid Input grid: cube: u,v,pol
    virtual void reverse(const casa::Vector<double>& cellSize, 
        const casa::Cube<casa::Complex>& grid) = 0; 

    /// Estimate spectral visibility data from the grid
    /// @param cellSize Input Cell sizes (wavelengths)
    /// @param grid Output weights: cube of same shape as visibility
    virtual void reverse(const casa::Vector<double>& cellSize, 
        const casa::Array<casa::Complex>& grid) = 0; 
};

}
}
#endif /*IVISGRIDDER_H_*/
