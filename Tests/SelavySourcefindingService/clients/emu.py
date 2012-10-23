#!/usr/bin/env python

import sys
import os
import hashlib
import uuid
from ftplib import FTP

def usage(progname):
    print "Usage:"
    print progname + " upload <filename.fits>"
    print "or"
    print progname + " submit <filename.job>"
    print "or"
    print progname + " download <jobid>"
    sys.exit(1)

def ftpconnect():
    global ftp
    ftp = FTP('ftp.atnf.csiro.au')
    ftp.login('duchamp004', 'uva8Quae')

def sendfile(localname, remotename):
    ftpconnect()
    ftp.cwd('incoming')
    ftp.storbinary("STOR " + remotename, open(localname, "rb"))
    ftp.quit()

def upload(filename):
    if os.path.splitext(filename)[1] != ".fits":
        print "Error: File should have fits extension"
        sys.exit(1)

    print "Calculating MD5 hash of file..."
    # Calculate the MD5 digest for the file
    m = hashlib.md5()
    f = open(filename, 'rb')
    while True:
        t = f.read(1024)
        if len(t) == 0:
            break # end of file
        m.update(t)
    digest = m.hexdigest()
    f.close
    
    # Upload file
    print "Uploading file " + filename + "..."
    remotename = digest + ".fits"
    sendfile(filename, remotename)

    # Upload the completion file (indicates completion of the first upload)
    sendfile("/dev/null", remotename + ".md5")

    print "Upload completed. File ID: " + remotename

def submit(filename):
    f=open(filename,'r')
    lines=f.read().split('\n')
    f.close()
    if(len([elem for elem in lines if (len(elem.split())==3 and elem[0]!='#' and elem.split()[0]=='Selavy.email')])==0):
        print "Error: No valid email address provided. Not submitting job."
        sys.exit(1)

    print "Submitting job " + filename + "..."
    job_uuid = uuid.uuid4()
    remotename = str(job_uuid) + ".job"
    sendfile(filename, remotename)

    # Upload the completion file (indicates completion of the first upload)
    sendfile("/dev/null", remotename + ".md5")

    print "Job submission complete. Job ID: " + str(job_uuid)

def download(jobid):
    print "Checking for results to download..."
    ftpconnect()
    ftp.cwd('outgoing')
    files = []
    # Find all files matching the pattern
    ftp.retrlines("NLST " + jobid + ".*", files.append)
    if len(files) == 0:
        print "No files to download for jobid " + jobid
        ftp.quit()
        return

    for filename in files:
        print "Downloading file " + filename
        file = open(filename, 'wb')
        ftp.retrbinary("RETR " + filename, file.write)
        file.close

    ftp.quit()

# main()
if len(sys.argv) != 3:
    usage(sys.argv[0])

if sys.argv[1] == "upload":
    upload(sys.argv[2])
elif sys.argv[1] == "submit":
    submit(sys.argv[2])
elif sys.argv[1] == "download":
    download(sys.argv[2])
else:
    usage(sys.argv[0])
