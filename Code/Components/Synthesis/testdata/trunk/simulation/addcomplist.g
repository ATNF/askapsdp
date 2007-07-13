
include 'image.g';
include 'gaussian.g';

const addgaussian2d := function(a, height=1, center=[0,0], 
				fwhm=[1,1]/fwhm_to_natural, pa=0)
{

  # Rotate if necessary
  cpa := cos(pa);
  spa := sin(pa);

  width := abs(fwhm) * fwhm_to_natural;

  maxextent:=as_integer(5*fwhm[1]);
  nx:=a::shape[1];  
  ny:=a::shape[2];

  if((as_integer(center[1])<1)||(as_integer(center[1])>nx)) return a;
  if((as_integer(center[2])<1)||(as_integer(center[2])>ny)) return a;

  x:=max(1, (center[1]-maxextent)):min(nx, (center[1]+maxextent));
  y:=max(1, (center[2]-maxextent)):min(nx, (center[2]+maxextent));

  for(iy in y) {
    rx :=  cpa*(x-center[1]) + spa*(iy-center[2]);
    ry := -spa*(x-center[1]) + cpa*(iy-center[2]);
    r:=(rx/width[1])^2+(ry/width[2])^2;
    g:=height * exp(-r);
    a[x, iy, 1, ]+:=g;
  }
  return a;
}

addcomplist:=function(im, cl) {
  cs:=im.coordsys();
  ncomp:=cl.length();
  print "Processing ", ncomp, "components";
  pix:=im.getchunk();
  nx:=pix::shape[1];  
  ny:=pix::shape[2];
  dx:=180*60*60*abs(cs.increment()[1])/pi;

  for (icomp in 1:ncomp) {
    comp:=cl.component(icomp, T);
    dir:=cs.topixel([comp.shape.direction.m0.value,
		     comp.shape.direction.m1.value]);
    if((as_integer(dir[1])<1)||(as_integer(dir[1])>nx)||
       (as_integer(dir[2])<1)||(as_integer(dir[2])>ny)) {
      print 'Component ', icomp, ' is off the grid: ',
	  comp.shape.direction.m0.value,
	  comp.shape.direction.m1.value, dir
    }
    else {
      if(comp.shape.type=='Point') {
	pix[as_integer(dir[1]), as_integer(dir[2]), , 1]+:=comp.flux.value;
    }
      else {
	bmaj:=dq.convert(comp.shape.majoraxis, 'arcsec').value/dx;
	bmin:=dq.convert(comp.shape.minoraxis, 'arcsec').value/dx;
	bpa :=dq.convert(comp.shape.positionangle, 'rad').value;
	pix:=addgaussian2d(pix, comp.flux.value[1], dir,
			   [bmaj, bmin], bpa);
      }
    }
  }
  return im.putchunk(pix);
}
