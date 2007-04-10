#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <mpi.h>
#include <malloc.h>

#define ROOTPRINTF(a)  if (rank == 0) printf a;
size_t stob (char *s);
void report_time (char *prog, unsigned long count, char *unit);
void start_timer();
void stop_timer();


main (ac, av)
char    ** av;
{
  char    *buf, *file;
  int     fd; 
  size_t  bsiz, fsiz;
  size_t  cnt, n, total;
  int     hflag = 0;
  int     ierr, rank, nproc;
  unsigned long dataCnt;

  ierr = MPI_Init(&ac, &av);
  ierr = MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  ierr = MPI_Comm_size(MPI_COMM_WORLD, &nproc);

  ac--, av++;
  
  if (ac != 3) {
  Usage:;
    ROOTPRINTF(("Usage: write file filesize bsize\n"));
    MPI_Abort( MPI_COMM_WORLD, 1 );
    exit(1);
  }
  file = (char *) malloc(sizeof(av[0])+5);
  sprintf(file, "%s.%d", av[0], rank);
  
  fsiz = stob(av[1]);
  bsiz = stob(av[2]);
  if (fsiz < 1) {
    ROOTPRINTF(("write: bad file size (%d)\n", fsiz));
    MPI_Abort( MPI_COMM_WORLD, 1 );
    exit(1);
  }
  if (bsiz < 1) {
    ROOTPRINTF(("write: bad block size (%d)\n", bsiz));
    MPI_Abort( MPI_COMM_WORLD, 1 );
  }
  if ((buf = malloc(bsiz)) == 0) {
    perror("malloc");
    MPI_Abort( MPI_COMM_WORLD, 1 );
  }

  if ((fd = open(file, O_WRONLY|O_TRUNC|O_CREAT, 0666)) < 0) {
    perror(file);
    MPI_Abort( MPI_COMM_WORLD, 1 );
  }

  total = fsiz;
  
  MPI_Barrier( MPI_COMM_WORLD );
  start_timer();
  while (total > 0) {
    if ((cnt = bsiz) > total)
      cnt = total;
    if ((n = write(fd, buf, cnt)) <= 0)
      break;
    total -= n;
  }
  
  /* Add any system call necessary to force data to disk here */
  fsync(fd);
  
  close(fd);
  MPI_Barrier( MPI_COMM_WORLD );
  stop_timer();
  
  if (n < 0) {
    perror("write: write file");
    MPI_Abort( MPI_COMM_WORLD, 1 );
    
  }

  ROOTPRINTF(("write: total write count %u, block size %d, nproc %d\n", fsiz, bsiz, nproc));
  dataCnt = ((unsigned long)nproc) * ((unsigned long)fsiz/1024/1024);
  if (rank == 0) report_time("write", dataCnt, "Mbytes");

  /* Test the read() function */

  if ((fd = open(file, O_RDONLY, 0666)) < 0) {
    perror(file);
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  total = fsiz;

  MPI_Barrier( MPI_COMM_WORLD );
  start_timer();
  while(total > 0) {
    if ((cnt = bsiz) > total)
      cnt = total;
    if ((n = read(fd, buf, cnt)) <= 0)
      break;
    total -= n;
  }

  close(fd);
  MPI_Barrier( MPI_COMM_WORLD);
  stop_timer();

  if (n < 0) {
    perror("read: read file");
    MPI_Abort( MPI_COMM_WORLD, 1);
  }
  ROOTPRINTF(("read: total read count %u, block size %d, nproc %d\n", fsiz, bsiz, nproc));
  dataCnt = ((unsigned long)nproc) * ((unsigned long)fsiz/1024/1024);
  if (rank == 0)
    report_time("read", dataCnt, "MBytes");
  
  MPI_Finalize();
  exit(0);
}

size_t stob (char *s)
{
  size_t  n, c, b;
  
  if (*s == '0') {
    if (*++s == 'x') {
      b = 16;
      s++;
    } else
      b = 8;
  } else
    b = 10;
  
  for (n = 0; c = *s++; ) {
    switch (c) {
    case '8': case '9':
      if (b == 8)
	break;
    case '0': case '1': case '2':
    case '3': case '4': case '5':
    case '6': case '7':
      n = n * b + c - '0';
      continue;
    case 'a': case 'b': case 'c':
    case 'd': case 'e': case 'f':
    case 'A': case 'B': case 'C':
    case 'D': case 'E': case 'F':
      if (b < 16)
	break;
      n = n * 16 + (c & 0xF) + 9;
      continue;
    }
    break;
  }
  switch (c) {
  case 'b': case 'B':
    n *= 512;
    break;
  case 'k': case 'K':
    n *= 1024;
    break;
  case 'm': case 'M':
    n *= 1024*1024;
    break;
  }
  return n;
}

double         elps;

void start_timer ()
{
  elps = MPI_Wtime();
}

void stop_timer ()
{
  elps = MPI_Wtime()-elps;
}

void report_time (char *prog, unsigned long count, char *unit)
{
  printf("%s: real %.3f\n", prog, elps);
  printf("%s: rate is %u%s / %.3fsec == %.3f%s/sec\n", prog, count, unit, elps, ((double)count)/elps, unit);
}

