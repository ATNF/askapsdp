/// @file CleanRequest.cc
///
/// @copyright (c) 2009 CSIRO
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

// Include own header file first
#include <messages/CleanRequest.h>

// ASKAPsoft includes
#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>
#include <casa/Arrays/Array.h>
#include <Blob/BlobArray.h>
#include <Blob/BlobSTL.h>

// Using
using namespace askap;
using namespace askap::cp;

CleanRequest::CleanRequest()
{
}

CleanRequest::~CleanRequest()
{
}

IMessage::MessageType CleanRequest::getMessageType(void) const
{
    return IMessage::CLEAN_REQUEST;
}

/////////////////////////////////////////////////////////////////////
// Setters
/////////////////////////////////////////////////////////////////////
void CleanRequest::set_payloadType(PayloadType type)
{
    itsPayloadType = type;
}

void CleanRequest::set_patchId(int patchId)
{
    itsPatchId = patchId;
}

void CleanRequest::set_dirty(const casa::Array<float>& dirty)
{
    itsDirty = dirty;
}

void CleanRequest::set_psf(const casa::Array<float>& psf)
{
    itsPsf = psf;
}

void CleanRequest::set_mask(const casa::Array<float>& mask)
{
    itsMask = mask;
}

void CleanRequest::set_model(const casa::Array<float>& model)
{
    itsModel = model;
}

void CleanRequest::set_threshold(double threshold)
{
    itsThreshold = threshold;
}

void CleanRequest::set_thresholdUnits(std::string thresholdUnits)
{
    itsThresholdUnits = thresholdUnits;
}

void CleanRequest::set_fractionalThreshold(double fractionalThreshold)
{
    itsFractionalThreshold = fractionalThreshold;
}

void CleanRequest::set_scales(casa::Vector<float> scales)
{
    itsScales = scales;
}

void CleanRequest::set_niter(int niter)
{
    itsNiter = niter;
}

void CleanRequest::set_gain(double gain)
{
    itsGain = gain;
}

/////////////////////////////////////////////////////////////////////
// Getters
/////////////////////////////////////////////////////////////////////
CleanRequest::PayloadType CleanRequest::get_payloadType(void) const
{
    return itsPayloadType;
}

int CleanRequest::get_patchId(void) const
{
    return itsPatchId;
}

const casa::Array<float>& CleanRequest::get_dirty(void) const
{
    return itsDirty;
}

casa::Array<float>& CleanRequest::get_dirty(void)
{
    return itsDirty;
}

const casa::Array<float>& CleanRequest::get_psf(void) const
{
    return itsPsf;
}

casa::Array<float>& CleanRequest::get_psf(void)
{
    return itsPsf;
}

const casa::Array<float>& CleanRequest::get_mask(void) const
{
    return itsMask;
}

casa::Array<float>& CleanRequest::get_mask(void)
{
    return itsMask;
}

const casa::Array<float>& CleanRequest::get_model(void) const
{
    return itsModel;
}

casa::Array<float>& CleanRequest::get_model(void)
{
    return itsModel;
}

double CleanRequest::get_threshold(void) const
{
    return itsThreshold;
}

std::string CleanRequest::get_thresholdUnits(void) const
{
    return itsThresholdUnits;
}

double CleanRequest::get_fractionalThreshold(void) const
{
    return itsFractionalThreshold;
}

casa::Vector<float> CleanRequest::get_scales(void) const
{
    return itsScales;
}

int CleanRequest::get_niter(void) const
{
    return itsNiter;
}

double CleanRequest::get_gain(void) const
{
    return itsGain;
}


/////////////////////////////////////////////////////////////////////
// Serializers
/////////////////////////////////////////////////////////////////////
void CleanRequest::writeToBlob(LOFAR::BlobOStream& os) const
{
    os << static_cast<int>(itsPayloadType);
    if (itsPayloadType == WORK) {
        os  << itsPatchId << itsDirty << itsPsf
            << itsMask << itsModel << itsThreshold << itsThresholdUnits
            << itsFractionalThreshold << itsScales << itsNiter << itsGain;
    }
}

void CleanRequest::readFromBlob(LOFAR::BlobIStream& is)
{
    int payloadType;
    is >> payloadType;
    itsPayloadType = static_cast<PayloadType>(payloadType);

    if (itsPayloadType == WORK) {
        is >> itsPatchId >> itsDirty >> itsPsf >> itsMask
            >> itsModel >> itsThreshold >> itsThresholdUnits >> itsFractionalThreshold
            >> itsScales >> itsNiter >> itsGain;
    }
}
