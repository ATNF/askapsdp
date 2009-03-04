#!/bin/sh

data=data/10uJy.txt
tmpfile=tmpfile

for flux in {100,200,400}; do
    fluxcut=`echo $flux | awk '{print $1/1000000.}'`

    out="data/${flux}uJy_pt.txt"
    cat > $tmpfile <<EOF 
BEGIN{ct=0;}
{if(ct>1){
  if(\$3>${fluxcut} && \$4==0.){
    ra=\$1/15.;rah=int(ra); ram=int((ra-rah)*60.); ras=((ra-rah)*60.-ram)*60.;
    absdec=sqrt(\$2*\$2); decd=int(absdec); decm=int((absdec-decd)*60.); decs=((absdec-decd)*60.-decm)*60;
    decsign=absdec;
    if(\$2<0) decsign=-1*absdec; 
    flux=\$3;
    printf "%02d:%02d:%05.2f %02d:%02d:%05.2f %10.8f %6.4f %6.4f %6.4f\n",rah,ram,ras,decsign,decm,decs,flux,\$4,\$5,\$6;
  }
 } 
 ct++;}
EOF
    awk -f $tmpfile $data > $out

    out="data/${flux}uJy_pt.ann"
#cat > $tmpfile <<EOF 
#BEGIN{ct=0; ct2=1;printf "COORD w\nCOLOR SEA GREEN\nFONT lucidasans-10\n";}
#{if(ct>1){
#  if(\$3>${fluxcut} && \$4==0.){
#    printf "CIRCLE %12.8f %12.8f %10.8f\n\nTEXT %12.8f %12.8f %d\n",\$1,\$2,10./(3600.*sqrt(2*log(2))),\$1,\$2,ct2++;
#  }
# } 
# ct++;}
#EOF
    cat > $tmpfile <<EOF 
BEGIN{ct=0; ct2=1;printf "COORD w\nCOLOR SEA GREEN\nFONT lucidasans-10\n";}
{if(ct>1){
  if(\$3>${fluxcut} && \$4==0.){
    printf "CIRCLE %12.8f %12.8f %10.8f\nTEXT %12.8f %12.8f %d\n",\$1,\$2,10./3600.,\$1,\$2,ct2++;
  }
 } 
 ct++;}
EOF
    awk -f $tmpfile $data > $out

    for radius in {1,2,3}; do

	out="data/${flux}uJy_pt_${radius}deg.txt"
	cat > $tmpfile <<EOF
BEGIN{ct=0;rabase=187.5;decbase=-45.;DtoR=3.141592654/180.}
{if(ct>1){
  if(\$3>${fluxcut} && \$4==0.){
    ra=\$1/15.;rah=int(ra); ram=int((ra-rah)*60.); ras=((ra-rah)*60.-ram)*60.;
    absdec=sqrt(\$2*\$2); decd=int(absdec); decm=int((absdec-decd)*60.); decs=((absdec-decd)*60.-decm)*60;
    decsign=absdec;
    if(\$2<0) decsign=-1*absdec;
    cos_sep=cos((\$1-rabase)*DtoR)*cos(\$2*DtoR)*cos(decbase*DtoR) + sin(\$2*DtoR)*sin(decbase*DtoR); cos_limit=cos(${radius}*DtoR);
    flux=\$3;
    if(cos_sep>cos_limit) printf "%02d:%02d:%05.2f %02d:%02d:%05.2f %10.8f %6.4f %6.4f %6.4f\n",rah,ram,ras,decsign,decm,decs,flux,\$4,\$5,\$6;
  }
 }
 ct++;}
EOF
	awk -f $tmpfile $data > $out

	out="data/${flux}uJy_pt_${radius}deg.ann"
#cat > $tmpfile <<EOF 
#BEGIN{ct=0;ct2=1;rabase=187.5;decbase=-45.;DtoR=3.141592654/180.; printf "COORD w\nCOLOR SEA GREEN\nFONT lucidasans-10\n";}
#{if(ct>1){
#  if(\$3>${fluxcut} && \$4==0.){
#    cos_sep=cos((\$1-rabase)*DtoR)*cos(\$2*DtoR)*cos(decbase*DtoR) + sin(\$2*DtoR)*sin(decbase*DtoR); cos_limit=cos(1.*DtoR);
#    if(cos_sep>cos_limit) printf "CIRCLE %12.8f %12.8f %10.8f\nTEXT %12.8f %12.8f %d\n",\$1,\$2,10./(3600.*sqrt(2*log(2))),\$1,\$2,ct2++;
#  }
# } 
# ct++;}
#EOF
	cat > $tmpfile <<EOF 
BEGIN{ct=0;ct2=1;rabase=187.5;decbase=-45.;DtoR=3.141592654/180.; printf "COORD w\nCOLOR SEA GREEN\nFONT lucidasans-10\n";}
{if(ct>1){
  if(\$3>${fluxcut} && \$4==0.){
    cos_sep=cos((\$1-rabase)*DtoR)*cos(\$2*DtoR)*cos(decbase*DtoR) + sin(\$2*DtoR)*sin(decbase*DtoR); cos_limit=cos(${radius}*DtoR);
    if(cos_sep>cos_limit) printf "CIRCLE %12.8f %12.8f %10.8f\nTEXT %12.8f %12.8f %d\n",\$1,\$2,10./3600.,\$1,\$2,ct2++;
  }
 } 
 ct++;}
EOF
	awk -f $tmpfile $data > $out

    done

done 

rm -f $tmpfile
