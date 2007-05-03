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
#ifndef TABLEVISGRIDDER_H_
#define TABLEVISGRIDDER_H_

#include <synthesis/gridding/IVisGridder.h>

namespace conrad
{
namespace synthesis
{

class TableVisGridder : public IVisGridder
{
public:
	TableVisGridder();
	virtual ~TableVisGridder();
	
	/// Grid the visibility data onto the grid using multifrequency
	/// synthesis. Note that the weights allow complete flexibility
	/// @param uvw Input uvw locations of sample points
	/// @param visibility Input visibility samples
	/// @param frequency Input frequencies of the channels
	/// @param cellSize Input Cell sizes (wavelengths)
	/// @param grid Output grid: cube: u,v,pol
	/// @param grid Output weights: vector: pol
	virtual void forward(const casa::Vector<casa::RigidVector<casa::Double, 3> >& uvw,
			const casa::Cube<casa::Float>& visibility,
			const casa::Cube<casa::Float>& weight,
			const casa::Vector<casa::Double>& frequency,
			const casa::Vector<casa::Double>& cellSize,
			casa::Cube<casa::Complex>& grid,
			casa::Vector<casa::Float>& weights);
			
	/// Grid the spectral visibility data onto the grid
	/// Note that the weights allow complete flexibility
	/// @param uvw Input uvw locations of sample points
	/// @param visibility Input visibility samples
	/// @param frequency Input frequencies of the channels
	/// @param cellSize Input Cell sizes (wavelengths)
	/// @param grid Output grid: cube: u,v,chan,pol
	/// @param grid Output weights: vector: chan,pol
	virtual void forward(const casa::Vector<casa::RigidVector<casa::Double, 3> >& uvw,
			const casa::Cube<casa::Float>& visibility,
			const casa::Cube<casa::Float>& weight,
			const casa::Vector<casa::Double>& frequency,
			const casa::Vector<casa::Double>& cellSize,
			casa::Array<casa::Complex>& grid,
			casa::Matrix<casa::Float>& weights);
			
	/// Estimate visibility data from the grid using multifrequency
	/// synthesis. 
	/// @param uvw Input uvw locations of sample points
	/// @param frequency Input frequencies of the channels
	/// @param cellSize Input Cell sizes (wavelengths)
	/// @param grid Input grid: cube: u,v,pol
	/// @param visibility Output visibility samples
	/// @param grid Output weights: cube of same shape as visibility
	virtual void reverse(const casa::Vector<casa::RigidVector<casa::Double, 3> >& uvw,
			const casa::Cube<casa::Float>& grid, 
			const casa::Vector<casa::Double>& frequency,
			const casa::Vector<casa::Double>& cellSize,
			casa::Cube<casa::Float>& visibility,
			casa::Cube<casa::Float>& weight);

	/// Estimate spectral visibility data from the grid
	/// @param uvw Input uvw locations of sample points
	/// @param frequency Input frequencies of the channels
	/// @param cellSize Input Cell sizes (wavelengths)
	/// @param grid Input grid: cube: u,v,chan,pol
	/// @param visibility Output visibility samples
	/// @param grid Output weights: cube of same shape as visibility
	virtual void reverse(const casa::Vector<casa::RigidVector<casa::Double, 3> >& uvw,
			const casa::Array<casa::Float>& grid, 
			const casa::Vector<casa::Double>& frequency,
			const casa::Vector<casa::Double>& cellSize,
			casa::Cube<casa::Float>& visibility,
			casa::Cube<casa::Float>& weight);
			
private:
	void generic
};

}
}
#endif /*IVISGRIDDER_H_*/
