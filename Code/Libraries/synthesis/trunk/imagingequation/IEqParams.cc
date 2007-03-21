#include "IEqParams.h"

namespace conrad
{

IEqParams::IEqParams(const string& parmtable) {
}

// Store as a table
void IEqParams::saveAsTable(const string& parmtable) const {
}

void IEqParams::add(const string& key, const IEqParam& param) {
	insert(make_pair(key, param));
}

}