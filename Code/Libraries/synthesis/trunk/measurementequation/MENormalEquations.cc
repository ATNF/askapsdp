#include <measurementequation/MEDesignMatrix.h>
#include <measurementequation/MENormalEquations.h>
#include <measurementequation/MEParams.h>

namespace conrad {
namespace synthesis {

MENormalEquations::MENormalEquations(const MEParams& ip)
{
	itsType=MENormalEquations::COMPLETE;
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
		itsType=other.itsType;
		itsConstraintMatrix=other.itsConstraintMatrix;
		itsDataVector=other.itsDataVector;
	}
}

MENormalEquations::MENormalEquations(const MEDesignMatrix& dm, const MENormalEquations::Type type)
{
	itsType=type;
	vector<string> names=dm.names();
	vector<string>::iterator iterRow;
	vector<string>::iterator iterCol;
	switch(type) {
		case MENormalEquations::COMPLETE:
		case MENormalEquations::DIAGONAL:
		case MENormalEquations::DIAGONAL_PSF:
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

void MENormalEquations::setType(const MENormalEquations::Type type)
{
	itsType=type;
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