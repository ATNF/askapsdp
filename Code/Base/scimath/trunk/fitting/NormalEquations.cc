#include <fitting/DesignMatrix.h>
#include <fitting/NormalEquations.h>
#include <fitting/Params.h>

#include <casa/aips.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/Arrays/Vector.h>

#include <stdexcept>

namespace conrad {
namespace scimath {

NormalEquations::NormalEquations(const Params& ip) : itsParams(ip)
{
	vector<string> names=ip.freeNames();
	vector<string>::iterator iterRow;
	vector<string>::iterator iterCol;
	for (iterRow=names.begin();iterRow!=names.end();++iterRow) {
        itsDataVector[*iterRow]=casa::Vector<double>(0);
        itsShape[*iterRow]=casa::IPosition(1,0);
        itsReference[*iterRow]=casa::IPosition(1,0);
		for (iterCol=names.begin();iterCol!=names.end();++iterCol) {
            itsNormalMatrix[*iterRow][*iterCol]=casa::Matrix<double>(0,0);
            itsNormalMatrixSlice[*iterRow][*iterCol]=casa::Matrix<double>(0,0);
            itsNormalMatrixDiagonal[*iterRow][*iterCol]=casa::Matrix<double>(0,0);
		}
	}
}

NormalEquations::NormalEquations(const NormalEquations& other) 
{
	operator=(other);
}

NormalEquations& NormalEquations::operator=(const NormalEquations& other)
{
	if(this!=&other) {
		itsParams=other.itsParams;
        itsNormalMatrix=other.itsNormalMatrix;
        itsNormalMatrixSlice=other.itsNormalMatrixSlice;
        itsNormalMatrixDiagonal=other.itsNormalMatrixDiagonal;
		itsDataVector=other.itsDataVector;
        itsShape=other.itsShape;
        itsReference=other.itsReference;
	}
}

NormalEquations::NormalEquations(const DesignMatrix& dm) 
{
    itsParams=dm.parameters();
    vector<string> names=dm.names();
    vector<string>::iterator iterRow;
    vector<string>::iterator iterCol;
    const uint nDataSet=dm.residual().size();
    
    // This looks hairy but it's all just linear algebra!
    for (iterRow=names.begin();iterRow!=names.end();iterRow++) {
        bool first=true;
        for (uint iDataSet=0;iDataSet<nDataSet;iDataSet++) {
            // Need to special case for CASA product limitation
            if(dm.derivative(*iterRow)[iDataSet].ncolumn()==1) {         
                const casa::Vector<casa::Double>& aV(dm.derivative(*iterRow)[iDataSet].column(0));
                if(first) {
                    itsDataVector[*iterRow]=sum(((aV)*(dm.residual()[iDataSet])));
                    first=false;
                }
                else {
                    itsDataVector[*iterRow]+=sum(((aV)*(dm.residual()[iDataSet])));
                }
            }
            else {
                if(first) {
                    itsDataVector[*iterRow]=(product(transpose(dm.derivative(*iterRow)[iDataSet]),dm.residual()[iDataSet]));
                    first=false;
                }
                else {
                    itsDataVector[*iterRow]+=(product(transpose(dm.derivative(*iterRow)[iDataSet]),dm.residual()[iDataSet]));
                }
            }
        }
    }
    // Outside loops are over parameter names
    for (iterCol=names.begin();iterCol!=names.end();iterCol++) {
        const uint nACol=dm.derivative(*iterCol).size();
        // Inside loops are over lists of derivatives
        for (uint iACol=0;(iACol<nACol);iACol++) {
            for (iterRow=names.begin();iterRow!=names.end();iterRow++) {
                bool first=true;
                const uint nARow=dm.derivative(*iterRow).size();
                for (uint iARow=0;(iARow<nARow);iARow++) {
                    if((dm.derivative(*iterRow)[iARow].ncolumn()==1)&&(dm.derivative(*iterCol)[iACol].ncolumn()==1)) {
                        const casa::Vector<casa::Double>& aRowV(dm.derivative(*iterRow)[iARow].column(0));
                        const casa::Vector<casa::Double>& aColV(dm.derivative(*iterCol)[iACol].column(0));
                        if(first) {
                            itsNormalMatrix[*iterRow][*iterCol].resize(1,1);
                            itsNormalMatrix[*iterRow][*iterCol].set(sum(((aRowV)*(aColV))));
                            first=false;
                        }
                        else {
                            itsNormalMatrix[*iterRow][*iterCol]+=sum(((aRowV)*(aColV)));
                        }
                    }
                    else {
                        if(first) {
                            itsNormalMatrix[*iterRow][*iterCol]=(product(transpose(dm.derivative(*iterRow)[iARow]),dm.derivative(*iterCol)[iACol]));
                            first=false;
                        }
                        else {
                            itsNormalMatrix[*iterRow][*iterCol]+=(product(transpose(dm.derivative(*iterRow)[iARow]),dm.derivative(*iterCol)[iACol]));
                        }
                    }
                }
            }
        }
    }
}

void NormalEquations::add(const DesignMatrix& dm)
{
    itsParams.merge(dm.parameters());
    vector<string> names=dm.names();
    vector<string>::iterator iterRow;
    vector<string>::iterator iterCol;
    const uint nDataSet=dm.residual().size();
    
    // This looks hairy but it's all just linear algebra!
    for (iterRow=names.begin();iterRow!=names.end();iterRow++) {
        bool first=(itsDataVector[*iterRow].size()==0);
        for (uint iDataSet=0;iDataSet<nDataSet;iDataSet++) {
            // Need to special case for CASA product limitation
            if(dm.derivative(*iterRow)[iDataSet].ncolumn()==1) {         
                const casa::Vector<casa::Double>& aV(dm.derivative(*iterRow)[iDataSet].column(0));
                if(first) {
                    itsDataVector[*iterRow]=sum(((aV)*(dm.residual()[iDataSet])));
                    first=false;
                }
                else {
                    itsDataVector[*iterRow]+=sum(((aV)*(dm.residual()[iDataSet])));
                }
            }
            else {
                if(first) {
                    itsDataVector[*iterRow]=(product(transpose(dm.derivative(*iterRow)[iDataSet]),dm.residual()[iDataSet]));
                    first=false;
                }
                else {
                    itsDataVector[*iterRow]+=(product(transpose(dm.derivative(*iterRow)[iDataSet]),dm.residual()[iDataSet]));
                }
            }
        }
    }
    // Outside loops are over parameter names
    for (iterCol=names.begin();iterCol!=names.end();iterCol++) {
        const uint nACol=dm.derivative(*iterCol).size();
        // Inside loops are over lists of derivatives
        for (uint iACol=0;(iACol<nACol);iACol++) {
            for (iterRow=names.begin();iterRow!=names.end();iterRow++) {
                bool first=(itsNormalMatrix[*iterRow][*iterCol].nrow()==0);
                const uint nARow=dm.derivative(*iterRow).size();
                for (uint iARow=0;(iARow<nARow);iARow++) {
                    if((dm.derivative(*iterRow)[iARow].ncolumn()==1)&&(dm.derivative(*iterCol)[iACol].ncolumn()==1)) {
                        const casa::Vector<casa::Double>& aRowV(dm.derivative(*iterRow)[iARow].column(0));
                        const casa::Vector<casa::Double>& aColV(dm.derivative(*iterCol)[iACol].column(0));
                        if(first) {
                            itsNormalMatrix[*iterRow][*iterCol].resize(1,1);
                            itsNormalMatrix[*iterRow][*iterCol].set(sum(((aRowV)*(aColV))));
                            first=false;
                        }
                        else {
                            itsNormalMatrix[*iterRow][*iterCol]+=sum(((aRowV)*(aColV)));
                        }
                    }
                    else {
                        if(first) {
                            itsNormalMatrix[*iterRow][*iterCol]=(product(transpose(dm.derivative(*iterRow)[iARow]),dm.derivative(*iterCol)[iACol]));
                            first=false;
                        }
                        else {
                            itsNormalMatrix[*iterRow][*iterCol]+=(product(transpose(dm.derivative(*iterRow)[iARow]),dm.derivative(*iterCol)[iACol]));
                        }
                    }
                }
            }
        }
    }
}

NormalEquations::~NormalEquations()
{
	reset();
}
void NormalEquations::merge(const NormalEquations& other) 
{
    itsParams.merge(other.itsParams);
    vector<string> names=itsParams.names();
    vector<string>::iterator iterRow;
    vector<string>::iterator iterCol;
    
    for (iterCol=names.begin();iterCol!=names.end();iterCol++) {
        if(itsDataVector[*iterCol].size()!=other.itsDataVector[*iterCol].size()) {
            itsDataVector[*iterCol]=other.itsDataVector[*iterCol];
        }
        else {
            itsDataVector[*iterCol]+=other.itsDataVector[*iterCol];
        }
        itsShape[*iterCol]=other.itsShape[*iterCol];
        for (iterRow=names.begin();iterRow!=names.end();iterRow++) {
            if(itsNormalMatrix[*iterCol][*iterRow].shape()!=other.itsNormalMatrix[*iterCol][*iterRow].shape()) {
                itsNormalMatrix[*iterCol][*iterRow]=other.itsNormalMatrix[*iterCol][*iterRow];
            }
            else {
                itsNormalMatrix[*iterCol][*iterRow]+=other.itsNormalMatrix[*iterCol][*iterRow];
            }
            if(itsNormalMatrixSlice[*iterCol][*iterRow].shape()!=other.itsNormalMatrixSlice[*iterCol][*iterRow].shape()) {
                itsNormalMatrixSlice[*iterCol][*iterRow]=other.itsNormalMatrixSlice[*iterCol][*iterRow];
            }
            else {
                itsNormalMatrixSlice[*iterCol][*iterRow]+=other.itsNormalMatrixSlice[*iterCol][*iterRow];
            }
            if(itsNormalMatrixDiagonal[*iterCol][*iterRow].shape()!=other.itsNormalMatrixDiagonal[*iterCol][*iterRow].shape()) {
                itsNormalMatrixDiagonal[*iterCol][*iterRow]=other.itsNormalMatrixDiagonal[*iterCol][*iterRow];
            }
            else {
                itsNormalMatrixDiagonal[*iterCol][*iterRow]+=other.itsNormalMatrixDiagonal[*iterCol][*iterRow];
            }
        }
    }
}


