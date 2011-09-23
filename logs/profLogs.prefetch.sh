#!/bin/bash

file=$1
runs=$2
dev=$3

echo "input(ns)"
cat $file | grep -A4 "PROFILING: \[$dev\] 'fill input texture'" | grep "queued > ended:" | awk '{print $4}' | awk -F "ns" '{print $1}' | awk '{ sum += $NF } END {print (sum + 0) / '$runs'}'
echo "classification(ns)"
cat $file | grep -A4 "PROFILING: \[$dev\] 'mcClassification' kernel" | grep "queued > ended:" | awk '{print $4}' | awk -F "ns" '{print $1}' | awk '{ sum += $NF } END {print (sum + 0) / '$runs'}'
echo "compaction(ns)"
cat $file | grep -A4 "PROFILING: \[$dev\] 'mcCompaction' kernel" | grep "queued > ended:" | awk '{print $4}' | awk -F "ns" '{print $1}' | awk '{ sum += $NF } END {print (sum + 0) / '$runs'}'
echo "generation(ns)"
cat $file | grep -A4 "PROFILING: \[$dev\] 'mcGeneration' kernel" | grep "queued > ended:" | awk '{print $4}' | awk -F "ns" '{print $1}' | awk '{ sum += $NF } END {print (sum + 0) / '$runs'}'
echo "input2(ns)"
cat $file | grep -A4 "PROFILING: \[$dev\] 'fill input texture2'" | grep "started > ended:" | awk '{print $4}' | awk -F "ns" '{print $1}' | awk '{ sum += $NF } END {print (sum + 0) / '$runs'}'
echo "total(ms)"
