#!/usr/bin/env bash

startHA=`echo $nfeeds | awk '{if($1%2==0) ha=-1.*int($1/2.)/10.; else ha=-1.*int($1/2.+1)/10.; printf "%3.1f",ha}'`
#echo $startHA
ha=$startHA
calHArange=()
for((f=1;f<=$nfeeds;f++)); do
    oldha=$ha
    ha=`echo $ha | awk '{printf "%3.1f", $1+0.1}'`
#    echo "${oldha}h,${ha}h"
    calHArange=(${calHArange[@]} "${oldha}h,${ha}h")
done
