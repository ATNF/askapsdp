#include <fitting/DesignMatrix.h>
#include <fitting/NormalEquations.h>
#include <fitting/Params.h>

#include <casa/aips.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/Arrays/Vector.h>

#include <stdexcept>

namespace conrad {
namespace scimath {

NormalEquations::NormalEquations(const Params& ip) : 	itsParams(ip)
{
	itsApprox=NormalEquations::COMPLETE;
	vector<string> names=ip.freeNames();
	vector<string>::iterator iterRow;
	vector<string>::iterator iterCol;
	for (iterRow=names.begin();iterRow!=names.end();++iterRow) {
		itsDataVector[*iterRow]=casa::Vector<double>(0);
		for (iterCol=names.begin();iterCol!=names.end();++iterCol) {
			itsNormalMatrix[*iterRow][*iterCol]=casa::Matrix<double>(0,0);
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
		itsApprox=other.itsApprox;
		itsNormalMatrix=other.itsNormalMatrix;
		itsDataVector=other.itsDataVector;
	}
}

NormalEquations::NormalEquations(const DesignMatrix& dm, 
	const NormalEquations::Approximation approx)
{
	itsParams=dm.parameters();
	itsApprox=approx;
	vector<string> names=dm.names();
	vector<string>::iterator iterRow;
	vector<string>::iterator iterCol;
	const uint nDataSet=dm.residual().size();
	
	switch(approx) {
		case NormalEquations::COMPLETE:
			// This looks hairy but it's all just linear algebra!
			for (iterRow=names.begin();iterRow!=names.end();iterRow++) {
				bool first=true;
				for (uint iDataSet=0;iDataSet<nDataSet;iDataSet++) {
					// Need to special case for CASA product limitation
					if(dm.derivative(*iterRow)[iDataSet].ncolumn()==1) {		 
						const casa::Vector<casa::DComplex>& aV(dm.derivative(*iterRow)[iDataSet].column(0));
						if(first) {
							itsDataVector[*iterRow]=sum(real(conj(aV)*(dm.residual()[iDataSet])));
							first=false;
						}
						else {
							itsDataVector[*iterRow]+=sum(real(conj(aV)*(dm.residual()[iDataSet])));
						}
					}
					else {
						if(first) {
							itsDataVector[*iterRow]=real(product(adjoint(dm.derivative(*iterRow)[iDataSet]),dm.residual()[iDataSet]));
							first=false;
						}
						else {
							itsDataVector[*iterRow]+=real(product(adjoint(dm.derivative(*iterRow)[iDataSet]),dm.residual()[iDataSet]));
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
								const casa::Vector<casa::DComplex>& aRowV(dm.derivative(*iterRow)[iARow].column(0));
								const casa::Vector<casa::DComplex>& aColV(dm.derivative(*iterCol)[iACol].column(0));
								if(first) {
									itsNormalMatrix[*iterRow][*iterCol].resize(1,1);
									itsNormalMatrix[*iterRow][*iterCol].set(sum(real(conj(aRowV)*(aColV))));
									first=false;
								}
								else {
									itsNormalMatrix[*iterRow][*iterCol]+=sum(real(conj(aRowV)*(aColV)));
								}
							}
							else {
								if(first) {
									itsNormalMatrix[*iterRow][*iterCol]=real(product(adjoint(dm.derivative(*iterRow)[iARow]),dm.derivative(*iterCol)[iACol]));
									first=false;
								}
								else {
									itsNormalMatrix[*iterRow][*iterCol]+=real(product(adjoint(dm.derivative(*iterRow)[iARow]),dm.derivative(*iterCol)[iACol]));
								}
							}
						}
					}
				}
			}
			break;
//		case NormalEquations::DIAGONAL_COMPLETE:
//			throw(std::invalid_argument("Normal Equation approximation DIAGONAL_COMPLETE not yet implemented"));
//		case NormalEquations::DIAGONAL_SLICE:
//			throw(std::invalid_argument("Normal Equation approximation DIAGONAL_SLICE not yet implemented"));
//		case NormalEquations::DIAGONAL_DIAGONAL:
//			throw(std::invalid_argument("Normal Equation approximation DIAGONAL_DIAGONAL not yet implemented"));
//		default:
//			throw(std::invalid_argument("Unknown Normal Equation approximation"));
	}
}

NormalEquations::~NormalEquations()
{
	reset();
}

const Params& NormalEquations::parameters() const
{
	return itsParams;
}

Params& NormalEquations::parameters() 
{
	return itsParams;
}

void NormalEquations::setApproximation(const NormalEquations::Approximation approx)
{
	itsApprox=approx;
}

void NormalEquations::merge(const NormalEquations& other) 
{
	if(itsApprox!=other.itsApprox) {
		throw(std::invalid_argument("Normal equation approximations are different and cannot be merged"));
	} 
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
		for (iterRow=names.begin();iterRow!=names.end();iterRow++) {
			if(itsNormalMatrix[*iterCol][*iterRow].shape()!=other.itsNormalMatrix[*iterCol][*iterRow].shape()) {
				itsNormalMatrix[*iterCol][*iterRow]=other.itsNormalMatrix[*iterCol][*iterRow];
			}
			else {
				itsNormalMatrix[*iterCol][*iterRow]+=other.itsNormalMatrix[*iterCol][*iterRow];
			}
		}
	}
}

	/// Return normal equations
std::map<string, std::map<string, casa::Matrix<double> > > NormalEquations::normalMatrix() const
{
	return itsNormalMatrix;
}

	/// Return data vector
std::map<string, casa::Vector<double> > NormalEquations::dataVector() const
{
	return itsDataVector;
}


void NormalEquations::reset()
{
	map<string, casa::Vector<double> >::iterator iterRow;
	map<string, casa::Matrix<double> >::iterator iterCol;
	for (iterRow=itsDataVector.begin();iterRow!=itsDataVector.end();iterRow++) {
		itsDataVector[iterRow->first].resize();
		for (iterCol=itsNormalMatrix[iterRow->first].begin();iterCol!=itsNormalMatrix[iterRow->first].end();iterCol++) {
			itsNormalMatrix[iterRow->first][iterCol->first].resize();
		}
	}
	itsNormalMatrix.clear();
	itsDataVector.clear();
}
}
}
