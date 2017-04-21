#!/usr/bin/env python

import matplotlib.pyplot as plt
import numpy as np
import os
import os.path
import sys
import glob


from pyraf import iraf
from iraf import images
from iraf import imutil
#from iraf import stsdas

ezero=0.0004
photdir="photometry"

def WeightedMean(magl):
    #print magl
    #mag, emag = [np.sum(x/y/y)/np.sum(1./y/y),1./np.sum(1./y/y) for x,y in magl]
    msig=[]
    sig=[]
    for x,y in magl:
        msig.append(x/y/y)
        sig.append(1./y/y)
    mag=np.sum(msig)/np.sum(sig)
    emag=np.sqrt(1./np.sum(sig))
    return [mag,emag]

def ReadMJD(infile):
    filel=infile.replace(".txt","masked.fits").replace("/"+photdir,"")
    #print filel
    t=iraf.hselect(filel,fields="MJD",expr="yes",Stdout=1) 
    #print t
    return float(t[0])

def AveradeMag(indir, band, nstar, ignore):
    fl=glob.glob(indir+"/*")
    bandfind = band+"f"
    m=[]
    px=[]
    py=[]
    t=[]
    result=[]
    for i in fl:
        #print i
        path,ext = os.path.splitext(i)
        if ext == ".txt" and bandfind in path:
            frameid=i[-8:-4]
            if frameid in ignore: continue
            #print i
            f=open(i)
            l=f.readlines()
            f.close
            for x in l:
                #print x
                ll=x.split()
                m.append([float(ll[2]), float(ll[3]) if float(ll[3])!=0.0 else ezero])
                px.append(float(ll[0]))
                py.append(float(ll[1]))
            t.append(ReadMJD(i))
    #print m
    tt=t[0::nstar]
    tm=np.mean(tt)
    result.append(tm)
    for i in range(nstar):
        mag=m[i::nstar]
        posx=px[i::nstar]
        posy=py[i::nstar]
        #print mag, len(mag)
        magm=WeightedMean(mag)
        result.append([np.mean(posx),np.mean(posy),magm[0],magm[1]])
        #result.append(WeightedMean(mag))
    #print tm,posm,magm
    #result.append([tm,posm,magm])
    return result

# Usage mag.mean.py band nstar
if __name__ == '__main__':

    f=open(sys.argv[1])
    l=f.readlines()
    f.close
    band=sys.argv[2]

    nstar=int(sys.argv[3])
    magj=[]
    for i in l:
        ll=i.split()
        ignore=ll[2:]
        indir=os.path.join(ll[0],"n"+ll[1],photdir)
        #nstar is target + references
        magj=(AveradeMag(indir, band, nstar, ignore))
        print "%s %s %.5f " % (ll[0], ll[1], magj[0]),
        for j in range(nstar):
            print "%.3f %.3f %.4f %.4f" % (magj[j+1][0], magj[j+1][1], magj[j+1][2], magj[j+1][3]),
        print ""
