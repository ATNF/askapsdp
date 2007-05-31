#include <fitting/ParamsCASATable.h>
#include <fitting/Params.h>

#include <string>

#include <casa/aips.h>
#include <tables/Tables/TableLocker.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/ExprNode.h>
#include <tables/Tables/ExprNodeSet.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/TableRecord.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayUtil.h>
#include <casa/Arrays/Slice.h>
#include <casa/Utilities/Regex.h>
#include <casa/Utilities/GenSort.h>
#include <casa/BasicMath/Math.h>

using namespace casa;

namespace conrad
{
namespace scimath
{
    
    const String colName("NAME");
    const String colValues("VALUES");
    const String colAxes("DOMAIN");
    const String colStart("START");
    const String colEnd("END");
    const String colFree("FREE");

ParamsCASATable::ParamsCASATable(const std::string& tablename, bool exists)
{
    if(exists) {
        openTable(tablename);
    }
    else {
        createTable(tablename);
    }    
}

void ParamsCASATable::createTable(const std::string& tablename)
{
    itsTableName=tablename;
    itsTableDesc.addColumn (ScalarColumnDesc<String>(colName));
    itsTableDesc.addColumn (ArrayColumnDesc<String>(colAxes));
    itsTableDesc.addColumn (ArrayColumnDesc<double>(colStart,1));
    itsTableDesc.addColumn (ArrayColumnDesc<double>(colEnd,1));
    itsTableDesc.addColumn (ArrayColumnDesc<double>(colValues));
    itsTableDesc.addColumn (ScalarColumnDesc<bool>(colFree));
  
    SetupNewTable newtab(itsTableName, itsTableDesc, Table::New);
    itsTable=Table(newtab,0,False,Table::LittleEndian);
}

bool ParamsCASATable::openTable(const std::string& tablename)
{
    if(!(Table::isReadable(tablename))) {
        return false;
    }
    
    itsTableName=tablename;
    itsTable=Table(itsTableName);
    return true;
}

ParamsCASATable::~ParamsCASATable()
{
    itsTable.flush();
}

bool ParamsCASATable::getParameters(Params& ip) const
{
    if(!(Table::isReadable(itsTable.tableName()))){
        return false;
    }
 
    ROScalarColumn<String> nameCol (itsTable, colName);
    ROArrayColumn<String> axesCol (itsTable, colAxes);
    ROArrayColumn<double> valCol (itsTable, colValues);
    ROArrayColumn<double> startCol (itsTable, colStart);
    ROArrayColumn<double> endCol (itsTable, colEnd);
    ROScalarColumn<bool> freeCol (itsTable, colFree);
    
    int rownr=itsTable.nrow();

    for (int rownr=0;rownr<itsTable.nrow();rownr++) {
        casa::String name; 
        nameCol.get(rownr, name);
        casa::Array<double> value;        
        valCol.get(rownr, value); 

        casa::Vector<String> axesNames;
        axesCol.get(rownr, axesNames);
        casa::Vector<double> start;
        startCol.get(rownr, start);
        casa::Vector<double> end;
        endCol.get(rownr, end);
        bool free;
        freeCol.get(rownr, free);
        Axes ax;
        for (int i=0;i<axesNames.nelements();i++) {
            ax.add(axesNames(i), start(i), end(i));
        }
        ip.add(name, value, ax);       
    }

    
	return false;
};

casa::Vector<casa::String> ParamsCASATable::toCASAString(const std::vector<std::string>& s) {
    casa::Vector<casa::String> result(s.size());
    for (int i=0;i<s.size();i++) {
        result(i)=s[i];
    }
    return result;
}

std::vector<std::string> ParamsCASATable::toStdString(const casa::Vector<casa::String>& s) {
    std::vector<std::string> result(s.nelements());
    for (int i=0;i<s.nelements();i++) {
        result[i]=s(i);
    }
    return result;
}

bool ParamsCASATable::setParameters(const Params& ip) {
    itsTable.reopenRW();
    TableLocker locker(itsTable, FileLocker::Write);
    ScalarColumn<String> nameCol (itsTable, colName);
    ArrayColumn<String> axesCol (itsTable, colAxes);
    ArrayColumn<double> valCol (itsTable, colValues);
    ArrayColumn<double> startCol (itsTable, colStart);
    ArrayColumn<double> endCol (itsTable, colEnd);
    ScalarColumn<bool> freeCol (itsTable, colFree);
    
    std::vector<string> names(ip.names());
    int rownr=itsTable.nrow();
    
    for (std::vector<string>::iterator it=names.begin();it!=names.end();it++) {
        itsTable.addRow();
        nameCol.put(rownr, *it);
        valCol.put(rownr, ip.value(*it)); 

        Axes ax(ip.axes(*it));
        axesCol.put(rownr, toCASAString(ax.names())); 
        startCol.put(rownr, casa::Vector<double>(ax.start())); 
        endCol.put(rownr, casa::Vector<double>(ax.end()));
        
        freeCol.put(rownr, ip.isFree(*it));
        
        rownr++; 
    }

}

}
}
