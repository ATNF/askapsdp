#!/bin/sh
numactl --membind 0 --cpunodebind 0 ./GridPerf 4 > instance1.out &
numactl --membind 1 --cpunodebind 1 ./GridPerf 4 > instance2.out
