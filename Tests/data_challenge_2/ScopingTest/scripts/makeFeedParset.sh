#!/bin/bash -l

xloc=(1 1 1 2 2 2 3 3 3)
yloc=(1 2 3 1 2 3 1 2 3)

#parset=parsets/ASKAP9feeds_ref${POINTING}.in
feedparset=ASKAP9feeds_ref${POINTING}.in
cat > $feedparset <<EOF
feeds.spacing        =       1deg
feeds.names          =       [feed0, feed1, feed2, feed3, feed4, feed5, feed6, feed7, feed8]

EOF

N=0
while [ $N -lt 9 ]; do

    xoff=`expr ${xloc[$N]} - ${xloc[$POINTING]}`
    yoff=`expr ${yloc[$N]} - ${yloc[$POINTING]}`

    cat >> $feedparset <<EOF
feeds.feed${N}       =       [$xoff, $yoff]
EOF
    
    N=`expr $N + 1`

done
