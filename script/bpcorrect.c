/* Bad pixel correction for IRSF */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "fitsio.h"

/* Image Size */
#define LINE1PIX 1024
#define LINE2PIX 1024 

#define BADPIXV -32000. // from domask

/* print cfitsio errors */
void printerror(int status);

int main(int argc, char *argv[]){

  fitsfile *fp, *fp0;
  int i, j, status, nfound, anynull;
  long fpixel, nelements;
  long naxes[2], nbuffer;
  float nullval; 
  float buffer[LINE1PIX];

  int kp,km,lp,lm;
  float rxp,rxm,ryp,rym,rr;

  int refx[3]={865,425,460}, refy[3]={980,790,655};
  float bpval;

  float *z;
  float *zn;

  //fprintf(stderr,"OK\n");
  if(argc!=3){
    printf("Usage: %s input output\n", argv[0]);
    exit(1);
    }

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

  /* Check Bad pixel Value */
  if (strncmp(argv[1]+strlen(argv[1])-11, "j", 1)==0){
    bpval=z[refx[0]*LINE2PIX+refy[0]];
  } else if (strncmp(argv[1]+strlen(argv[1])-11, "h", 1)==0){
    bpval=z[refx[1]*LINE2PIX+refy[1]];
  } else if (strncmp(argv[1]+strlen(argv[1])-11, "k", 1)==0){
    bpval=z[refx[2]*LINE2PIX+refy[2]];
  } else {
    bpval=BADPIXV;
  }

  fprintf(stderr, "badpixel value = %.2f\n", bpval);

  /* badpixel correction */
  /* NOTE: the edges are not corrected */
  for (i = 1; i < LINE2PIX -1; i++){
    for (j = 1; j < LINE1PIX -1; j++){
      if (z[j*LINE2PIX+i]==bpval){
	/* x +direction */
	for (kp=j; kp < LINE1PIX; kp++){
	  if (z[kp*LINE2PIX+i]!=bpval) break;
	}
	/* x -direction */
	for (km=j; km >= 0; km--){
	  if (z[km*LINE2PIX+i]!=bpval) break;
	}
	/* y +direction */
	for (lp=i; lp < LINE2PIX; lp++){
	  if (z[j*LINE2PIX+lp]!=bpval) break;
	}
	/* y -direction */
	for (lm=i; lm >=0 ; lm--){
	  if (z[j*LINE2PIX+lm]!=bpval) break;
	}
	/* derive weight */
	rxp=kp-j;
	rxm=j-km;
	ryp=lp-i;
	rym=i-lm;

	rr=1./(1./rxp + 1./rxm + 1./ryp + 1./rym);

	rxp=rr/rxp;
	rxm=rr/rxm;
	ryp=rr/ryp;
	rym=rr/rym;
	
	zn[j*LINE2PIX+i] = rxp*z[kp*LINE2PIX+i] + rxm*z[km*LINE2PIX+i] + 
	  ryp*z[j*LINE2PIX+lp] + rym*z[j*LINE2PIX+km];
      } else {
	zn[j*LINE2PIX+i]=z[j*LINE2PIX+i];
      }
    }
  }


  ///  fprintf(stderr,"\nOK\n");

  fprintf(stderr,"\nWrite %s ... ",argv[2]);
  status=0;
  if (fits_create_file(&fp, argv[2], &status))
    printerror(status);
  
  if (fits_open_file(&fp0, argv[1], READONLY, &status))
    printerror(status);
  if (fits_copy_header(fp0, fp, &status))
    printerror(status);
  if (fits_close_file(fp0, &status))
    printerror(status);

  fpixel=1;
  fprintf(stderr,"%d x %d format ...",LINE1PIX,LINE2PIX);
  nelements=LINE1PIX;
  for(i=0;i<LINE2PIX;i++){
    for(j=0;j<LINE1PIX;j++) buffer[j]=zn[j*LINE2PIX+i];
    //    for(j=0;j<LINE1PIXO;j++) buffer[j]=z1[j][i];
    if(fits_write_img(fp, TFLOAT, fpixel, nelements, buffer, &status))
      printerror(status);
    fpixel+=LINE1PIX;
  }

  if (fits_close_file(fp, &status))
    printerror(status);

  fprintf(stderr,"Done.\n");


  return 0;
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
