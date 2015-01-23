/// @file
///
/// XXX Notes on program XXX
///
/// @copyright (c) 2008 CSIRO
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
/// @author XXX XXX <XXX.XXX@csiro.au>
///
#include <preprocessing/VariableThresholdingHelpers.h>
#include <cppunit/extensions/HelperMacros.h>
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <duchamp/Utils/Statistics.hh>

ASKAP_LOGGER(logger2, ".maskSlidingMathTest");
namespace askap {
namespace analysis {

/* const size_t dim=8; */
/* const size_t boxWidth=1; */

class MaskedSlidingMathTest : public CppUnit::TestFixture {
        CPPUNIT_TEST_SUITE(MaskedSlidingMathTest);
        CPPUNIT_TEST(testBoxMean);
        CPPUNIT_TEST(testBoxStddev);
        CPPUNIT_TEST(testBoxMedian);
        CPPUNIT_TEST(testBoxMadfm);
        CPPUNIT_TEST_SUITE_END();

    private:
        casa::MaskedArray<Float> inputMaskArr;
        casa::LogicalArray inputMask;
        casa::Array<Float> inputArr;
        casa::Array<Float> checkBoxMeanArr;
        casa::Array<Float> checkBoxMedianArr;
        casa::Array<Float> checkBoxStddevArr;
        casa::Array<Float> checkBoxMadfmArr;
        casa::IPosition shape;
    public:

        void setUp()
        {

            const size_t arrsize = dim * dim;
            float input[arrsize] = { 1, 2, 3, 4, 5, 6, 7, 8,
                                     9, 10, 11, 12, 13, 14, 15, 16,
                                     17, 18, 19, 20, 21, 22, 23, 24,
                                     25, 26, 27, 28, 29, 30, 31, 32,
                                     33, 34, 35, 36, 37, 38, 39, 40,
                                     41, 42, 43, 44, 45, 46, 47, 48,
                                     49, 50, 51, 52, 53, 54, 55, 56,
                                     57, 58, 59, 60, 61, 62, 63, 64
                                   };
            shape = casa::IPosition(2, dim, dim);
            inputArr.takeStorage(shape, input);
            inputMask = casa::LogicalArray(shape, true);
            // Flag three points in one of the corners to test for the response
            inputMask(casa::IPosition(2, 7, 0)) = false;
            inputMask(casa::IPosition(2, 6, 0)) = false;
            inputMask(casa::IPosition(2, 7, 1)) = false;
            inputMaskArr.setData(inputArr, inputMask);

            // The following is the unmasked mean array
            float checkBoxMean[arrsize] = {0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 10, 11, 12, 13, 14, 15, 0,
                                           0, 18, 19, 20, 21, 22, 23, 0,
                                           0, 26, 27, 28, 29, 30, 31, 0,
                                           0, 34, 35, 36, 37, 38, 39, 0,
                                           0, 42, 43, 44, 45, 46, 47, 0,
                                           0, 50, 51, 52, 53, 54, 55, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0
                                          };
            checkBoxMeanArr.takeStorage(shape, checkBoxMean);
            // Now correct for the three values in the corner where we have the mask
            checkBoxMeanArr(casa::IPosition(2, 6, 1)) = (6 + 14 + 15 +
                    22 + 23 + 24) / 6.;
            checkBoxMeanArr(casa::IPosition(2, 5, 1)) = (5 + 6 + 13 + 14 +
                    15 + 21 + 22 + 23) / 8.;
            checkBoxMeanArr(casa::IPosition(2, 6, 2)) = (14 + 15 + 22 + 23 +
                    24 + 30 + 31 + 32) / 8.;

            // Median array has the same values away from the flagged points
            checkBoxMedianArr.takeStorage(shape, checkBoxMean);
            // Now correct for the three values in the corner where we have the mask
            checkBoxMedianArr(casa::IPosition(2, 6, 1)) = 18.5;
            checkBoxMedianArr(casa::IPosition(2, 5, 1)) = 14.5;
            checkBoxMedianArr(casa::IPosition(2, 6, 2)) = 23.5;

            float base[arrsize] = {0, 0, 0, 0, 0, 0, 0, 0,
                                   0, 1, 1, 1, 1, 1, 1, 0,
                                   0, 1, 1, 1, 1, 1, 1, 0,
                                   0, 1, 1, 1, 1, 1, 1, 0,
                                   0, 1, 1, 1, 1, 1, 1, 0,
                                   0, 1, 1, 1, 1, 1, 1, 0,
                                   0, 1, 1, 1, 1, 1, 1, 0,
                                   0, 0, 0, 0, 0, 0, 0, 0
                                  };

            checkBoxStddevArr.takeStorage(shape, base);
            float stddevVal = sqrt(2.*(7 * 7 + 8 * 8 + 9 * 9 + 1) / 8.);
            checkBoxStddevArr *= stddevVal;
            // Now correct for the three values in the corner where we have the mask
            float tmp = checkBoxMeanArr(casa::IPosition(2, 6, 1));
            checkBoxStddevArr(casa::IPosition(2, 6, 1)) = sqrt(((6 - tmp) * (6 - tmp) +
                    (14 - tmp) * (14 - tmp) +
                    (15 - tmp) * (15 - tmp) +
                    (22 - tmp) * (22 - tmp) +
                    (23 - tmp) * (23 - tmp) +
                    (24 - tmp) * (24 - tmp)) / 5.);
            tmp = checkBoxMeanArr(casa::IPosition(2, 5, 1));
            checkBoxStddevArr(casa::IPosition(2, 5, 1)) = sqrt(((5 - tmp) * (5 - tmp) +
                    (6 - tmp) * (6 - tmp) +
                    (13 - tmp) * (13 - tmp) +
                    (14 - tmp) * (14 - tmp) +
                    (15 - tmp) * (15 - tmp) +
                    (21 - tmp) * (21 - tmp) +
                    (22 - tmp) * (22 - tmp) +
                    (23 - tmp) * (23 - tmp)) / 7.);
            tmp = checkBoxMeanArr(casa::IPosition(2, 6, 2));
            checkBoxStddevArr(casa::IPosition(2, 6, 2)) = sqrt(((14 - tmp) * (14 - tmp) +
                    (15 - tmp) * (15 - tmp) +
                    (22 - tmp) * (22 - tmp) +
                    (23 - tmp) * (23 - tmp) +
                    (24 - tmp) * (24 - tmp) +
                    (30 - tmp) * (30 - tmp) +
                    (31 - tmp) * (31 - tmp) +
                    (32 - tmp) * (32 - tmp)) / 7.);



            checkBoxMadfmArr.takeStorage(shape, base);
            float madfmVal = 7.0 / Statistics::correctionFactor;
            checkBoxMadfmArr *= madfmVal;
            // Now correct for the three values in the corner where we have the mask
            checkBoxMadfmArr(casa::IPosition(2, 6, 1)) = 4.5 / Statistics::correctionFactor;
            checkBoxMadfmArr(casa::IPosition(2, 5, 1)) = 7.0 / Statistics::correctionFactor;
            checkBoxMadfmArr(casa::IPosition(2, 6, 2)) = 7.0 / Statistics::correctionFactor;

        }

