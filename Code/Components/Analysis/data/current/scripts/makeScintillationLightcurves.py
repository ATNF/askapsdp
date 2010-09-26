#!/usr/bin/env python

import askap.analysis.data

import numpy as np
from numpy import *
import scipy
import scipy.stats
import scipy.fftpack
import matplotlib.pyplot as plt
from optparse import OptionParser

# One term of the series expansion in calculating L_-7/6
def lagterm(x,m):
    return (-1)**m * x**m * scipy.special.gamma(7./6.+m) / (scipy.special.gamma(7./6.)*scipy.special.gamma(1+m)**2)

# Calculating the Laguerre polynomial L_-7/6
def lag_7on6(x):
    if(x>=20):
        return (x**(-7./6.) + (49./36.)*x**(-13./6.) + (8281./2592.)*x**(-19./6.))/scipy.special.gamma(-1./6.)
    else:
        m=0
        delta = lagterm(x,m)
        sumdelt = delta
        while ( abs(delta) > 1.e-6 ):
            m = m + 1
            delta = lagterm(x,m)
            sumdelt = sumdelt + delta
        return sumdelt

def lag_7on6_many(x):
    size = x.size
    result = array([])
    result.resize(size)
    for i in range(size):
        result[i] = lag_7on6(x[i])
    return result
    
# Calculating size from flux, assuming brightness temp of 10^12 K
def flux2size(flux,lam=0.21):
    k=1.3807e-23
    T=1.e12
    return sqrt(lam**2 * flux * 1.e-26 / (2.*pi*k*T))

if __name__ == '__main__':

    np.random.seed()

    parser = OptionParser()
    parser.add_option("-i","--inputs", dest="inputfile", default="", help="Input catalogue file [default: %default]")
    parser.add_option("-o","--outputs", dest="outputfile", default="", help="Output catalogue file [default: %default]")
    parser.add_option("-s","--sample", type="int", dest="step", default=60, help="Sampling interval [s] [default: %default]")
    parser.add_option("-x","--xmax", type="int", dest="xmax", default=75, help="Maximum lag for autocovariance (dimensionless units) [default: %default]")
    parser.add_option("-c","--compCol", type="int", dest="componentLoc", default=0, help="Column in input file with component ID number [default: %default]")
    parser.add_option("-f","--fluxCol", type="int", dest="fluxLoc", default=11, help="Column in input file with flux [default: %default]")

    (options, args) = parser.parse_args()

    if(options.inputfile==''):
        print "No catalogue given! Exiting."
        exit(1)
    elif(not os.path.exists(options.inputfile)):
        print "Input file %s does not exist! Exiting."
        exit(1)

    # Fiducial constants used in scaling
    viss = 1.e5
    sm=9.8e15
    z=1.2e19
    s0=6.4e7
    lam=300./1400.
    r_e = 2.82e-15
    k = 2.*pi/lam

    # Small scale calculation & FFT
    scaleSmall = 2.**(10./3.)*pi**2 * r_e**2 * lam**2 * sm * scipy.special.gamma(7./6.) *s0**2 * (k**2*s0**2/z**2)**(1./6.)
    xfactorSmall = k*s0*viss / z 
    nstepSmall = int(options.xmax/xfactorSmall/options.step + 1)
    nsampSmall = nstepSmall*options.step + 1
    tauSmall = double(array(range(-nsampSmall,nsampSmall,options.step)))
    x = xfactorSmall * tau
    ySmall = scaleSmall*lag_7on6_many(x**2)
    absFySmall = sqrt(abs(scipy.fftpack.fft(ySmall/float(ySmall.size))))

    infile=open(options.inputfile,'r')
    outfile=open(outputfile,'w')
    
    for s in infile:
        line = s.split()
        if(s[0]!='#'):

            component = line[options.componentLoc]
            logflux = line[options.fluxLoc]

            theta0 = flux2size(10**logflux)

            fy=array()
            if(theta0<1./(k*s0) ):
                fysize=nstepSmall
                ysize=ySmall.size
                fy=absFySmall.copy()
            else:
                scale = 2.*pi**2 * r_e**2 * lam**2 * sm * scipy.special.gamma(7./6.) * z**2 / (2 * pi / lam)**2 / (z**2 * theta0**2)**(7./6.)
                xfactor = viss / (2. * z * theta0)
                n_step = int(options.xmax/xfactor/options.step + 1)
                n_samp = n_step*options.step + 1
                tau = double(array(range(-n_samp,n_samp,options.step)))
                x = xfactor * tau
                y = scale*lag_7on6_many(x**2)
                fy=sqrt(abs(scipy.fftpack.fft(y/float(y.size))))
                ysize=y.size
                size=n_step

            np.random.seed()
            phases=array(np.random.random(size+1)*2*pi)
            cphases=cos(phases) + sin(phases)*1j

            newy = array(range(2*size+1),dtype=complex)
            newy[0:size+1] = abs_fy[0:size+1] * cphases
            newy[size+1:] = abs_fy[size+1:]*cphases.conjugate()[:0:-1]
            delta = scipy.fftpack.ifft(newy*float(ysize)).real
            newflux = pow(10,f)*(1 + delta)

            outfile.write(component+"  ")
            for f in newflux:
                outfile.write("%12.8f "%f)
            outfile.write("\n")
