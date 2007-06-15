/// @file
///
/// TableVisGridder: Table-based visibility gridder. 
///
/// This supports gridders with a table lookup.
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
	
	/// Standard two dimensional gridding using a convolution function
    /// in a table
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

    // The convolution function is stored as a cube so that we can use the third axes
    // for data dependent variations e.g. w projection. The function cOffset can be
    // used to generate this offset.
    casa::Cube<float> itsC;
    virtual int cOffset(int, int)=0;

    // Support, oversampling, size, and center of the convolution function.
    int itsSupport;
    int itsOverSample;
    int itsCSize;
    int itsCCenter;
    
    /// If !itsInM these functions assume that the convolution function
    /// is specified in wavelengths. This is not always the case e.g. for antenna illumination
    /// pattern gridding. In that case, set itsInM to true.
    bool itsInM;

    // Initialize the convolution function - this is the key function to override
    virtual void initConvolutionFunction(IDataSharedIter& idi, const casa::Vector<double>& cellSize,
        const casa::IPosition& shape);
        
    // Find the cellsize from the image shape and axis definitions
    void findCellsize(casa::Vector<double>& cellSize, const casa::IPosition& imageShape, 
        const scimath::Axes& axes);
    
    /// Functions to do the real work. We may need to override these for derived classes so we
    /// make them virtual and protected. 

    /// Visibility to image for a cube (MFS)
    virtual void genericReverse(const casa::Vector<casa::RigidVector<casa::Double, 3> >& uvw,
                    const casa::Cube<casa::Complex>& visibility,
                    const casa::Cube<float>& visweight,
                    const casa::Vector<double>& freq,
                    const casa::Vector<double>& cellSize,
                    casa::Cube<casa::Complex>& grid,
                    casa::Vector<float>& sumwt);
                    
    /// Visibility weights to image for a cube (MFS)
    virtual void genericReverseWeights(const casa::Vector<casa::RigidVector<casa::Double, 3> >& uvw,
                    const casa::Cube<float>& visweight,
                    const casa::Vector<double>& freq,
                    const casa::Vector<double>& cellSize,
                    casa::Cube<casa::Complex>& grid);
                    
    /// Image to visibility for a cube (MFS))
    virtual void genericForward(const casa::Vector<casa::RigidVector<casa::Double, 3> >& uvw,
                    casa::Cube<casa::Complex>& visibility,
                    casa::Cube<float>& visweight,
                    const casa::Vector<double>& freq,
                    const casa::Vector<double>& cellSize,
                    const casa::Cube<casa::Complex>& grid);
                    
    /// Visibility to image for an array (spectral line)
    virtual void genericReverse(const casa::Vector<casa::RigidVector<casa::Double, 3> >& uvw,
                    const casa::Cube<casa::Complex>& visibility,
                    const casa::Cube<float>& visweight,
                    const casa::Vector<double>& freq,
                    const casa::Vector<double>& cellSize,
                    casa::Array<casa::Complex>& grid,
                    casa::Matrix<float>& sumwt);
                    
    /// Visibility weights to image for an array (spectral line)
    void genericReverseWeights(const casa::Vector<casa::RigidVector<casa::Double, 3> >& uvw,
                    const casa::Cube<float>& visweight,
                    const casa::Vector<double>& freq,
                    const casa::Vector<double>& cellSize,
                    casa::Array<casa::Complex>& grid);
                    
    /// Image to visibility for an array (spectral line)
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