	/// Return normal equations
std::map<string, std::map<string, casa::Matrix<double> > >& NormalEquations::normalMatrix() const
{
	return itsNormalMatrix;
}

std::map<string, std::map<string, casa::Vector<double> > >& NormalEquations::normalMatrixDiagonal() const
{
    return itsNormalMatrixDiagonal;
}

std::map<string, std::map<string, casa::Vector<double> > >& NormalEquations::normalMatrixSlice() const
{
    return itsNormalMatrixSlice;
}

    /// Return data vector
std::map<string, casa::Vector<double> >& NormalEquations::dataVector() const
{
    return itsDataVector;
}

    /// Return shape
std::map<string, casa::IPosition >& NormalEquations::shape() const
{
    return itsShape;
}


    /// Return reference
std::map<string, casa::IPosition >& NormalEquations::reference() const
{
    return itsReference;
}


void NormalEquations::reset()
{
	map<string, casa::Vector<double> >::iterator iterRow;
	map<string, casa::Matrix<double> >::iterator iterCol;
	for (iterRow=itsDataVector.begin();iterRow!=itsDataVector.end();iterRow++) {
		itsDataVector[iterRow->first].resize();
		for (iterCol=itsNormalMatrix[iterRow->first].begin();iterCol!=itsNormalMatrix[iterRow->first].end();iterCol++) {
            itsNormalMatrix[iterRow->first][iterCol->first].resize();
            itsNormalMatrixSlice[iterRow->first][iterCol->first].resize();
            itsNormalMatrixDiagonal[iterRow->first][iterCol->first].resize();
		}
	}
    itsNormalMatrix.clear();
    itsNormalMatrixSlice.clear();
    itsNormalMatrixDiagonal.clear();
	itsDataVector.clear();
    itsShape.clear();
    itsReference.clear();
}

void NormalEquations::add(const string& name, const casa::Matrix<double>& normalmatrix, 
        const casa::Vector<double>& datavector, const casa::IPosition& shape) 
{
    
    if(datavector.size()!=itsDataVector[name].size()) {
        itsDataVector[name]=datavector;
    }
    else {
        itsDataVector[name]+=datavector;
    }
    if(normalmatrix.shape()!=itsNormalMatrix[name][name].shape()) {
        itsNormalMatrix[name][name]=normalmatrix;
    }
    else {
        itsNormalMatrix[name][name]+=normalmatrix;
    }
    itsShape[name]=shape;
}

void NormalEquations::add(const string& name, const casa::Matrix<double>& normalmatrix, 
        const casa::Vector<double>& datavector) 
{
    casa::IPosition shape(1, datavector.nelements());
    add(name, normalmatrix, datavector, shape);
}

void NormalEquations::addSlice(const string& name, const casa::Vector<double>& normalmatrix, 
        const casa::Vector<double>& datavector, const casa::IPosition& shape,
        const casa::IPosition& reference) 
{
    
    if(datavector.size()!=itsDataVector[name].size()) {
        itsDataVector[name]=datavector;
    }
    else {
        itsDataVector[name]+=datavector;
    }
    if(normalmatrix.shape()!=itsNormalMatrix[name][name].shape()) {
        itsNormalMatrixSlice[name][name]=normalmatrix;
    }
    else {
        itsNormalMatrixSlice[name][name]+=normalmatrix;
    }
    itsShape[name]=shape;
    itsReference[name]=reference;
}

void NormalEquations::addDiagonal(const string& name, const casa::Vector<double>& normalmatrix, 
        const casa::Vector<double>& datavector, const casa::IPosition& shape) 
{
    
    if(datavector.size()!=itsDataVector[name].size()) {
        itsDataVector[name]=datavector;
    }
    else {
        itsDataVector[name]+=datavector;
    }
    if(normalmatrix.shape()!=itsNormalMatrix[name][name].shape()) {
        itsNormalMatrixDiagonal[name][name]=normalmatrix;
    }
    else {
        itsNormalMatrixDiagonal[name][name]+=normalmatrix;
    }
    itsShape[name]=shape;
}

void NormalEquations::addDiagonal(const string& name, const casa::Vector<double>& normalmatrix, 
        const casa::Vector<double>& datavector) 
{
    casa::IPosition shape(1, datavector.nelements());
    addDiagonal(name, normalmatrix, datavector, shape);
}


const Params& NormalEquations::parameters() const
{
    return itsParams;
}
    
Params& NormalEquations::parameters()
{
    return itsParams;
}


}
}
