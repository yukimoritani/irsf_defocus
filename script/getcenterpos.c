/* Bad pixel correction for IRSF */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "fitsio.h"

/* Image Size */
#define LINE1PIX 1024
#define LINE2PIX 1024 

/* print cfitsio errors */
void printerror(int status);

/*Calc backgrond level*/
float EstimateSky(float *z, int nite, int nx, int ny, float x0, float y0, float rr);
/* Replace pixel */
void ReplPix(float *z, float *z2, float thres, int nx, int ny);
/* Calc Gravity Centre */
void CalcGCStack(float *z, float *x, float *y, int nx, int ny, int ww);
/* Make radial profile */
float MeasureRadius(float *z1, int nx, int ny, int np, float cx, float cy);

/*distance*/
float Distance(float x1, float y1, float x2, float y2);


int main(int argc, char *argv[]){

  fitsfile *fp0;
  int i, j, status, nfound, anynull;
  long fpixel;
  long naxes[2], nbuffer;
  float nullval; 
  float buffer[LINE1PIX];

  float x0, y0, thres, radi;

  float *z, *zn;
  int ww;

  //fprintf(stderr,"OK\n");
  if(argc!=5){
    printf("Usage: %s input x y w\n", argv[0]);
    printf("This program seaches centroid in 2*w X 2*w box around (x,y)\n");
    exit(1);
    }

  sscanf(argv[2],"%f", &x0);
  sscanf(argv[3],"%f", &y0);
  sscanf(argv[4],"%d", &ww);


  z = (float*) malloc(sizeof(float) * LINE2PIX * LINE1PIX);
  zn = (float*) malloc(sizeof(float) * LINE2PIX * LINE1PIX);

  /*Read inputfile*/
  fprintf(stderr,"Reading %s  ... ", argv[1]);
  status=0;
  if(fits_open_file(&fp0, argv[1], READONLY, &status))
    printerror(status);
  if(fits_read_keys_lng(fp0, "NAXIS", 1, 2, naxes, &nfound, &status)) 
    printerror(status);
  //fprintf(stderr,"%ld x %ld format \n",naxes[0],naxes[1]);
  if((naxes[0]!=LINE1PIX)||(naxes[1]!=LINE2PIX)){
    fprintf(stderr,"\nIllegal image format\n");
    free(z);
    free(zn);
    exit(0);
  }
  
  //  fprintf(stderr,"OK1\n");
  fpixel = 1;
  nullval = 0;
  nbuffer = naxes[0];
  for(i=0;i<naxes[1];i++){
    //  fprintf(stderr,"OK2\n");
    if (fits_read_img(fp0, TFLOAT, fpixel, nbuffer, &nullval, buffer, &anynull, &status))
      printerror(status);
    for(j=0;j<naxes[0];j++) z[j*LINE2PIX+i]=buffer[j];
    fpixel+=naxes[0];
  }
  if(fits_close_file(fp0, &status))
    printerror(status);
  fprintf(stderr,"Succeed.\n");


  /* Measure background level around (x,y): r=3*ww */
  thres = EstimateSky(z, 3, LINE1PIX, LINE2PIX, x0, y0, (float)(3*ww));
  fprintf(stderr, "threshod: %e\n", thres);

  /* Replace to 1/0 according to threshod */
  ReplPix(z, zn, thres, LINE1PIX, LINE2PIX);

  /* Measure Ccentre of glavity */
  CalcGCStack(zn, &x0, &y0, LINE1PIX, LINE2PIX, ww);

  radi=MeasureRadius(z, LINE1PIX, LINE2PIX, ww*2, x0, y0);

  fprintf(stderr,"Centre: x= %.3f , y = %.3f\n", x0, y0);
  fprintf(stderr,"Radius: %.2f\n", radi);
  fprintf(stdout,"%.3f %.3f %.2f\n", x0, y0, radi);
		     



  free(z);
  free(zn);




  return 0;
}

/*Calc backgrond level*/
float EstimateSky(float *z, int nite, int nx, int ny, float x0, float y0, float rr){

  float sky;
  float mean=0., mean1=0., stddev=1e+6, zz=0.;
  int i,j,k,num=0;

  for(k=0; k<nite; k++){
    mean1=0.;
    zz=0.;
    num=0;
    for(i=0; i<nx; i++){
      for(j=0; j<ny; j++){
	if(Distance((float)i, (float)j, x0, y0) > rr) continue;
        //fprintf(stderr,"%d %d--", i, j);
        /* only use values lower than mean + 1.*sigma*/
        if(z[i*ny+j]<=(mean+stddev)){
          mean1+=z[i*ny+j];
          zz+=z[i*ny+j]*z[i*ny+j];
          num++;
        }
      }
    }


    mean = mean1/(float)num;
    if ((zz/(float)num - mean*mean)<0) break;
    else{
     stddev = sqrt(zz/(float)num - mean*mean); 
     //fprintf(stderr, "%e, %e\n", mean, stddev);
    }
  }

  /* finally, sky is determined as mean + 2*stddev */
  //sky=mean+1.*stddev;
  sky = mean + 2*stddev;
  //sky=mean+20*stddev;
  
  return sky;
  
}

