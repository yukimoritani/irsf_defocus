#!/usr/bin/env python

import os
import sys
import numpy

def Mask(frameid, band, indir, outdir):
    ## J-band
    infile=indir+band+"f"+frameid+".fits"
    outfile=outdir+band+"/f"+frameid+"masked.fits"
    #print infile,outfile
    os.system("bpcorrect %s %s" % (infile,outfile))
    
if __name__ == '__main__':

    listfile="obslist"

    f=open(listfile)
    line = f.readlines()
    f.close

    for l in line:
        ll=l.split()
        if not os.path.exists("./%s" % ll[1]):
            os.mkdir("./%s" % ll[1])
        frameid=ll[2][-9:-5]
        indir=ll[2][:-11]
        #print ll[2],frameid,indir

        Mask(frameid, "j", indir, ll[1])
        Mask(frameid, "h", indir, ll[1])
        Mask(frameid, "k", indir, ll[1])
