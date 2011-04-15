csvfile='aa.csv'
outfile='SKA1AA100m.in'
dmax=100.0
lonref=40.019
latref=-30.7131

import math
import csv

sm_a = 6378137.0
invf = 298.257223563
f = 1.0 / invf

# Convert WGS-84 to ITRF
# lat and lon are the latitude and longitude in radians, h is the height in metres.
def WGS84ToITRF (lat, lon, h):
    SINK = math.sin(lat)
    COSK = math.cos(lat)
    e2 = 2.0 * f - f * f
    v = sm_a / math.sqrt(1.0 - e2 * SINK * SINK)
    x = (v + h) * COSK * math.cos(lon)
    y = (v + h) * COSK * math.sin(lon)
    z = ((1 - e2) * v + h) * SINK
    return x, y, z

def distance(lat, lon, latref, longref):
    return math.sqrt((lat-latref)*(lat-latref)+(lon-lonref)*(lon-lonref))*17687.5

ants=[]

fileid=file(csvfile, 'U')
n=0
while(True):
    line=fileid.readline()
    if(line==''):
        break;
#    ants.append(int(line.split(',')[0]))
    line=line.split(',')
    lon=float(line[1])
    lat=float(line[2])
    if(distance(lat, lon, latref, lonref)<dmax):
        n=n+1
        ants.append(n)
fileid.close()

print ants

s='antennas.SKA1.names = ['
for i in range(n):
    if(i<n-1):
        s=s+'AA%d,'%ants[i]
    else:
        s=s+'AA%d'%ants[i]

s=s+']\n'

print "Read %d lines" % n

outfileid=file(outfile, 'w')
outfileid.write('antennas.telescope = SKA1\n')
outfileid.write('antennas.SKA1.coordinates = global\n')
outfileid.write(s)
outfileid.write('antennas.SKA1.diameter = 15m\n')
outfileid.write('antennas.SKA1.scale = 1.0\n')
outfileid.write('antennas.SKA1.mount = alt-az\n')

fileid=file(csvfile, 'U')
csr=csv.reader(fileid)
ant=1
for row in csr:
    lon=float(row[1])
    lat=float(row[2])
    el=300.0
    if(distance(lat, lon, latref, lonref)<dmax):
        outfileid.write('# lat: %s; long: %s; el: %s\n' % (lat, lon, el))
        (xx, yy, zz) = WGS84ToITRF(lat*math.pi/180.0, lon*math.pi/180.0, el)
        outfileid.write('antennas.SKA1.AA%d=[%s, %s, %s]\n' % (ant, xx, yy, zz))
    ant=ant+1

fileid.close()
outfileid.close()
