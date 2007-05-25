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
#include <boost/shared_ptr.hpp>
#include <dataaccess/IDataIterator.h>
#include <dataaccess/SharedIter.h>

namespace conrad
{
namespace synthesis
{

class TableVisGridder : public IVisGridder
{
public:
	
	// Standard two dimensional gridding
	TableVisGridder(IDataSharedIter& idi);
	
	virtual ~TableVisGridder();
	
	/// Grid the visibility data onto the grid using multifrequency
	/// synthesis. Note that the weights allow complete flexibility
	/// @param cellSize Input Cell sizes (wavelengths)
	/// @param grid Output grid: cube: u,v,pol
	/// @param weights Output weights: vector: pol
	virtual void reverse(const casa::Vector<double>& cellSize,
			casa::Cube<casa::Complex>& grid,
			casa::Vector<float>& weights);
			
	/// Grid the spectral visibility data onto the grid
	/// Note that the weights allow complete flexibility
	/// @param cellSize Input Cell sizes (wavelengths)
	/// @param grid Output grid: cube: u,v,chan,pol
	/// @param weights Output weights: vector: pol
	virtual void reverse(const casa::Vector<double>& cellSize,
			casa::Array<casa::Complex>& grid,
			casa::Matrix<float>& weights);
			
	/// Estimate visibility data from the grid using multifrequency
	/// synthesis. 
	/// @param cellSize Input Cell sizes (wavelengths)
	/// @param grid Input grid: cube: u,v,pol
	virtual void forward(const casa::Vector<double>& cellSize, const casa::Cube<casa::Complex>& grid); 

	/// Estimate spectral visibility data from the grid
	/// @param cellSize Input Cell sizes (wavelengths)
	/// @param grid Output weights: cube of same shape as visibility
	virtual void forward(const casa::Vector<double>& cellSize, const casa::Array<casa::Complex>& grid); 

protected:

    casa::Cube<float> itsC;
    int itsSupport;
    int itsOverSample;
    int itsCSize;
    int itsCCenter;
    virtual int cOffset(int, int)=0;
			
private:
    IDataSharedIter itsIdi;
	void genericReverse(const casa::Vector<casa::RigidVector<casa::Double, 3> >& uvw,
					const casa::Cube<casa::Complex>& visibility,
					const casa::Cube<float>& visweight,
					const casa::Vector<double>& freq,
					const casa::Vector<double>& cellSize,
					casa::Cube<casa::Complex>& grid,
					casa::Vector<float>& sumwt);
					
	void genericForward(const casa::Vector<casa::RigidVector<casa::Double, 3> >& uvw,
					casa::Cube<casa::Complex>& visibility,
					casa::Cube<float>& visweight,
					const casa::Vector<double>& freq,
					const casa::Vector<double>& cellSize,
					const casa::Cube<casa::Complex>& grid);
					
	void initConvolutionFunction();
};

}
}
#endif
