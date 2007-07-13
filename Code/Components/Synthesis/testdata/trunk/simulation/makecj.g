include 'note.g';

msname     := 'sim.ms';
cjcl       := 'cj.cl';
nvssmodel  := 'nvss.model';
cjmodel    := 'cj.model';
totalmodel := 'total.model';
asciifile  := 'cj.list';

include 'measures.g';
pc:=dm.direction('J2000', '12h30m00.00', '-45d00m00.0');

include 'table.g';

tabledelete(cjmodel);
tabledelete(totalmodel);

#
# Make the raw image
#  
include 'imager.g';
myimager := imager(msname);
myimager.setimage(nx=6000, ny=6000, cellx='6arcsec',
		  celly='6arcsec', stokes="I" , mode="mfs" ,
		  doshift=T, phasecenter=pc,
		  nchan=1, spwid=1);
myimager.make(cjmodel);
myimager.done();

include 'componentlist.g';
if(tableexists(cjcl)) {
  cl:=componentlist(cjcl);
}
else {
  cl:=emptycomponentlist();
  include 'table.g';
  tabledelete(cjtab);
  cltab:=spaste(asciifile, '.tab');
  t:=tablefromascii(cltab, asciifile);
  nrows:=t.nrows();
  note('Found ', nrows, ' rows');
  ra:=t.getcol('RA');
  dec:=t.getcol('DEC');
  flux:=t.getcol('FLUX');
  bmaj:=t.getcol('BMAJ');
  bmin:=t.getcol('BMIN');
  bpa:=t.getcol('BPA');
  tflux:=0.0;
  for (row in 1:nrows) {
    bpa:=bpa%360;
    cl.simulate(1, log=F);
    ncomp:=cl.length();
    cl.setflux(ncomp, [flux[row], 0.0, 0.0, 0.0], log=F);
    cl.setrefdirframe(ncomp, 'J2000', log=F);
    cl.setrefdir(ncomp, ra[row], 'deg', dec[row], 'deg', log=F);
    if((bmaj[row]>0.0)&&(bmin[row]>0.0)) {
      cl.setshape(ncomp, 'GAUSSIAN',
		  majoraxis=spaste(bmaj[row], 'arcsec'),
		  minoraxis=spaste(bmin[row], 'arcsec'),
		  positionangle=spaste(bpa[row]%360, 'deg'), log=F);
    }
    else {
      cl.setshape(ncomp, 'POINT', log=F);
    }
  }
  cl.rename(cjcl);
}
 
note('Found ', cl.length(), ' CJ components');
include 'image.g';
im:=image(cjmodel);
include 'addcomplist.g';
addcomplist(im, cl);
ims:=im.convolve2d(spaste(cjmodel, '.smoothed'),
		   major='30arcsec', minor='30arcsec', 
		   overwrite=T);
im.done();
ims.done();
cl.done();

imt:=imagecalc(totalmodel, spaste(nvssmodel, '+', cjmodel), overwrite=T);
imt.done();

imt:=imagecalc(spaste(totalmodel, '.smoothed'),
	       spaste(nvssmodel, '.smoothed+', cjmodel, '.smoothed'),
	       overwrite=T);
imt.view();
