#include <measurementequation/MEDesignMatrix.h>
#include <measurementequation/MENormalEquations.h>
#include <measurementequation/MEParams.h>

#include <casa/aips.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/Arrays/Vector.h>

#include <stdexcept>

namespace conrad {
namespace synthesis {

MENormalEquations::MENormalEquations(const MEParams& ip) : 	itsParams(ip)
{
	itsApprox=MENormalEquations::COMPLETE;
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

MENormalEquations::MENormalEquations(const MENormalEquations& other) 
{
	operator=(other);
}

MENormalEquations& MENormalEquations::operator=(const MENormalEquations& other)
{
	if(this!=&other) {
		itsParams=other.itsParams;
		itsApprox=other.itsApprox;
		itsNormalMatrix=other.itsNormalMatrix;
		itsDataVector=other.itsDataVector;
	}
}

MENormalEquations::MENormalEquations(const MEDesignMatrix& dm, 
	const MENormalEquations::Approximation approx)
{
	itsParams=dm.parameters();
	itsApprox=approx;
	vector<string> names=dm.names();
	vector<string>::iterator iterRow;
	vector<string>::iterator iterCol;
	switch(approx) {
		case MENormalEquations::COMPLETE:
			for (iterRow=names.begin();iterRow!=names.end();++iterRow) {
				itsDataVector[*iterRow]=real(product(adjoint(dm.itsAMatrix[*iterRow]),dm.itsBVector));
			}
			for (iterRow=names.begin();iterRow!=names.end();++iterRow) {
				for (iterCol=names.begin();iterCol!=names.end();++iterCol) {
					itsNormalMatrix[*iterRow][*iterCol]=real(product(adjoint(dm.itsAMatrix[*iterRow]),dm.itsAMatrix[*iterCol]));
				}
			}
			break;
		case MENormalEquations::DIAGONAL_COMPLETE:
			throw(std::invalid_argument("Normal Equation approximation DIAGONAL_COMPLETE not yet implemented"));
		case MENormalEquations::DIAGONAL_SLICE:
			throw(std::invalid_argument("Normal Equation approximation DIAGONAL_SLICE not yet implemented"));
		case MENormalEquations::DIAGONAL_DIAGONAL:
			throw(std::invalid_argument("Normal Equation approximation DIAGONAL_DIAGONAL not yet implemented"));
		default:
			throw(std::invalid_argument("Unknown Normal Equation approximation"));
	}
}

MENormalEquations::~MENormalEquations()
{
	reset();
}

const MEParams& MENormalEquations::parameters() const
{
	return itsParams;
}

MEParams& MENormalEquations::parameters() 
{
	return itsParams;
}

void MENormalEquations::setApproximation(const MENormalEquations::Approximation approx)
{
	itsApprox=approx;
}

void MENormalEquations::merge(const MENormalEquations& other) 
{
}

	/// Return normal equations
std::map<string, std::map<string, casa::Matrix<double> > > MENormalEquations::normalMatrix() const
{
	return itsNormalMatrix;
}

	/// Return data vector
std::map<string, casa::Vector<double> > MENormalEquations::dataVector() const
{
	return itsDataVector;
}


void MENormalEquations::reset()
{
	map<string, casa::Vector<double> >::iterator iterRow;
	map<string, casa::Matrix<double> >::iterator iterCol;
	for (iterRow=itsDataVector.begin();iterRow!=itsDataVector.end();++iterRow) {
		itsDataVector[iterRow->first].resize();
		for (iterCol=itsNormalMatrix[iterRow->first].begin();iterCol!=itsNormalMatrix[iterRow->first].end();++iterCol) {
			itsNormalMatrix[iterRow->first][iterCol->first].resize();
		}
	}
	itsNormalMatrix.clear();
	itsDataVector.clear();
}
}
}