#! /usr/bin/python

import string

with open("AveragedMetrics") as f:
    content = f.readlines()

averages = {}
counts = {}
for line in content:
    split = line.replace(" ","").replace("\n","").split(":")
    if len(split) > 1:
        key = split[0]
        val = split[1]
        #print "key=" + key + " : val=" + val
        if not key in averages:
            averages[key] = float(val)
            counts[key] = 1
        else:
            averages[key] = averages[key] + float(val)
            counts[key] = counts[key] + 1

for key in averages:
    #print key + ": " + str(averages[key])
    averages[key] = averages[key] / counts[key]

print "Average metrics:"
for key in averages:
    print key + ": " + str(averages[key])
