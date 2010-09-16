#!/usr/bin/env python

import askap.analysis.data
import numpy as np
from numpy import *
import scipy.stats
import scipy.fftpack
import matplotlib.pyplot as plt

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

    plt.figure(1, figsize=(11.7,11.7), dpi=72)

    # Fiducial constants used in scaling
    viss = 1.e5
    sm=9.8e15
    z=1.2e19
    s0=6.4e7
    lam=300./1400.
    r_e = 2.82e-15
    k = 2.*pi/lam

    n_samp = 1000001
    step = 2000
    n_step = n_samp / step

    # Log of the source fluxes to be tested, in Jy
    fluxes = range(0,-8,-2)
    plotf = -2

    for f in fluxes:

        theta0 = flux2size(pow(10,f))
        print f,theta0 * (180/pi) *3600.*1000, 1000./(k*s0)

        tau = double(array(range(-n_samp,n_samp,step)))

        if(theta0>1./(k*s0)):
            scale = 2.*pi**2 * r_e**2 * lam**2 * sm * scipy.special.gamma(7./6.) * z**2 / (2 * pi / lam)**2 / (z**2 * theta0**2)**(7./6.)
            x = viss * tau / (2. * z * theta0)
            print "Case 1"
        else:
            scale = 2.**(10./3.)*pi**2 * r_e**2 * lam**2 * sm * scipy.special.gamma(7./6.) *s0**2 * (k**2*s0**2/z**2)**(1./6.)
            x = k*s0*viss * tau / z 
            print "Case 2"
                     
        y = scale*lag_7on6_many(x**2)

        plt.subplot(221)
        plt.plot(y)
            
#        plt.subplot(222)
#        plt.plot(tau,y)

        # make a set of random phases
        np.random.seed()
        phases=array(np.random.random(n_step+1)*2*pi)
        cphases=cos(phases) + sin(phases)*1j

        # FT the autocovariance and take square root
        fy = scipy.fftpack.fft(y/float(y.size))
        abs_fy = sqrt(abs(fy))
        
        # apply phases, with complex conjugate in mirrored half
        newy = array(range(2*n_step+1),dtype=complex)
        newy[0:n_step+1] = abs_fy[0:n_step+1] * cphases
        newy[n_step+1:] = abs_fy[n_step+1:]*cphases.conjugate()[:0:-1]

        plt.subplot(222)
        plt.plot(newy)
            
        # inverse-FT back and scale to get the time-series
        delta = scipy.fftpack.ifft(newy*float(y.size)).real
        newflux = pow(10,f)*(1 + delta)

        # print some basic stats on the time series
        print f,theta0,mean(newflux),sqrt(var(newflux)),mean(delta),sqrt(var(delta)),"\n"

        LCfile = file("LC_F%d.dat"%f,"w")
        for i in range(tau.size):
            LCfile.write("%g %g %g %g\n"%(tau[i],y[i],delta[i],newflux[i]))
        LCfile.close()
        
        tautau =  double(range(tau.size))/(tau.size)*((tau[1]-tau[0])*tau.size)
        plt.subplot(223)
        plt.plot(tautau,newflux/pow(10,f))
        #plt.plot(tautau,newflux)
        #plt.plot(tautau,(newflux-mean(newflux))/sqrt(var(newflux)))
            #        plt.plot(tau,(newflux-mean(newflux))/sqrt(var(newflux)))
            #        plt.plot((newflux-mean(newflux))/sqrt(var(newflux)))
            #        plt.plot(t-mean(t))

        plt.subplot(224)
        #plt.plot(tautau[tautau<30000],newflux[tautau<30000])
        plt.plot(tautau[tautau<30000],newflux[tautau<30000]/pow(10,f))
        
    plt.savefig('lightcurves.png')
    plt.close()

