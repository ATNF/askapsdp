//# MWSubtractStepBBS.cc
//#
/// @copyright (c) 2007 CSIRO
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
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <mwcontrol/MWSubtractStepBBS.h>
#include <mwcommon/MWStepFactory.h>
#include <mwcommon/MWError.h>

using namespace LOFAR;

namespace askap { namespace cp {

  MWSubtractStepBBS::MWSubtractStepBBS()
  {}

  MWSubtractStepBBS::~MWSubtractStepBBS()
  {}

  MWSubtractStepBBS* MWSubtractStepBBS::clone() const
  {
    return new MWSubtractStepBBS(*this);
  }

  MWStep::ShPtr MWSubtractStepBBS::create()
  {
    return MWStep::ShPtr (new MWSubtractStepBBS());
  }

  void MWSubtractStepBBS::registerCreate()
  {
    MWStepFactory::push_back ("MWSubtractStepBBS", &create);
  }

  std::string MWSubtractStepBBS::className() const
  {
    static std::string name("MWSubtractStepBBS");
    return name;
  }

  void MWSubtractStepBBS::visit (MWStepVisitor& visitor) const
  {
    visitor.visitSubtract (*this);
  }

  void MWSubtractStepBBS::toBlob (BlobOStream& bs) const
  {
    bs.putStart (className(), 1);
    itsProp.toBlob (bs);
    bs.putEnd();
  }

  void MWSubtractStepBBS::fromBlob (BlobIStream& bs)
  {
    int vers = bs.getStart (className());
    ASKAPASSERT (vers == 1);
    itsProp.fromBlob (bs);
    bs.getEnd();
  }

}} // end namespaces
