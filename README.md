# irsf_defocus
Scripts for analysis of IRSF defocued, pixel-fixed images. 

## script list
### mask.py
Badpixel correction for files listed in "objlist"

### bbcorrect.c
Badpixcel correction

### getcent.py 
Macro for measure centroid and spot radius of images in "objlist" for listed region.
Region is listed on "xposlist", where x is j, h or k.

### getcenterpos.c
Measure centroid and spot radius of each file.

### apphot.py
Aperture photometry

### mag_mean.py
Derive weighted-mean value of magniude measured by apphot.py

### mag_relative.py
Calculate relative magnitude.
