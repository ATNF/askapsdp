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

outfileid=file('SKA1AA.in', 'w')
outfileid.write('antennas.telescope = SKA1\n')
outfileid.write('antennas.SKA1.coordinates = global\n')
outfileid.write('antennas.SKA1.names = [AA1,AA2,AA3,AA4,AA5,AA6,AA7,AA8,AA9,AA10,AA11,AA12,AA13,AA14,AA15,AA16,AA17,AA18,AA19,AA20,AA21,AA22,AA23,AA24,AA25,AA26,AA27,AA28,AA29,AA30,AA31,AA32,AA33,AA34,AA35,AA36,AA37,AA38,AA39,AA40,AA41,AA42,AA43,AA44,AA45,AA46,AA47,AA48,AA49,AA50]\n')
outfileid.write('antennas.SKA1.diameter = 15m\n')
outfileid.write('antennas.SKA1.scale = 1.0\n')
outfileid.write('antennas.SKA1.mount = alt-az\n')

fileid=file('aa.csv', 'U')
csr=csv.reader(fileid)
id=1
for row in csr:
    lon=float(row[1])
    lat=float(row[2])
    el=300.0
    outfileid.write('# lat: %s; long: %s; el: %s\n' % (lat, lon, el))
    (xx, yy, zz) = WGS84ToITRF(lat*math.pi/180.0, lon*math.pi/180.0, el)
    outfileid.write('antennas.SKA1.AA%d=[%s, %s, %s]\n' % (id, xx, yy, zz))
    id=id+1

fileid.close()
outfileid.close()
