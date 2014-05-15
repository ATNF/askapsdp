#!/usr/bin/env python

import askap.analysis.data
from numpy import *
import MySQLdb
from optparse import OptionParser

SKADS_host = "gijane.atnf.csiro.au"
SKADS_user = "skads"
SKADS_pass = "skads"
SKADS_db = "S3SEX"


if __name__ == '__main__':

    parser = OptionParser()
    parser.add_option("-H","--Host", dest="host", default=SKADS_host, help="Database host [default: %default]")
    parser.add_option("-u","--user", dest="user", default=SKADS_user, help="Database user [default: %default]")
    parser.add_option("-p","--password", dest="password", default=SKADS_pass, help="Database password [default: %default]")
    parser.add_option("-d","--db", dest="db", default=SKADS_db, help="Database name [default: %default]")
    parser.add_option("-t","--table", dest="table", default="Galaxies", help="Table name [default: %default]")
    parser.add_option("-f","--file", dest="infile", default="/exported2/SKADS/hi/catalog_simon_z1_galaxies_ascii.dat", help="ASCII file to send to database [default: %default]")

    (options, args) = parser.parse_args()
        
    dbHandle = MySQLdb.connect(host=options.host, user=options.user, passwd=options.password, db=options.db)
    cursor = dbHandle.cursor()
    
    cursor.execute("DROP TABLE IF EXISTS %s"%options.table)

    columns = """
galaxy 
cluster              
galaxyid
clusterID               
box
hubbletype
right_ascension
declination
distance
zapparent
himass
h2mass
hiintflux
cointflux_1
cointflux_2
cointflux_3
cointflux_4
cointflux_5
cointflux_6
cointflux_7
cointflux_8
cointflux_9
cointflux_10
diskpositionangle
diskinclination
gasscaleradius
rmolc
hiaxisratio
himajoraxis_msunpc
himajoraxis_max
himajoraxis_50max
himajoraxis_10max
himajoraxis_halfmass
h2axisratio
h2majoraxis_msunpc
h2majoraxis_50max
h2majoraxis_10max
h2majoraxis_halfmass
balancemajoraxis
hilumcenter
hilumpeak
hiwidthpeak
hiwidth50
hiwidth20
columcenter
columpeak
cowidthpeak
cowidth50
cowidth20
cofillingfactor
""".split()
    
    createTable="""
CREATE TABLE Galaxies (
    `galaxy` INT(10) UNSIGNED NOT NULL, 
    `cluster` INT(10) UNSIGNED NOT NULL,  
    `galaxyid` CHAR(20) NOT NULL,  
    `clusterid` CHAR(20) NOT NULL,  
    `box` INT(10) UNSIGNED NOT NULL,  
    `hubbletype` DOUBLE NOT NULL,  
    `right_ascension` DOUBLE NOT NULL COMMENT 'degrees', 
    `declination` DOUBLE NOT NULL COMMENT 'degrees', 
    `distance` DOUBLE NOT NULL , 
    `zapparent` DOUBLE NOT NULL, 
    `himass` FLOAT NOT NULL COMMENT 'Msun', 
    `h2mass` FLOAT NOT NULL COMMENT 'Msun', 
    `hiintflux` FLOAT NOT NULL COMMENT 'Jy km/s', 
    `cointflux_1` FLOAT NOT NULL COMMENT 'Jy km/s', 
    `cointflux_2` FLOAT NOT NULL COMMENT 'Jy km/s', 
    `cointflux_3` FLOAT NOT NULL COMMENT 'Jy km/s', 
    `cointflux_4` FLOAT NOT NULL COMMENT 'Jy km/s', 
    `cointflux_5` FLOAT NOT NULL COMMENT 'Jy km/s', 
    `cointflux_6` FLOAT NOT NULL COMMENT 'Jy km/s', 
    `cointflux_7` FLOAT NOT NULL COMMENT 'Jy km/s', 
    `cointflux_8` FLOAT NOT NULL COMMENT 'Jy km/s', 
    `cointflux_9` FLOAT NOT NULL COMMENT 'Jy km/s', 
    `cointflux_10` FLOAT NOT NULL COMMENT 'Jy km/s', 
    `diskpositionangle` FLOAT NOT NULL COMMENT 'rad', 
    `diskinclination` FLOAT NOT NULL COMMENT 'rad', 
    `gasscaleradius` FLOAT NOT NULL COMMENT 'kpc', 
    `rmolc` FLOAT NOT NULL, 
    `hiaxisratio` FLOAT NOT NULL, 
    `himajoraxis_msunpc` FLOAT NOT NULL COMMENT 'arcsec', 
    `himajoraxis_max` FLOAT NOT NULL COMMENT 'arcsec', 
    `himajoraxis_50max` FLOAT NOT NULL COMMENT 'arcsec', 
    `himajoraxis_10max` FLOAT NOT NULL COMMENT 'arcsec', 
    `himajoraxis_halfmass` FLOAT NOT NULL COMMENT 'arcsec', 
    `h2axisratio` FLOAT NOT NULL, 
    `h2majoraxis_msunpc` FLOAT NOT NULL COMMENT 'arcsec', 
    `h2majoraxis_50max` FLOAT NOT NULL COMMENT 'arcsec', 
    `h2majoraxis_10max` FLOAT NOT NULL COMMENT 'arcsec', 
    `h2majoraxis_halfmass` FLOAT NOT NULL COMMENT 'arcsec', 
    `balancemajoraxis` FLOAT NOT NULL COMMENT 'arcsec', 
    `hilumcenter` FLOAT NOT NULL COMMENT 's/km', 
    `hilumpeak` FLOAT NOT NULL COMMENT 's/km', 
    `hiwidthpeak` FLOAT NOT NULL COMMENT 'km/s', 
    `hiwidth50` FLOAT NOT NULL COMMENT 'km/s', 
    `hiwidth20` FLOAT NOT NULL COMMENT 'km/s', 
    `columcenter` FLOAT NOT NULL COMMENT 's/km', 
    `columpeak` FLOAT NOT NULL COMMENT 's/km', 
    `cowidthpeak` FLOAT NOT NULL COMMENT 'km/s', 
    `cowidth50` FLOAT NOT NULL COMMENT 'km/s', 
    `cowidth20` FLOAT NOT NULL COMMENT 'km/s', 
    `cofillingfactor` FLOAT NOT NULL, 
     PRIMARY KEY (`galaxy`)) ENGINE=MyISAM DEFAULT CHARSET=latin1;
"""
    cursor.execute(createTable)

    inputFile = file(options.infile, "r")
    
    for line in inputFile:

        inputs = line[:-1].split()
        
        inputline = "INSERT INTO %s (%s"%(options.table,columns[0])
        for i in range(1,len(columns)):
            inputline += ",%s"%columns[i]
        inputline += ") VALUES (%s"%inputs[0]
        for i in range(1,len(columns)):
            inputline += ",%s"%inputs[i]
        inputline += ")"

        cursor.execute(inputline)


    inputFile.close()

    
