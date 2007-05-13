#include <fitting/Params.h>
#include <fitting/Equation.h>

#include <stdexcept>

namespace conrad
{
namespace scimath
{

Equation::Equation(const Equation& other) {
    operator=(other);
}

Equation& Equation::operator=(const Equation& other) {
    if(this!=&other) {
        itsParams=other.itsParams;
        itsDefaultParams=other.itsDefaultParams;
    }
}

/// Using specified parameters
Equation::Equation(const Params& ip) : itsParams(ip) {};

Equation::~Equation(){};

/// Access the parameters
const Params& Equation::parameters() const {return itsParams;};
Params& Equation::parameters() {return itsParams;};

/// Set the parameters to new values
/// @param ip Parameters
void Equation::setParameters(const Params& ip) {itsParams=ip;};

/// Check if set of parameters is valid for this equation
/// @param ip Parameters
bool Equation::complete(const Params& ip) {return itsDefaultParams.isCongruent(ip);};

/// Return a default set of parameters
/// @param ip Parameters
const Params& Equation::defaultParameters() const {return itsDefaultParams;};

}

}
