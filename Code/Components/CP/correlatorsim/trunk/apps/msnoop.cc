/// @file msnoop.cc
///
/// @description
/// This program snoops the metadata stream being published by the telescope
/// operating system (TOS), decodes the output and writes it to stdout.
///
/// @copyright (c) 2010 CSIRO
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

// Include package level header file
#include "askap_correlatorsim.h"

// System includes
#include <string>
#include <iostream>
#include <iomanip>
#include <unistd.h>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "askap/AskapLogging.h"
#include "CommandLineParser.h"
#include "Common/ParameterSet.h"
#include "tosmetadata/MetadataReceiver.h"

// ICE interface includes
#include "CommonTypes.h"
#include "TypedValues.h"

using namespace askap::cp;
using namespace askap::interfaces;
using namespace askap::interfaces::datapublisher;

// Globals
static bool verbose = false;

ASKAP_LOGGER(logger, ".msnoop");

class MetadataSubscriber : virtual public MetadataReceiver {
    public:
        MetadataSubscriber(const std::string& locatorHost,
                const std::string& locatorPort,
                const std::string& topicManager,
                const std::string& topic,
                const std::string& adapterName) :
            MetadataReceiver(locatorHost, locatorPort, topicManager, topic, adapterName)
    {
    }

        virtual void receive(const TimeTaggedTypedValueMap& msg)
        {
            // Print out the header
            std::cout << "Header:" << std::endl;
            std::cout << "\ttimestamp: " << msg.timestamp << std::endl;

            // Print out the data section
            const TypedValueMap& data = msg.data;
            std::cout << "Data Payload:" << std::endl;
            TypedValueMap::const_iterator it = data.begin();
            while (it != data.end()) {
                decodeValue(it->first, it->second);
                ++it;
            }
        }

        void decodeValue(const std::string& key, const TypedValuePtr tv) {
            std::cout << "\t" << key << " : ";

            switch (tv->type) {
                // Scalar
                case TypeNull :
                    printNull();
                    newline();
                    break;

                case TypeInt :
                    print(TypedValueIntPtr::dynamicCast(tv)->value);
                    newline();
                    break;

                case TypeLong :
                    print(TypedValueLongPtr::dynamicCast(tv)->value);
                    newline();
                    break;

                case TypeString :
                    print(TypedValueStringPtr::dynamicCast(tv)->value);
                    newline();
                    break;

                case TypeBool :
                    print(TypedValueBoolPtr::dynamicCast(tv)->value);
                    newline();
                    break;

                case TypeFloat :
                    print(TypedValueFloatPtr::dynamicCast(tv)->value);
                    newline();
                    break;

                case TypeDouble :
                    print(TypedValueDoublePtr::dynamicCast(tv)->value);
                    newline();
                    break;

                case TypeFloatComplex :
                    print(TypedValueFloatComplexPtr::dynamicCast(tv)->value);
                    newline();
                    break;

                case TypeDoubleComplex :
                    print(TypedValueDoubleComplexPtr::dynamicCast(tv)->value);
                    newline();
                    break;

                case TypeDirection :
                    print(TypedValueDirectionPtr::dynamicCast(tv)->value);
                    newline();
                    break;

                    // Sequences
                case TypeIntSeq :
                    print(TypedValueIntSeqPtr::dynamicCast(tv)->value);
                    newline();
                    break;

                case TypeLongSeq :
                    print(TypedValueLongSeqPtr::dynamicCast(tv)->value);
                    newline();
                    break;

                case TypeStringSeq :
                    print(TypedValueStringSeqPtr::dynamicCast(tv)->value);
                    newline();
                    break;

                case TypeBoolSeq :
                    print(TypedValueBoolSeqPtr::dynamicCast(tv)->value);
                    newline();
                    break;

                case TypeFloatSeq :
                    print(TypedValueFloatSeqPtr::dynamicCast(tv)->value);
                    newline();
                    break;

                case TypeDoubleSeq :
                    print(TypedValueDoubleSeqPtr::dynamicCast(tv)->value);
                    newline();
                    break;

                case TypeFloatComplexSeq :
                    print(TypedValueFloatComplexSeqPtr::dynamicCast(tv)->value);
                    newline();
                    break;

                case TypeDoubleComplexSeq :
                    print(TypedValueDoubleComplexSeqPtr::dynamicCast(tv)->value);
                    newline();
                    break;

                case TypeDirectionSeq :
                    print(TypedValueDirectionSeqPtr::dynamicCast(tv)->value);
                    newline();
                    break;

                default:
                    std::cout << "< Unknown type >" << std::endl;
            }
        }

