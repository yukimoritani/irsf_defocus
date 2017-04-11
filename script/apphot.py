#!/usr/bin/env python

#fits...set your fitsname
#outfile...name of coo file
#mag_1...magfile(phot)

from pyraf import iraf
from iraf import noao
from iraf import digiphot
from iraf import apphot
from iraf import images
import numpy as np
import os
import sys

def SetIRAFRar():

    # datamax_datamin
    iraf.apphot.datapars.datamax=33000
    iraf.apphot.datapars.datamin=0
 
    # set param
    iraf.apphot.datapars.readnois=30
    iraf.apphot.datapars.epadu=5
 
    # fwhm
    #iraf.apphot.datapars.fwhm=3
    #iraf.apphot.datapars.itime=30
 
    # utility 
    iraf.apphot.phot.interactive="no"
    iraf.apphot.phot.verify="no"
    iraf.apphot.phot.verbose="no"
    iraf.apphot.photpars.zmag=21
 
    # imstatics
    iraf.imstat.fields="midpt,stddev"
    iraf.imstat.lower="-1000"
    iraf.imstat.upper="30000"
    iraf.imstat.nclip=10
    iraf.imstat.lsigma=3
    iraf.imstat.usigma=3
    iraf.imstat.binwidth=0.1
    iraf.imstat.format="yes"
    iraf.imstat.cache="yes"


def Photometry(setn, flist, band):
    # J-band
    os.system("head -1 -q %s/xylist/%sf*pos | awk '{print $3}'> tmp" % (setn, band))  
    ff=open("tmp")
    fr=ff.readlines()
    ff.close
    r=[]
    for x in fr:
        r.append(float(x))
    radi=np.nanmean(r)
    print s, band, radi

    # aperture,annulus,dannulus
    iraf.apphot.photpars.apertures=radi
    iraf.apphot.fitskypars.annulus=radi+4
    iraf.apphot.fitskypars.dannulus=10

    for x in flist:
        if x[0] in s:
            infile="%s/xylist/%sf%s.pos" % (setn, band,x[1])
            outfile="%s/photometry/%sf%s.mag" % (setn, band, x[1])     
            dumped="%s/photometry/%sf%s.txt" % (setn, band, x[1])
            fits="%s/%sf%smasked.fits" % (setn, band, x[1])
            print infile,fits
            iraf.phot(fits,coords=infile,output=outfile,wcsin="physical")
            iraf.txdump(outfile,fields="xc,yc,mag,merr,flux,msky",expr="yes",Stdout=dumped)
 

if __name__ == '__main__':

    iraf.unlearn("apphot")

    SetIRAFRar()

    #fl=os.listdir("./")
    objlist="obslist"
    f=open(objlist)
    l=f.readlines()
    f.close

    fl=[]
    fs=[]
    for i in l:
        ll=i.split()
        fl.append([ ll[1] , ll[2][-9:-5] ])
        fs.append(ll[1])
    #print fl
    fs0=list(set(fs))
    #print fs0

    for s in fs0:
        if not os.path.exists("./%s/photometry" % s):
            os.mkdir("./%s/photometry" % s)

        Photometry(s,fl,"j")
        Photometry(s,fl,"h")
        Photometry(s,fl,"k")

