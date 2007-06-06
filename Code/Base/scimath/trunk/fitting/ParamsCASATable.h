/// @file
///
/// ParamsTable: Base class for storing and retrieving Params by 
/// domain specification.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
#ifndef SCIMATHPARAMSCASATABLE_H_
#define SCIMATHPARAMSCASATABLE_H_

#include <casa/aips.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableDesc.h>

#include <fitting/ParamsTable.h>

#include <string>
#include <vector>

namespace conrad
{
namespace scimath
{

class ParamsCASATable : public ParamsTable
{
public:
    /// Construct from an existing table or create a new table
	ParamsCASATable(const std::string& tablename, bool exists=false);
	
	virtual ~ParamsCASATable();
	
	/// Get the parameters for a specified domain
	/// @param ip Template of parameters - must match
    /// @param dom domain of parameters
    virtual bool getParameters(Params& ip) const;
    virtual bool getParameters(Params& ip, const Domain& dom) const;

	/// Set the parameters for a given domain
	/// @param ip Parameters to set
    /// @param dom domain of parameters
    virtual bool setParameters (const Params& ip);
    virtual bool setParameters (const Params& ip, const Domain& dom);
    
private:
    casa::Vector<casa::String> toCASAString(const std::vector<std::string>& s);
    std::vector<std::string> toStdString(const casa::Vector<casa::String>& s);

    void createTable(const std::string& tablename);
    bool openTable(const std::string& tablename);
    
    std::string itsTableName;
    casa::Table itsTable;
    casa::TableDesc itsTableDesc;

};

}
}

#endif /*PARAMSTABLE_H_*/
