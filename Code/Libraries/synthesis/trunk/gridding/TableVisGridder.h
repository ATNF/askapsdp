/// @file
///
/// TableVisGridder: Table-based visibility gridder. 
///
/// This supports gridders with a table loopkup.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef TABLEVISGRIDDER_H_
#define TABLEVISGRIDDER_H_

#include <gridding/IVisGridder.h>

namespace conrad
{
namespace synthesis
{

class TableVisGridder : public IVisGridder
{
public:

	// Types of convolution function
	enum Type {
		STANDARD=0,
		WPROJECTION,
		ILLUMINATION,
		TABLE
	};
	
	// Standard two dimensional gridding
	TableVisGridder();
	
//	// W projection gridding
//	TableVisGridder(const int wPlanes, const float maxBaseline);
//	
//	// Illumination pattern
//	TableVisGridder(const float diameter, const float blockage);
//
//	// Two dimensional table-based
//	TableVisGridder(const casa::Matrix<float>& table, const uint support,
//		const uint overSample);
//		 
	virtual ~TableVisGridder();
	
	/// Grid the visibility data onto the grid using multifrequency
	/// synthesis. Note that the weights allow complete flexibility
	/// @param ida Data Accessor
	/// @param cellSize Input Cell sizes (wavelengths)
	/// @param grid Output grid: cube: u,v,pol
	/// @param weights Output weights: vector: pol
	virtual void forward(const IDataAccessor& ida,
			const casa::Vector<double>& cellSize,
			casa::Cube<casa::Complex>& grid,
			casa::Vector<float>& weights);
			
	/// Grid the spectral visibility data onto the grid
	/// Note that the weights allow complete flexibility
	/// @param ida Data Accessor
	/// @param cellSize Input Cell sizes (wavelengths)
	/// @param grid Output grid: cube: u,v,chan,pol
	/// @param weights Output weights: vector: pol
	virtual void forward(const IDataAccessor& ida,
			const casa::Vector<double>& cellSize,
			casa::Array<casa::Complex>& grid,
			casa::Matrix<float>& weights);
			
	/// Estimate visibility data from the grid using multifrequency
	/// synthesis. 
	/// @param ida Data Accessor
	/// @param cellSize Input Cell sizes (wavelengths)
	/// @param grid Input grid: cube: u,v,pol
	virtual void reverse(IDataAccessor& ida, 
			const casa::Cube<casa::Complex>& grid, 
			const casa::Vector<double>& cellSize);

	/// Estimate spectral visibility data from the grid
	/// @param ida Data Accessor
	/// @param cellSize Input Cell sizes (wavelengths)
	/// @param grid Output weights: cube of same shape as visibility
	virtual void reverse(IDataAccessor& ida, 
			const casa::Array<casa::Complex>& grid, 
			const casa::Vector<double>& cellSize);

			
private:
	void genericForward(const casa::Vector<casa::RigidVector<casa::Double, 3> >& uvw,
					const casa::Cube<casa::Complex>& visibility,
					const casa::Cube<float>& visweight,
					const casa::Vector<double>& freq,
					const casa::Vector<double>& cellSize,
					const casa::Cube<float>& C,
					const int support,
					const int overSample,
					const casa::Matrix<uint>& cOffset,
					casa::Cube<casa::Complex>& grid,
					casa::Vector<float>& sumwt);
					
	void genericReverse(const casa::Vector<casa::RigidVector<casa::Double, 3> >& uvw,
					casa::Cube<casa::Complex>& visibility,
					casa::Cube<float>& visweight,
					const casa::Vector<double>& freq,
					const casa::Vector<double>& cellSize,
					const casa::Cube<float>& C,
					const int support,
					const int overSample,
					const casa::Matrix<uint>& cOffset,
					const casa::Cube<casa::Complex>& grid);
					
	void initConvolutionFunction();
};

}
}
#endif /*IVISGRIDDER_H_*/
