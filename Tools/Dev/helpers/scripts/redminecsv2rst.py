#!/usr/bin/env python

import sys
import csv

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print "usage: %s <redmine_csv_file>" % sys.argv[0]
    fname = sys.argv[1]
    with open(fname, 'rb') as csvfile:
        reader = csv.reader(csvfile)
        first = True        
        print "============= ==========="
        print "Issue         Description"
        print "============= ==========="
        for row in reader:
            if first:
                first = False
                continue
            print ":issue:`%s` %s" % (row[0], row[5])
        print "============= ==========="
