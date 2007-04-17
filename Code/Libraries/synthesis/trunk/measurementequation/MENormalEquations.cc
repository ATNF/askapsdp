#include <measurementequation/MEDesignMatrix.h>
#include <measurementequation/MENormalEquations.h>
#include <measurementequation/MEParams.h>

namespace conrad {
namespace synthesis {

MENormalEquations::MENormalEquations(const MEParams& ip) : 	itsParams(ip)
{
	itsApprox=MENormalEquations::COMPLETE;
	vector<string> names=ip.freeNames();
	vector<string>::iterator iterRow;
	vector<string>::iterator iterCol;
	for (iterRow=names.begin();iterRow!=names.end();++iterRow) {
		itsDataVector[*iterRow]=casa::Array<double>(casa::IPosition(0));
		for (iterCol=names.begin();iterCol!=names.end();++iterCol) {
			itsConstraintMatrix[*iterRow][*iterCol]=casa::Array<double>(casa::IPosition(0));
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
		itsConstraintMatrix=other.itsConstraintMatrix;
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
		case MENormalEquations::DIAGONAL_COMPLETE:
		case MENormalEquations::DIAGONAL_SLICE:
		case MENormalEquations::DIAGONAL_DIAGONAL:
			// TODO: To fill in correctly!	
			for (iterRow=names.begin();iterRow!=names.end();++iterRow) {
				itsDataVector[*iterRow]=casa::Array<double>(casa::IPosition(0));
				for (iterCol=names.begin();iterCol!=names.end();++iterCol) {
					itsConstraintMatrix[*iterRow][*iterCol]=casa::Array<double>(casa::IPosition(0));
			}
		}
		break;
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

void MENormalEquations::reset()
{
	map<string, casa::Array<double> >::iterator iterRow;
	map<string, casa::Array<double> >::iterator iterCol;
	for (iterRow=itsDataVector.begin();iterRow!=itsDataVector.end();++iterRow) {
		itsDataVector[iterRow->first].resize();
		for (iterCol=itsConstraintMatrix[iterRow->first].begin();iterCol!=itsConstraintMatrix[iterRow->first].end();++iterCol) {
			itsConstraintMatrix[iterRow->first][iterCol->first].resize();
		}
	}
	itsConstraintMatrix.clear();
	itsDataVector.clear();
}
}
}