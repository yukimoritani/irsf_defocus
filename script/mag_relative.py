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
import mag_mean

#ezero=0.0004
#photdir="photometry"

if __name__ == '__main__':

    f=open(sys.argv[1])
    l=f.readlines()
    f.close
    date_org=sys.argv[2]
    setn_org=sys.argv[3]
 
    print "#Origin: %s set n%s" % (date_org, setn_org)

    for i in l:
        ll=i.split()
        if ll[0] == date_org and ll[1] == setn_org:
            mag_org=map(float, ll[5::4])
            #print mag_org

    for i in l:
        ll=i.split()
        date=ll[0]
        setn=ll[1]
        mjd=ll[2]
        magt=map(float, ll[5::4])
        maget=map(float, ll[6::4])
        #print magt, maget
        if len(magt) != len(mag_org) or len(magt) != len(maget):
            print "Number of stars is different: %s set n%s" % (date, setn)
        else:
            # make list of residual from the origin
            mag_sub=[[y-x,z] for x,y,z in zip(mag_org,magt,maget)]
            #mag_sub=[]
            #for x,y,z in zip(mag_org,magt,maget):
            #    mag_sub.extend([y-x,z])
            # average reference mag: weighted mean
            mag_ref=mag_mean.WeightedMean(mag_sub[1:])
            #print mag_sub
            mag_target=mag_sub[0][0] - mag_ref[0]
            # mag error: std error of ref and IRAF mag error]
            #print a, np.var(mag_sub[1:], axis=0),np.power(mag_sub[0][1],2.)
            mage_target=np.sqrt(np.var(mag_sub[1:], axis=0)[0]/(len(mag_sub[1:])-1.)+np.power(mag_sub[0][1],2.))
        print "%s %s %s %.3f %.3f" % (date, setn, mjd, mag_target, mage_target)

