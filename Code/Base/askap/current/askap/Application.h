/// @file Application.h
/// @brief Base class for applications
///
/// @copyright (c) 2012 CSIRO
/// Australia Telescope National Facility (ATNF)
/// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
/// PO Box 76, Epping NSW 1710, Australia
/// atnf-enquiries@csiro.au
///
/// This file is part of the ASKAP software distribution.
///
/// The ASKAP software distribution is free software: you can redistribute it
/// and/or modify it under the terms of the GNU General Public License as
/// published by the Free Software Foundation; either version 2 of the License,
/// or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
///
/// @author Ben Humphreys <ben.humphreys@csiro.au>

#ifndef ASKAP_APPLICATION_H
#define ASKAP_APPLICATION_H

// System includes
#include <string>

// ASKAPsoft includes
#include "boost/program_options.hpp"
#include "Common/ParameterSet.h"

namespace askap {

    /// @brief Generic application class.
    /// This generic application class encapsulates the standard startup/shutdown
    /// features of ASKAPsoft applications. It includes:
    /// - Standard approach to command line parameters
    /// - Usage/help message
    /// - Setup logging
    /// - Parsing of ParameterSet configuration file
    /// - Handling of exceptions so they don't propogate out of main()
    ///
    /// Here is an example usage:
    /// @code{.cpp}
    /// class MyApp : public askap::Application
    /// {
    ///    public:
    ///        virtual int run(int argc, char* argv[]) {
    ///
    ///            // Your code goes here
    ///
    ///            // You can get the parset like this
    ///            const LOFAR::ParameterSet parset = config();
    ///
    ///            return 0;
    ///        }
    /// };
    ///
    /// int main(int argc, char *argv[])
    /// {
    ///    MyApp app;
    ///    return app.main(argc, argv);
    /// }
    /// @endcode
    ///
    /// Command line parameters can be added before the application is run:
    /// @code{.cpp}
    /// int main(int argc, char *argv[])
    /// {
    ///    MyApp app;
    ///    app.addParameter("foo", "f", "Foo parameter");
    ///    app.addParameter("bar", "b", "Bar parameter has a default", "defaultbar");
    ///    return app.main(argc, argv);
    /// }
    /// @endcode
    ///
    /// These command line parameters can be inspected with the parameter() and
    /// parameterExists() function.
    class Application {
        public:
            /// Constructor
            Application();

            /// Destructor
            virtual ~Application();

            /// This function is implemented by sub-classes. i.e. The users of
            /// this class.
            virtual int run(int argc, char *argv[]) = 0;

            /// This must be called by the user, typically in the program main().
            /// It performs initialisation, calls the user implemented run() method,
            /// then performs any necessary finalisation.
            virtual int main(int argc, char *argv[]);

            /// Obtains the parameter set that was specified on the command line
            /// @return a copy of the parameter set
            LOFAR::ParameterSet config(void) const;

            /// Adds a command line parameter. The user can then call
            /// parameter() or parameterExists() to get the value of the
            /// parameter or determine if the user specified it on the
            /// command line.
            ///
            /// @param[in] keyLong  the long form of the parameter name. Note:
            ///                     This must be longer than one character
            /// @param[in] keyShort the short form of the parameter name. Note:
            ///                     This must be only one character
            /// @param[in] description  a description of the parameter to be used
            ///                         in the usage message.
            /// @param[in] hasValue     true if the parameter is expected to be specified
            ///                         along with a value (e.g. "-f filename"), otherwise
            ///                         false if specification of the parameter alone is
            ///                         sufficient (e.g. "-f").
            /// @throws AskapError  If the keyLong or keyShort are not of the
            ///                     correct length.
            void addParameter(const std::string& keyLong,
                              const std::string& keyShort,
                              const std::string& description,
                              bool hasValue = true);

            /// Adds a command line parameter. The user can then call
            /// parameter() or parameterExists() to get the value of the
            /// parameter or determine if the user specified it on the
            /// command line.
            ///
            /// @param[in] keyLong  the long form of the parameter name. Note:
            ///                     This must be longer than one character
            /// @param[in] keyShort the short form of the parameter name. Note:
            ///                     This must be only one character
            /// @param[in] description  a description of the parameter to be used
            ///                         in the usage message.
            /// @param[in] defaultValue a default value to be used if the parameter
            ///                         is not explicitly specified on the command
            ///                         line.
            /// @throws AskapError  If the keyLong or keyShort are not of the
            ///                     correct length.
            void addParameter(const std::string& keyLong,
                              const std::string& keyShort,
                              const std::string& description,
                              const std::string& defaultValue);

            /// Returns the value specified for a given parameter. For example,
            /// if the command line option "-f filename.txt" is specified, then
            /// if this is called with param "f" the string "filename.txt" will
            /// be returned.
            ///
            /// @param[in] param    the parameter (key) for which the value
            ///                     is desired.
            /// @return the value specified for a given parameter.
            /// @throws AskapError  if the param specified was not set on the
            ///                     command line.
            std::string parameter(const std::string& param) const;

            /// Returns true if the parameter was specified on the command line
            /// otherwise false.
            bool parameterExists(const std::string& param) const;


        private:

            /// Processes the command line arguments
            void processCmdLineArgs(int argc, char *argv[]);

            /// Initialises logging
            void initLogging(const std::string& argv0);

            /// Reads the "config" parameter are builds the Parset itsParset.
            void initConfig();

            /// Prints the usage message to stderr then calls exit()
            void usage();

            /// Builds a key to be passed to the boost program options from a
            /// long form and a short form parameter.
            std::string buildKey(const std::string& keyLong,
                                 const std::string& keyShort);

            /// Boost program options
            boost::program_options::options_description itsOptionsDesc;

            /// Boost variable map
            boost::program_options::variables_map itsVarMap;

            /// Parameter set specified on the command line
            LOFAR::ParameterSet itsParset;
    };

} // End namespace askap

#endif