/* Making new image array of  1/0                             */
/* If value is higher/lower than threshod, the value is 1/0.  */
void ReplPix(float *z, float *z2, float thres, int nx, int ny){

  int i,j;

  for(i=0; i<nx; i++){
    for(j=0; j<ny; j++){
      if(z[i*ny+j]>thres){
        z2[i*ny+j]=1.;
        //z2[i][j]=z[i][j];
      }else{
        z2[i*ny+j]=0.;
      }
    }
  }

}

/* Calc Gravity Centre */
void CalcGCStack(float *z, float *x, float *y, int nx, int ny, int ww){


  int i,j, nel;
  int xc, yc;

  xc = (int)*x;
  yc = (int)*y;

  *x = 0.;
  *y = 0.;
  nel = 0;

  /* projec the ring to x.y axis */
  
  for(i=xc-ww; i<xc+ww; i++){
    if(i<0 || i >=nx)continue;
    for(j=yc-ww; j<yc+ww; j++){
      if(j<0 || i >=ny)continue;
      if (z[i*ny+j] == 1) {
        *x += (float)i;
        *y += (float)j;
	nel++;
      }
    }
  }

  if (nel >0){
   //fprintf(stderr, "%d\n", nel);
    *x /= (float)nel;
    *y /= (float)nel;
  }
}


/* Make radial profile */
float MeasureRadius(float *z1, int nx, int ny, int np, float cx, float cy){

  int index, nel[np], nn=0;
  int i, j, peaki;
  float dist;
  float prof[np], peak=0., radius=0., back=0.;
  float ratio=0.01, dd=0.005;


  for(i=0; i < np; i++){
    nel[i]=0;
    prof[i]=0.;
  }

  for(i=0;i<nx;i++){
    for(j=0;j<nx;j++){
      dist = Distance((float)i, (float)j, cx, cy);
      if( (dist-floorf(dist))< 0.5){
        index = (int)floorf(dist);
      } else{
        index = (int)ceilf(dist);
      }
      //fprintf(stderr,"%d ", index);
      if(index < np){
        prof[index]+=z1[i*ny+j];
        nel[index]++;
      }
    }
  }

  for(i=0; i < np; i++){
    prof[i] /= (float)nel[i];
  }

  for(i=(int)((float)np*0.8); i < np; i++){
    back+=prof[i];
    nn++;
  }

  back /= (float)nn;

  for(i=0; i < np; i++){
    prof[i]-=back;
    if (peak < prof[i]){
      peak=prof[i];
      peaki=i;
    }
  }

  for(i=0; i < np; i++){
    prof[i] /= peak;
    //fprintf(stdout, "%.3f\n", prof[i]);
  
  }

  nn=0;
  for(i=peaki;i<np;i++){
    if(fabsf(prof[i]-ratio)<dd){
      radius+=(float)i;
      //fprintf(stderr, "%d - ", i);
      nn++;
    }
  }

  radius /=(float)nn;

  return radius;

}


/* distance */
float Distance(float x1, float y1, float x2, float y2){

  float dist;
  float dx,dy;

  dx=x1-x2;
  dy=y1-y2;

  dist=sqrt(dx*dx+dy*dy);
  
  return dist;

}


/* Print out cfitsio error messages and exit program */
void printerror(int status) {

  char status_str[FLEN_STATUS], errmsg[FLEN_ERRMSG];
  
  if (status)
    fprintf(stderr, "\n*** Error occurred during program execution ***\n");
  
  fits_get_errstatus(status, status_str);   /* get the error description */
  fprintf(stderr, "\nstatus = %d: %s\n", status, status_str);

  /* get first message; null if stack is empty */
  if ( fits_read_errmsg(errmsg) ) {
    fprintf(stderr, "\nError message stack:\n");
    fprintf(stderr, " %s\n", errmsg);

    while ( fits_read_errmsg(errmsg) )  /* get remaining messages */
      fprintf(stderr, " %s\n", errmsg);
  }

  exit( status );       /* terminate the program, returning error status */
}
