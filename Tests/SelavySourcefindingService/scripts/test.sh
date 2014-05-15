#!/bin/bash -l

NUM=$1

MAIL=matthew.whiting@csiro.au

if [ $NUM -ge 5 ]; then

    /usr/sbin/sendmail $MAIL <<EOF
To: $MAIL
From: $MAIL
Subject: Testing

This is a test - we had input number $NUM

Results of df:
`df -h /exported/duchampsvc`
EOF

else

    echo A low number!

fi

