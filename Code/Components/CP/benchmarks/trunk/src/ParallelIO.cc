#include <iostream>
#include <fstream>
#include <cstdio>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef HAVE_MPI
#include <mpi.h>
#else
#include <sys/times.h>
#endif

using namespace std;

// Write one file per node
double doWriteOneFilePerNode(string &filename,
			     long fileSize,
			     int bufSize)
{
  int fd;

  // Open file
  //ofstream outFile (filename.c_str(), fstream::binary);
  //if (!outFile)
  fd = open(filename.c_str(), O_WRONLY|O_TRUNC|O_CREAT, 0666);
  if (fd <= 0)
    return 0.0;

  // Allocate buffer
  long lbufSize = (long)bufSize * 1024L * 1024L; // convert to bytes
  char *buf = new char[lbufSize];
  if (buf == NULL)
    return 0.0;

#ifdef HAVE_MPI
  // Wait for all nodes to reach this point
  MPI::COMM_WORLD.Barrier();
#endif

  // Start timer
  double elapsed = 0.0;
#ifdef HAVE_MPI
  elapsed = MPI::Wtime();
#else
  tms locTms;
  clock_t ticksElapsed = times(&locTms);
#endif

  // Write file using chunk of size bufSize until fileSize
  long total = fileSize * 1024L * 1024L;
  long cnt = 0;
  long n = 0;
  while (total > 0) {
    if ((cnt = lbufSize) > total)
      cnt = total;
    //outFile.write(buf, cnt);
    if ((n = write(fd, buf, cnt)) <= 0)
      break;
    //total -= cnt;
    total -= n;
  }

  // Flush all the buffer in the disk 
  //outFile.flush();
  fsync(fd);

  // Close file
  //outFile.close();
  close(fd);

#ifdef HAVE_MPI
  // Wait for all nodes to complete
  MPI::COMM_WORLD.Barrier();
#endif

  // Stop timer
#ifdef HAVE_MPI
  elapsed = MPI::Wtime() - elapsed;
#else
  ticksElapsed = times(&locTms) - ticksElapsed;
  elapsed = (double)ticksElapsed / ((double) sysconf(_SC_CLK_TCK));
#endif

  // Deletes allocated buffer
  delete [] buf;

  if (elapsed == 0.0) {
    elapsed = -1.0;
  }
  return (elapsed);
}

double doReadOneFilePerNode(string &filename, 
			    long fileSize,
			    int bufSize)
{
  // Open file
  int fd;
  //ifstream inFile (filename.c_str(), fstream::binary);
  //if (!inFile)
  if ((fd = open(filename.c_str(), O_RDONLY, 0666)) < 0) {
    return 0.0;
  }

  // Allocate buffer
  long lbufSize = (long)bufSize * 1024L * 1024L; // convert to Bytes
  char *buf = new char[lbufSize];
  if (buf == NULL)
    return 0.0;

#ifdef HAVE_MPI
  // Wait for all nodes to reach this point
  MPI::COMM_WORLD.Barrier();
#endif

  // Start timer
  double elapsed = 0.0;
#ifdef HAVE_MPI
  elapsed = MPI::Wtime();
#else
  tms locTms;
  clock_t ticksElapsed = times(&locTms);
#endif

  // Read file using chunk of size bufSize until fileSize
  //inFile.seekg(0);
  //while(!inFile.eof()) {
  //  inFile.read(buf, lbufSize);
  //}
  long total = fileSize * 1024L * 1024L;
  long cnt = 0;
  long n = 0;
  while(total > 0) {
    if ((cnt = lbufSize) > total)
      cnt = total;
    if ((n = read(fd, buf, cnt)) <= 0)
      break;
    total -= n;
  }

  // Close file
  //inFile.close();
  close(fd);

#ifdef HAVE_MPI
  // Wait for all nodes to complete
  MPI::COMM_WORLD.Barrier();
#endif

  // Stop timer
#ifdef HAVE_MPI
  elapsed = MPI::Wtime() - elapsed;
#else
  ticksElapsed = times(&locTms) - ticksElapsed;
  elapsed = (double)ticksElapsed / ((double) sysconf(_SC_CLK_TCK));
#endif

  // Deletes allocated buffer
  delete [] buf;

  if (elapsed == 0.0) {
    elapsed = -1.0;
  }
  return (elapsed);
}