        void newline(void) {
            std::cout << std::endl;
        }

        template <class T>
            void print(const T v) {
                std::cout << v;
            }

        void printNull(void) {
            std::cout << "<null>";
        }

        void print(const bool v) {
            if (v) {
                std::cout << "True";
            } else {
                std::cout << "False";
            }
        }

        void print(const float v) {
            std::cout << std::setprecision(8) << v;
        }

        void print(const double v) {
            std::cout << std::setprecision(16) << v;
        }

        void print(const FloatComplex v) {
            std::cout << "(" << std::setprecision(8) << v.real << " "
                << v.imag << ")";
        }

        void print(const DoubleComplex v) {
            std::cout << "(" << std::setprecision(16) << v.real << " "
                << v.imag << ")";
        }

        void print(const Direction v) {
            std::cout << "(" << std::setprecision(16) << v.coord1 << " "
                << v.coord2 << " " << v.sys << ")";
        }

        template <class T>
            void print(std::vector<T>& v) {
                if (!verbose) {
                    std::cout << "< Vector of size " << v.size()
                        << " - To display contents enable verbose mode >";
                    return;
                }
                typename std::vector<T>::iterator it = v.begin();
                std::cout << "[ ";
                while (it != v.end()) {
                    if (it != v.begin()) {
                        std::cout << ", ";
                    }
                    print(*it);
                    ++it;
                }
                std::cout << " ]";
            }
};

// main()
int main(int argc, char *argv[])
{
    // Initialise the logger
    std::ostringstream ss;
    ss << argv[0] << ".log_cfg";
    ASKAPLOG_INIT(ss.str().c_str());

    // Command line parser
    cmdlineparser::Parser parser;

    // Command line parameters
    cmdlineparser::FlaggedParameter<std::string> inputsPar("-inputs", "msnoop.in");
    cmdlineparser::FlagParameter verbosePar("-v");

    // Set handler for case where parameter is not set
    parser.add(inputsPar, cmdlineparser::Parser::throw_exception);
    parser.add(verbosePar, cmdlineparser::Parser::return_default);

    try {
        parser.process(argc, const_cast<char**> (argv));
        verbose = verbosePar.defined();
    } catch (const cmdlineparser::XParser&) {
        std::cout << "usage: " << argv[0] << " [-v] -inputs <filename>" << std::endl;
        std::cerr << "  -v      \tEnable more verbose output" << std::endl;
        std::cerr << "  -inputs <filename>\tFilename for the config file" << std::endl;

        return 1;
    }

    LOFAR::ParameterSet parset(inputsPar);
    const std::string locatorHost = parset.getString("ice.locator_host");
    const std::string locatorPort = parset.getString("ice.locator_port");
    const std::string topicManager = parset.getString("icestorm.topicmanager");
    const std::string topic = parset.getString("icestorm.topic");
    const std::string adapterName = parset.getString("ice.adapter_name");

    MetadataSubscriber subscriber(locatorHost, locatorPort , topicManager,
            topic, adapterName);

    std::cout << "Waiting for messages (press CTRL-C to exit)..." << std::endl;
    while (true) {
        sleep(1);
    }

    return 0;
}
