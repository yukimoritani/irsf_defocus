#!/usr/bin/env python

import os
import sys
import numpy

listfile="obslist"
listxydir="xylist"
masterpos="poslist"


def ReadPosFile(band):
    f=open(band+masterpos)
    line = f.readlines()
    f.close
    pos=[]
    for l in line:
       pos.append(l.split())
       #print pos
    return pos

def GetCenterPos(frameid, band, pos, indir, outdir):
    infile=indir+band+"f"+frameid+"masked.fits"
    outfile=outdir+band+"f"+frameid+".pos"
    #print infile,outfile
    for p in pos:
        os.system("getcenterpos %s %s %s %s >> %s" % (infile,p[0],p[1],p[2],outfile))


if __name__ == '__main__':

    # Read position list
    jpos=ReadPosFile("j")
    hpos=ReadPosFile("h")
    kpos=ReadPosFile("k")

    # open object list
    f=open(listfile)
    line = f.readlines()
    f.close

    for l in line:
        ll=l.split()
        if not os.path.exists("./%s/%s" % (ll[1],listxydir)):
            os.mkdir("./%s/%s" % (ll[1],listxydir))
        frameid=ll[2][-9:-5]
        indir=ll[1]+"/"
        outdir=ll[1]+"/"+listxydir+"/"
        #print ll[2],frameid,indir

        GetCenterPos(frameid, "j", jpos, indir, outdir)
        GetCenterPos(frameid, "h", jpos, indir, outdir)
        GetCenterPos(frameid, "k", jpos, indir, outdir)