        void testBoxMean()
        {
            casa::IPosition box(2, boxWidth, boxWidth);
            casa::Array<Float> middle(shape, 0.);
            casa::Array<Float> spread(shape, 0.);
            size_t dimMin = 0;
            size_t dimMax = dim;
            /* size_t dimMin=boxWidth; */
            /* size_t dimMax=dim-boxWidth; */
            casa::MaskedArray<Float> localInput(inputMaskArr);
            ASKAPLOG_DEBUG_STR(logger2, "Sliding math test - mean");
            slidingBoxMaskedStats(localInput, middle, spread, box, false);
            /* ASKAPLOG_DEBUG_STR(logger2, "Calculated mean follows: " << middle); */
            /* ASKAPLOG_DEBUG_STR(logger2, "Should be: " << checkBoxMeanArr); */
            for (size_t y = dimMin; y < dimMax; y++) {
                for (size_t x = dimMin; x < dimMax; x++) {
                    casa::IPosition pos(2, x, y);
                    CPPUNIT_ASSERT(fabs(middle(pos) - checkBoxMeanArr(pos)) < 1.e-5);
                }
            }
            ASKAPLOG_DEBUG_STR(logger2, "Confirming input unchanged:");
            /* ASKAPLOG_DEBUG_STR(logger2, "Input following slidingBoxMaskedStats: " << localInput.getArray()); */
            /* ASKAPLOG_DEBUG_STR(logger2, "Input Mask following slidingBoxMaskedStats: " << localInput.getMask()); */
            /* ASKAPLOG_DEBUG_STR(logger2, "Original Input: " << inputMaskArr.getArray()); */
            /* ASKAPLOG_DEBUG_STR(logger2, "Original Input Mask: " << inputMaskArr.getMask()); */
            for (size_t y = dimMin; y < dimMax; y++) {
                for (size_t x = dimMin; x < dimMax; x++) {
                    casa::IPosition pos(2, x, y);
                    CPPUNIT_ASSERT(fabs(inputArr(pos) - localInput.getArray()(pos)) < 1.e-5);
                }
            }
        }

