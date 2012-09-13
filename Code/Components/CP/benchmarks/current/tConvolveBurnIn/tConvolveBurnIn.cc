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
/// @detail
/// This C++ program has been written to demonstrate the convolutional resampling
/// algorithm used in radio interferometry. It should compile with:
/// mpicxx -O3 -fstrict-aliasing -fcx-limited-range -Wall -c tConvolveBurnIn.cc
/// mpicxx -O3 -fstrict-aliasing -fcx-limited-range -Wall -c Stopwatch.cc
/// mpicxx -O3 -fstrict-aliasing -fcx-limited-range -Wall -c Benchmark.cc
/// mpicxx -o tConvolveBurnIn tConvolveBurnIn.o Stopwatch.o Benchmark.o 
///
/// -fstrict-aliasing - tells the compiler that there are no memory locations
///                     accessed through aliases.
/// -fcx-limited-range - states that a range reduction step is not needed when
///                      performing complex division. This is an acceptable
///                      optimization.
///
/// @author Ben Humphreys <ben.humphreys@csiro.au>
/// @author Tim Cornwell  <tim.cornwell@csiro.au>

// Include own header file first
#include "tConvolveBurnIn.h"

// System & MPI includes
#include <iostream>
#include <cstdlib>
#include <mpi.h>

// BLAS includes
#ifdef USEBLAS

#define CAXPY cblas_caxpy
#define CDOTU_SUB cblas_cdotu_sub

#include <mkl_cblas.h>

#endif

// Local includes
#include "Benchmark.h"
#include "Stopwatch.h"

// Main testing routine
int main(int argc, char *argv[])
{
    // Initialize MPI
    int rc = MPI_Init(&argc, &argv);

    if (rc != MPI_SUCCESS) {
        printf("Error starting MPI program. Terminating.\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
    }

    if (argc != 1 || argc != 2) {
        std::cerr << "usage: " << argv[0] << " [# of cycles]" << std::endl;
        exit(1);
    }

    int ncycles = 1;
    if (argc == 2) {
        ncycles = atoi(argv[1]);
    }

    int numtasks;
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Setup the benchmark class
    Benchmark bmark;
    bmark.init();

    unsigned long griddingErrors = 0;
    unsigned long degriddingErrors = 0;
    for (int cycle = 1; cycle <= ncycles; ++cycle) {
        if (rank == 0) {
            std::cout << "+++++ Cycle " << cycle << " of " << ncycles << " +++++" << std::endl;
        }

        // Run the gridding
        Stopwatch sw;
        sw.start();
        bool gridSuccess = bmark.runGrid();
        double time = sw.stop();

        // Propogate the error count to the master
        int errors = gridSuccess ? 0 : 1;
        if (rank == 0) {
            int totalErrors = 0;
            griddingErrors += totalErrors;
            MPI_Reduce(&errors, &totalErrors, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
        } else {
            MPI_Reduce(&errors, 0, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
        }

        // Run the degridding
        sw.start();
        bool degridSuccess = bmark.runDegrid();
        time = sw.stop();

        // Propogate the error count to the master
        errors = degridSuccess ? 0 : 1;
        if (rank == 0) {
            int totalErrors = 0;
            MPI_Reduce(&errors, &totalErrors, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
            degriddingErrors += totalErrors;
        } else {
            MPI_Reduce(&errors, 0, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
        }

        // Report (master reports only)
        if (rank == 0) {
            std::cout << "    Number of processes:    " << numtasks << std::endl;
            std::cout << "    Gridding error count:   " << griddingErrors << std::endl;
            std::cout << "    Degridding error count: " << degriddingErrors << std::endl;
        }

        MPI_Barrier(MPI_COMM_WORLD);
    }

    MPI_Finalize();

    return 0;
}
