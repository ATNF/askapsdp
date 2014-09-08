#!/bin/bash -l

nfeedsSquare=`echo $nfeeds | awk '{print sqrt($1)}'`
xloc=()
yloc=()
for((x=1;x<=${nfeedsSquare};x++)); do
    for((y=1;y<=${nfeedsSquare};y++)); do
	xloc=(${xloc[@]} $x)
	yloc=(${yloc[@]} $y)
    done
done

#xloc=(1 1 1 2 2 2 3 3 3)
#yloc=(1 2 3 1 2 3 1 2 3)

#echo ${xloc[@]}
#echo ${yloc[@]}

feedparset=`echo $nfeeds $POINTING | awk '{printf "parsets/ASKAP%dfeeds_ref%02d.in",$1,$2}'`
feednames="feed0"
for((i=1;i<$nfeeds;i++)); do feednames="${feednames}, feed${i}"; done
cat > $feedparset <<EOF
feeds.spacing        =       1deg
feeds.names          =       [${feednames}]

EOF

for((i=0;i<${nfeeds};i++)); do

    xoff=`expr ${xloc[$i]} - ${xloc[$POINTING]}`
    yoff=`expr ${yloc[$i]} - ${yloc[$POINTING]}`

    #echo $i $POINTING $xoff $yoff

    cat >> $feedparset <<EOF
feeds.feed${i}       =       [$xoff, $yoff]
EOF
    
done