        void testBoxStddev()
        {
            casa::IPosition box(2, boxWidth, boxWidth);
            casa::Array<Float> middle(shape, 0.);
            casa::Array<Float> spread(shape, 0.);
            size_t dimMin = 0;
            size_t dimMax = dim;
            /* size_t dimMin=boxWidth; */
            /* size_t dimMax=dim-boxWidth; */
            casa::MaskedArray<Float> localInput(inputMaskArr);
            ASKAPLOG_DEBUG_STR(logger2, "Sliding math test - stddev");
            slidingBoxMaskedStats(localInput, middle, spread, box, false);
            ASKAPLOG_DEBUG_STR(logger2, "Calculated stddev follows: " << spread);
            ASKAPLOG_DEBUG_STR(logger2, "Should be: " << checkBoxStddevArr);
            for (size_t y = dimMin; y < dimMax; y++) {
                for (size_t x = dimMin; x < dimMax; x++) {
                    casa::IPosition pos(2, x, y);
                    CPPUNIT_ASSERT(fabs(middle(pos) - checkBoxMeanArr(pos)) < 1.e-5);
                }
            }
            ASKAPLOG_DEBUG_STR(logger2, "Confirming input unchanged");
            for (size_t y = dimMin; y < dimMax; y++) {
                for (size_t x = dimMin; x < dimMax; x++) {
                    casa::IPosition pos(2, x, y);
                    CPPUNIT_ASSERT(fabs(inputArr(pos) - localInput.getArray()(pos)) < 1.e-5);
                }
            }
        }

        void testBoxMedian()
        {
            casa::IPosition box(2, boxWidth, boxWidth);
            casa::Array<Float> middle(shape, 0.);
            casa::Array<Float> spread(shape, 0.);
            size_t dimMin = 0;
            size_t dimMax = dim;
            ASKAPLOG_DEBUG_STR(logger2, "Sliding math test - median");
            casa::MaskedArray<Float> localInput(inputMaskArr);
            slidingBoxMaskedStats(localInput, middle, spread, box, true);
            /* ASKAPLOG_DEBUG_STR(logger2, "Calculated median follows: " << middle); */
            /* ASKAPLOG_DEBUG_STR(logger2, "Should be: " << checkBoxMedianArr); */
            for (size_t y = dimMin; y < dimMax; y++) {
                for (size_t x = dimMin; x < dimMax; x++) {
                    casa::IPosition pos(2, x, y);
                    CPPUNIT_ASSERT(fabs(middle(pos) - checkBoxMedianArr(pos)) < 1.e-5);
                }
            }
            ASKAPLOG_DEBUG_STR(logger2, "Confirming input unchanged");
            for (size_t y = dimMin; y < dimMax; y++) {
                for (size_t x = dimMin; x < dimMax; x++) {
                    casa::IPosition pos(2, x, y);
                    CPPUNIT_ASSERT(fabs(inputArr(pos) - localInput.getArray()(pos)) < 1.e-5);
                }
            }

        }

        void testBoxMadfm()
        {
            casa::IPosition box(2, boxWidth, boxWidth);
            casa::Array<Float> middle(shape, 0.);
            casa::Array<Float> spread(shape, 0.);
            size_t dimMin = 0;
            size_t dimMax = dim;
            ASKAPLOG_DEBUG_STR(logger2, "Sliding math test - median");
            casa::MaskedArray<Float> localInput(inputMaskArr);
            slidingBoxMaskedStats(localInput, middle, spread, box, true);
            /* ASKAPLOG_DEBUG_STR(logger2, "Calculated madfm follows: " << spread); */
            /* ASKAPLOG_DEBUG_STR(logger2, "Should be: " << checkBoxMadfmArr); */
            for (size_t y = dimMin; y < dimMax; y++) {
                for (size_t x = dimMin; x < dimMax; x++) {
                    casa::IPosition pos(2, x, y);
                    CPPUNIT_ASSERT(fabs(spread(pos) - checkBoxMadfmArr(pos)) < 1.e-5);
                }
            }
            ASKAPLOG_DEBUG_STR(logger2, "Confirming input unchanged");
            for (size_t y = dimMin; y < dimMax; y++) {
                for (size_t x = dimMin; x < dimMax; x++) {
                    casa::IPosition pos(2, x, y);
                    CPPUNIT_ASSERT(fabs(inputArr(pos) - localInput.getArray()(pos)) < 1.e-5);
                }
            }

        }

        void tearDown()
        {
        }

};

}
}
