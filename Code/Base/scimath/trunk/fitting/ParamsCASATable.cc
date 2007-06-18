#include <fitting/ParamsCASATable.h>
#include <fitting/Params.h>
#include <fitting/Axes.h>

#include <string>
#include <iostream>

#include <casa/aips.h>
#include <tables/Tables/TableLocker.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/ColumnDesc.h>
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

#include <conrad/ConradError.h>

using namespace conrad;

using namespace casa;

namespace conrad
{
  namespace scimath
  {

    const String colName("NAME");
    const String colValues("VALUES");
    const String colAxes("AXES");
    const String colStart("AXESSTART");
    const String colEnd("AXESEND");
    const String colDomain("DOMAIN");
    const String colDomainStart("DOMAINSTART");
    const String colDomainEnd("DOMAINEND");
    const String colFree("FREE");

    ParamsCASATable::ParamsCASATable(const std::string& tablename, bool exists)
    {
      if(exists)
      {
        openTable(tablename);
      }
      else
      {
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
      itsTableDesc.addColumn (ArrayColumnDesc<String>(colDomain));
      itsTableDesc.addColumn (ArrayColumnDesc<double>(colDomainStart,1));
      itsTableDesc.addColumn (ArrayColumnDesc<double>(colDomainEnd,1));
      itsTableDesc.addColumn (ArrayColumnDesc<double>(colValues,-1));
      itsTableDesc.addColumn (ScalarColumnDesc<bool>(colFree));

      SetupNewTable newtab(itsTableName, itsTableDesc, Table::New);
      itsTable=Table(newtab,0,False,Table::LittleEndian);
      std::cout << "Successfully created new parameters table " << itsTableName << std::endl;
    }

    void ParamsCASATable::openTable(const std::string& tablename)
    {
      CONRADCHECK(Table::isReadable(tablename), "Parameters table " << tablename << " is not readable");

      itsTableName=tablename;
      itsTable=Table(itsTableName);
      std::cout << "Successfully opened existing parameters table " << itsTableName << std::endl;
    }

    ParamsCASATable::~ParamsCASATable()
    {
      itsTable.flush(true);
    }

    void ParamsCASATable::getParameters(Params& ip) const
    {
      Domain null;
      null.add("NULL", 0.0, 0.0);
      getParameters(ip, null);
    }

    void ParamsCASATable::getParameters(Params& ip, const Domain& domain) const
    {
      CONRADCHECK(Table::isReadable(itsTable.tableName()), "Parameters table " << itsTable.tableName() << " is not readable");

      ROScalarColumn<String> nameCol (itsTable, colName);
      ROArrayColumn<double> valCol (itsTable, colValues);
      ROArrayColumn<String> axesCol (itsTable, colAxes);
      ROArrayColumn<double> startCol (itsTable, colStart);
      ROArrayColumn<double> endCol (itsTable, colEnd);
      ROArrayColumn<String> domainCol (itsTable, colDomain);
      ROArrayColumn<double> domainStartCol (itsTable, colDomainStart);
      ROArrayColumn<double> domainEndCol (itsTable, colDomainEnd);
      ROScalarColumn<bool> freeCol (itsTable, colFree);

      CONRADCHECK(itsTable.nrow()>0, "Parameters table " << itsTable.tableName() << " is empty");

      for (int rownr=0;rownr<itsTable.nrow();rownr++)
      {
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
        Axes ax;
        for (int i=0;i<axesNames.nelements();i++)
        {
          ax.add(axesNames(i), start(i), end(i));
        }
        ip.add(name, value, ax);
        bool free;
        freeCol.get(rownr, free);
        if(free) 
        {
          ip.free(name);
        }
        else 
        {
          ip.fix(name);
        }
      }
    };

    casa::Vector<casa::String> ParamsCASATable::toCASAString(const std::vector<std::string>& s)
    {
      casa::Vector<casa::String> result(s.size());
      for (int i=0;i<s.size();i++)
      {
        result(i)=s[i];
      }
      return result;
    }

    std::vector<std::string> ParamsCASATable::toStdString(const casa::Vector<casa::String>& s)
    {
      std::vector<std::string> result(s.nelements());
      for (int i=0;i<s.nelements();i++)
      {
        result[i]=s(i);
      }
      return result;
    }

    void ParamsCASATable::setParameters(const Params& ip)
    {
      Domain null;
      null.add("NULL", 0.0, 0.0);
      setParameters(ip, null);
    }

    void ParamsCASATable::setParameters(const Params& ip, const Domain& domain)
    {
      itsTable.reopenRW();
      TableLocker locker(itsTable, FileLocker::Write);
      ScalarColumn<String> nameCol (itsTable, colName);
      ArrayColumn<double> valCol (itsTable, colValues);
      ArrayColumn<String> axesCol (itsTable, colAxes);
      ArrayColumn<double> startCol (itsTable, colStart);
      ArrayColumn<double> endCol (itsTable, colEnd);
      ArrayColumn<String> domainCol (itsTable, colDomain);
      ArrayColumn<double> domainStartCol (itsTable, colDomainStart);
      ArrayColumn<double> domainEndCol (itsTable, colDomainEnd);
      ScalarColumn<bool> freeCol (itsTable, colFree);

      std::vector<string> names(ip.names());
      int rownr=itsTable.nrow();

      for (std::vector<string>::iterator it=names.begin();it!=names.end();it++)
      {
        itsTable.addRow();
        nameCol.put(rownr, *it);
        valCol.put(rownr, ip.value(*it));

        Axes ax(ip.axes(*it));
        axesCol.put(rownr, toCASAString(ax.names()));
        startCol.put(rownr, casa::Vector<double>(ax.start()));
        endCol.put(rownr, casa::Vector<double>(ax.end()));

        domainCol.put(rownr, toCASAString(domain.names()));
        domainStartCol.put(rownr, casa::Vector<double>(domain.start()));
        domainEndCol.put(rownr, casa::Vector<double>(domain.end()));

        freeCol.put(rownr, ip.isFree(*it));

        rownr++;
      }

    }

  }
}