double doReadSameFileAllNodes(string &filename,
			      long filesize,
			      int bufsize )
{
  double elapsed = -1.0;

#ifdef HAVE_MPI

  MPI::File myFile = MPI::File::Open(MPI::COMM_WORLD, filename.c_str(), MPI::MODE_RDONLY, MPI::INFO_NULL);

  filesize = filesize * 1024L * 1024L; // in bytes
  long lbufSize = bufsize * 1024L * 1024L;
  char *buf = new char[lbufSize];

  MPI::Status status;

  // Sync all nodes to this point
  MPI::COMM_WORLD.Barrier();

  // Start timer
  elapsed = MPI::Wtime();

  // Loop to read chunk of data (each chunk of size bufsize)
  long total = filesize * 1024L * 1024L;
  long cnt = 0;
  long n = 0;
  while(total > 0) {
    if ((cnt = lbufSize) > total)
      cnt = total;
    myFile.Read(buf, cnt, MPI_BYTE, status);
    if ((n = status.Get_count(MPI_BYTE)) != cnt)
      break;
    total -= n;
  }

  // Wait for all nodes to complete
  MPI::COMM_WORLD.Barrier();

  // Stop timer
  elapsed = MPI::Wtime() - elapsed;

  myFile.Close();

  delete [] buf;

#endif

  return elapsed;
}

double doReadPartFileAllNodes(string &filename,
			      long filesize,
			      int bufsize,
			      int myrank,
			      int nprocs)
{
  double elapsed = -1.0;

#ifdef HAVE_MPI

  MPI::File myFile = MPI::File::Open(MPI::COMM_WORLD, filename.c_str(), MPI::MODE_RDONLY, MPI::INFO_NULL);

  // each node will see only a portion of the file
  filesize = filesize * 1024L * 1024L / nprocs; // in bytes
  long lbufSize = bufsize * 1024L * 1024L; // in bytes
  char *buf = new char[lbufSize];

  // Sync all nodes to this point
  MPI::COMM_WORLD.Barrier();

  // Start timer
  elapsed = MPI::Wtime();

  myFile.Seek(myrank * filesize, MPI_SEEK_SET);
  MPI::Status status;
  long total = filesize;
  long cnt = 0;
  long n = 0;
  while(total > 0) {
    if ((cnt = lbufSize) > total)
      cnt = total;
    myFile.Read(buf, cnt, MPI_BYTE, status);
    if ((n = status.Get_count(MPI_BYTE)) != cnt)
      break;
    total -= n;
  }

  // Wait for all nodes to complete
  MPI::COMM_WORLD.Barrier();

  // Stop timer
  elapsed = MPI::Wtime() - elapsed;

  myFile.Close();

  delete [] buf;

#endif

  return elapsed;
}

// Usage:
//    parallelIO [path] [file_prefix] [file_size] [buffer_size]

int main(int argc, char* argv[]) {

  if (argc != 6) {
    cout << "Usage: parallelIO path file_prefix out_file file_size buffer_size" << endl;
    return 1;
  }

  int rank = 0;
  int nproc = 1;

#ifdef HAVE_MPI
  // Initialise MPI libraries
  MPI::Init(argc, argv);
  rank = MPI::COMM_WORLD.Get_rank();
  nproc = MPI::COMM_WORLD.Get_size();
#endif

  // Parse the argument list
  string path(argv[1]);
  string filePrefix(argv[2]);
  string outFilename(argv[3]);
  long fileSize = atol(argv[4]); // in MB
  int bufSize = atoi(argv[5]); // in MB

  // Create filename using path and filePrefix
  path = path + '/';
  char strRank[4];
  sprintf(strRank, "%d", rank);
  string filename = path + filePrefix + '.' + strRank; 
  
  // Measure Write One File Per Node
  double writeResult = doWriteOneFilePerNode(filename, fileSize, bufSize);

  // Measure Read One File Per Node
  double readOneResult = doReadOneFilePerNode(filename, fileSize, bufSize);

  // Measure Read Same File All Nodes
  filename = path + filePrefix + ".0";
  double readSameResult = doReadSameFileAllNodes(filename, fileSize, bufSize);

  // Measure Read Same File by All Nodes (each node will read a portion)
  double readPartResult = doReadPartFileAllNodes(filename, fileSize, bufSize, rank, nproc);

  // Only one node saves report in file
  if (rank == 0) {
    // Open output file for APPEND
    filename = path + outFilename;
    ofstream reportFile (filename.c_str(), fstream::app);

    reportFile << "ParallelIO: Number of Nodes =" << nproc << "; File Size =" << fileSize << " MB; Buffer Size =" << bufSize <<" MB" << endl;
    reportFile << "Write (one file per node): PerNode ="<< writeResult << " sec; Rate =" << ((double)fileSize/writeResult) * (double)nproc << " MB/sec" << endl;
    reportFile << "Read (one file per node): PerNode =" << readOneResult << " sec; Rate =" << ((double)fileSize/readOneResult) * (double)nproc << " MB/sec" << endl;
    reportFile << "Read (one file all nodes): Slowest Node =" << readSameResult << " sec" << endl;
    reportFile << "Read (one/part file all nodes): Total =" << readPartResult << " sec" << endl;
    reportFile << "-------------------------" << endl;
    reportFile.close();
  }

#ifdef HAVE_MPI
  // MPI Program Ends
  MPI::Finalize();
#endif

}
