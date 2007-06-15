/// @file
///
/// ParamsCASATable: Concrete class for storing and retrieving Params by
/// domain specification to and from a CASA table.
///
/// (c) 2007 CONRAD, All Rights Reserved.
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

    /// @brief Store params into a CASA table
    class ParamsCASATable : public ParamsTable
    {
      public:
/// Construct from an existing table or create a new table
/// @param tablename Name of table to be opened or created
/// @param exists Does the file exist already?
        ParamsCASATable(const std::string& tablename, bool exists=false);

        virtual ~ParamsCASATable();

/// Get all the parameters
/// @param ip Template of parameters - must match
        virtual void getParameters(Params& ip) const;

/// Get the parameters for a specified domain
/// @param ip Template of parameters - must match
/// @param dom domain of parameters
        virtual void getParameters(Params& ip, const Domain& dom) const;

/// Set all the parameters
/// @param ip Parameters to set
        virtual void setParameters (const Params& ip);

/// Set the parameters for a given domain
/// @param ip Parameters to set
/// @param dom domain of parameters
        virtual void setParameters (const Params& ip, const Domain& dom);

      private:
      /// Helper function to convert a std::vector to a casa::Vector of strings
      /// @param s std::vector of std::strings to be converted
        casa::Vector<casa::String> toCASAString(const std::vector<std::string>& s);
      /// Helper function to convert a std::vector from a casa::Vector of strings
      /// @param s casa::Vector of std::Strings to be converted
        std::vector<std::string> toStdString(const casa::Vector<casa::String>& s);

/// Helper function to create a table with the specified name and required description
/// @param tablename Name of new table
        void createTable(const std::string& tablename);
        
/// Helper function to open an existing table
/// @param tablename Name of existing table
        void openTable(const std::string& tablename);

/// Table name
        std::string itsTableName;
        
        /// Table
        casa::Table itsTable;
        
        /// Table description
        casa::TableDesc itsTableDesc;

    };

  }
}
#endif                                            /*PARAMSTABLE_H_*/
