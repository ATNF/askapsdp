//
// Simple test program to examine processing speed and cache behavior.
//
// Example run 
// g++ -msse2 -O2 -fstrict-aliasing tSaxpy.cc -o tSaxpy;time ./tSaxpy
//Float
//2^0: 1 8.83731 (ns)
//2^1: 2 5.72486 (ns)
//2^2: 4 4.04966 (ns)
//2^3: 8 3.2893 (ns)
//2^4: 16 2.91672 (ns)
//2^5: 32 2.73621 (ns)
//2^6: 64 2.5383 (ns)
//2^7: 128 2.54507 (ns)
//2^8: 256 2.5201 (ns)
//2^9: 512 2.45864 (ns)
//2^10: 1024 2.4425 (ns)
//2^11: 2048 2.45247 (ns)
//2^12: 4096 2.44942 (ns)
//2^13: 8192 2.48426 (ns)
//2^14: 16384 2.48085 (ns)
//2^15: 32768 2.48958 (ns)
//2^16: 65536 2.49329 (ns)
//2^17: 131072 2.71859 (ns)
//2^18: 262144 3.81012 (ns)
//2^19: 524288 4.2592 (ns)
//2^20: 1048576 4.27347 (ns)
//2^21: 2097152 4.13812 (ns)
//2^22: 4194304 4.33921 (ns)
//Double
//2^0: 1 13.8606 (ns)
//2^1: 2 8.5519 (ns)
//2^2: 4 5.41724 (ns)
//2^3: 8 3.7911 (ns)
//2^4: 16 3.31233 (ns)
//2^5: 32 2.94748 (ns)
//2^6: 64 2.64817 (ns)
//2^7: 128 2.54131 (ns)
//2^8: 256 2.48116 (ns)
//2^9: 512 2.45513 (ns)
//2^10: 1024 2.43451 (ns)
//2^11: 2048 2.4362 (ns)
//2^12: 4096 2.68163 (ns)
//2^13: 8192 2.68047 (ns)
//2^14: 16384 2.67798 (ns)
//2^15: 32768 2.68083 (ns)
//2^16: 65536 2.72002 (ns)
//2^17: 131072 3.47831 (ns)
//2^18: 262144 6.02239 (ns)
//2^19: 524288 7.0084 (ns)
//2^20: 1048576 7.24217 (ns)
//2^21: 2097152 7.26197 (ns)
//2^22: 4194304 7.28578 (ns)
//
//real    1m45.679s
//user    1m37.778s
//sys     0m0.910s
//
// Note that the times in the middle range are similar but off by a 
// factor close to two at the high end, presumably when the data no 
// longer fits in the L2 cache
#include <iostream>
#include <ctime>
#include <complex>

using std::cout;
using std::endl;
using std::complex;

// Saxpy function
template<class T> 
void saxpy(const int n, const T* x, const T a, T* y)
{
  for (int i=0; i<n; i++)
  {
    (*(y++))+=a*(*(x++));
  }
}

// Function to time saxpy by running it a number of times. The number of repeats is
// chosen to balance the increasing number of elements
template <class T>
void timeSaxpy()
{

  int n=1;
  for (int ex=0; ex<23; ex++)
  {
    T* x=(T*)malloc(n*sizeof(T));
    T* y=(T*)malloc(n*sizeof(T));
    T a=T(10);
    for (int i=0; i<n; i++)
    {
      x[i]=T(i);
      y[i]=T(0);
    }
    int repeat=128*4194304/n;

    clock_t start, finish;

    start = clock();
    for (int r=0; r<repeat; r++)
    saxpy<T>(n, x, a, y);
    finish = clock();

    // Report on timings
    double time = (double(finish)-double(start))/CLOCKS_PER_SEC;
    cout << "2^" << ex << ": " << n << " " << 1e9*time/(double(repeat)*double(n)) << " (ns)" << endl;
    n*=2;
  }
}

main(int argc, char** argv)
{
  cout << "Float" << endl;
  timeSaxpy<float>();
  cout << "Double" << endl;
  timeSaxpy<double>();
  cout << "Complex" << endl;
  timeSaxpy<std::complex<float> >();
  cout << "DComplex" << endl;
  timeSaxpy<std::complex<double> >();
  return 0;
}
