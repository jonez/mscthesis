#!/bin/bash

file=$1
runs=$2

echo "input(ns)"
cat $file | grep -A4 "Profiling event 'fill input texture" | grep "queued > ended:" | awk '{print $4}' | awk -F "ns" '{print $1}' | awk '{ sum += $NF } END {print (sum + 0) / '$runs'}'
echo "valid input(ns)"
cat $file | grep -A4 "Profiling event 'fill valid space texture'" | grep "queued > ended:" | awk '{print $4}' | awk -F "ns" '{print $1}' | awk '{ sum += $NF } END {print (sum + 0) / '$runs'}'
echo "classification(ns)"
cat $file | grep -A4 "Profiling event 'classification kernel'" | grep "queued > ended:" | awk '{print $4}' | awk -F "ns" '{print $1}' | awk '{ sum += $NF } END {print (sum + 0) / '$runs'}'
echo "compaction(ns)"
cat $file | grep -A4 "Profiling event 'compaction kernel'" | grep "queued > ended:" | awk '{print $4}' | awk -F "ns" '{print $1}' | awk '{ sum += $NF } END {print (sum + 0) / '$runs'}'
echo "generation(ns)"
cat $file | grep -A4 "Profiling event 'generation kernel'" | grep "queued > ended:" | awk '{print $4}' | awk -F "ns" '{print $1}' | awk '{ sum += $NF } END {print (sum + 0) / '$runs'}'
echo "total(ms)"

