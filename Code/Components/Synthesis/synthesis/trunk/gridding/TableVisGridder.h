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
	
	// Standard two dimensional gridding
	TableVisGridder();
	
	virtual ~TableVisGridder();
	
    /// Grid the visibility data onto the grid using multifrequency
    /// synthesis. Note that the weights allow complete flexibility
    /// @param idi DataIterator
    /// @param axes axes specifications
    /// @param grid Output grid: cube: u,v,pol
    /// @param weights Output weights: vector: pol
    virtual void reverse(IDataSharedIter& idi,
            const scimath::Axes& axes,
            casa::Cube<casa::Complex>& grid,
            casa::Vector<float>& weights);
            
    /// Grid the spectral visibility data onto the grid
    /// Note that the weights allow complete flexibility
    /// @param idi DataIterator
    /// @param axes axes specifications
    /// @param grid Output grid: cube: u,v,chan,pol
    /// @param weights Output weights: vector: pol
    virtual void reverse(IDataSharedIter& idi,
            const scimath::Axes& axes,
            casa::Array<casa::Complex>& grid,
            casa::Matrix<float>& weights);
            
    /// Grid the visibility data onto the grid using multifrequency
    /// synthesis. Note that the weights allow complete flexibility
    /// @param idi DataIterator
    /// @param axes axes specifications
    /// @param grid Output grid: cube: u,v,pol
    /// @param weights Output weights: vector: pol
    virtual void reverseWeights(IDataSharedIter& idi,
            const scimath::Axes& axes,
            casa::Cube<casa::Complex>& grid);
            
    /// Grid the spectral visibility data onto the grid
    /// Note that the weights allow complete flexibility
    /// @param idi DataIterator
    /// @param axes axes specifications
    /// @param grid Output grid: cube: u,v,chan,pol
    /// @param weights Output weights: vector: pol
    virtual void reverseWeights(IDataSharedIter& idi,
            const scimath::Axes& axes,
            casa::Array<casa::Complex>& grid);
            
    /// Estimate visibility data from the grid using multifrequency
    /// synthesis. 
    /// @param idi DataIterator
    /// @param axes axes specifications
    /// @param grid Input grid: cube: u,v,pol
    virtual void forward(IDataSharedIter& idi,
        const scimath::Axes& axes,
        const casa::Cube<casa::Complex>& grid); 

    /// Estimate spectral visibility data from the grid
    /// @param idi DataIterator
    /// @param axes axes specifications
    /// @param grid Output weights: cube of same shape as visibility
    virtual void forward(IDataSharedIter& idi,
        const scimath::Axes& axes,
        const casa::Array<casa::Complex>& grid); 

    /// Correct for gridding convolution function
    /// @param axes axes specifications
    /// @param image image to be corrected
    virtual void correctConvolution(const scimath::Axes& axes,
        casa::Cube<double>& image);
        
    /// Apply gridding convolution function
    /// @param axes axes specifications
    /// @param image image to be corrected
    virtual void applyConvolution(const scimath::Axes& axes,
        casa::Cube<double>& image);
        

protected:

    casa::Cube<float> itsC;
    int itsSupport;
    int itsOverSample;
    int itsCSize;
    int itsCCenter;
    virtual int cOffset(int, int)=0;
    virtual void initConvolutionFunction(IDataSharedIter& idi, const casa::Vector<double>& cellSize,
        const casa::IPosition& shape);
    void findCellsize(casa::Vector<double>& cellSize, const casa::IPosition& imageShape, 
        const scimath::Axes& axes);
    
			
private:
    void genericReverse(const casa::Vector<casa::RigidVector<casa::Double, 3> >& uvw,
                    const casa::Cube<casa::Complex>& visibility,
                    const casa::Cube<float>& visweight,
                    const casa::Vector<double>& freq,
                    const casa::Vector<double>& cellSize,
                    casa::Cube<casa::Complex>& grid,
                    casa::Vector<float>& sumwt);
                    
    void genericReverseWeights(const casa::Vector<casa::RigidVector<casa::Double, 3> >& uvw,
                    const casa::Cube<float>& visweight,
                    const casa::Vector<double>& freq,
                    const casa::Vector<double>& cellSize,
                    casa::Cube<casa::Complex>& grid);
                    
    void genericForward(const casa::Vector<casa::RigidVector<casa::Double, 3> >& uvw,
                    casa::Cube<casa::Complex>& visibility,
                    casa::Cube<float>& visweight,
                    const casa::Vector<double>& freq,
                    const casa::Vector<double>& cellSize,
                    const casa::Cube<casa::Complex>& grid);
                    
    void genericReverse(const casa::Vector<casa::RigidVector<casa::Double, 3> >& uvw,
                    const casa::Cube<casa::Complex>& visibility,
                    const casa::Cube<float>& visweight,
                    const casa::Vector<double>& freq,
                    const casa::Vector<double>& cellSize,
                    casa::Array<casa::Complex>& grid,
                    casa::Matrix<float>& sumwt);
                    
    void genericReverseWeights(const casa::Vector<casa::RigidVector<casa::Double, 3> >& uvw,
                    const casa::Cube<float>& visweight,
                    const casa::Vector<double>& freq,
                    const casa::Vector<double>& cellSize,
                    casa::Array<casa::Complex>& grid);
                    
    void genericForward(const casa::Vector<casa::RigidVector<casa::Double, 3> >& uvw,
                    casa::Cube<casa::Complex>& visibility,
                    casa::Cube<float>& visweight,
                    const casa::Vector<double>& freq,
                    const casa::Vector<double>& cellSize,
                    const casa::Array<casa::Complex>& grid);
					
};

}
}
#endif
