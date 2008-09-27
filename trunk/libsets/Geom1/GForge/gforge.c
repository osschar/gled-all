/*      --------------------------------------------------------------
	Gforge as a whole, and all routines not commented otherwise,
	is Copyright 1995 by John Beale  <beale@jumpjibe.stanford.edu>
	The crater code is Copyright 1995 by Heiko Eissfeldt
	<heiko@colossus.escape.de>  (see crater.c).

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

	-------------------------------------------------------------
	Much of the base gforge code was taken from "ppmforge.c" (a "fractal
	forgery generator") for the PBMPLUS toolkit. From those comments:

	ppmforge.c was originally designed and implemented in December of
	1989 by John Walker as a stand-alone program for the Sun and MS-DOS
	under Turbo C. It was adapted in September of 1991 for use with Jef
	Poskanzer's raster toolkit.

	Author of ppmforge.c:

	    John Walker
	    kelvin@fourmilab.ch
	    <http://www.fourmilab.ch/>
	    (current as of May 26 1995)

    References cited in the ppmforge comments are:

	    Foley, J. D., and Van Dam, A., Fundamentals of Interactive
		Computer  Graphics,  Reading,  Massachusetts:  Addison
		Wesley, 1984.

	    Peitgen, H.-O., and Saupe, D. eds., The Science Of Fractal
		Images, New York: Springer Verlag, 1988.

	    Press, W. H., Flannery, B. P., Teukolsky, S. A., Vetterling,
		W. T., Numerical Recipes In C, New Rochelle: Cambridge
		University Press, 1988.

*/


#include <stdlib.h>
#include <stdio.h>  
#include <time.h>
#include <math.h>
#include <string.h>
/*#include "pp.h" */

#include "gforge.h"
#include "crater.h" /* from Heiko Eissfeldt <heiko@colossus.escape.de> */
#include "fftn.h"

#include <IL/il.h>
#include <IL/ilu.h>

#define GVERSION "gforge v1.3a 18MAY96"
#define GAUTHOR "John Beale <beale@best.com>"

void f_filter(float *a, int n, double center, double Q, int sign);
void rescale(float **a, int meshsize, float min, float max);
void findpeak(float *a, int meshsize, int *imax, int *jmax);
void fac_warn(int mesh);

/*  Local variables  */

char output_filename[80];                /* output filename string */
char input_filename[80];
char *string;                         /* temp string */
static double arand, gaussadd, gaussfac; /* Gaussian random parameters */
static double fracdim;                /* Fractal dimension */
static double powscale;               /* Power law scaling exponent */
static int meshsize;                  /* FFT mesh size */
static int screenxsize, screenysize;  /* output image size in pixels */
static int xstart, ystart;            /* offset in data array for image */
static float xfrac, yfrac;            /* peak location in image */
static double dim[10], dscale[10];    /* 1/f powers and relative scales */
static int d_factors;                 /* number of 1/f powers defined */
static unsigned int rseed = 3;        /* Current random seed */

/*static image_data img; */          /* struct for info to file open */

static Boolean seedspec = FALSE;      /* Did the user specify a seed ? */
static Boolean peakspec = FALSE;      /* Specified fixed peak location ? */
static Boolean include_craters = FALSE;  /* add craters to surface */
static Boolean wrap = TRUE;           /* wrap craters around at edges */
static double c_density = 1.0;        /* surface crater density factor */
static double ch_scale = 1.0;         /* crater amplitude scaling factor */
static Boolean limitspec = FALSE;     /* set different limits? */
static double limit_hi = 1.0;
static double limit_lo = 0.0;        /* limits for scaling before pow() */
static Boolean bpspec = FALSE;       /* specify bandpass filter ? */
static double bpcenter, bpQ;         /* bandpass parameters */
static Boolean brspec = FALSE;       /* specify bandreject fileter ? */
static double brcenter, brQ;         /* bandreject parameters */
static Boolean hpspec = FALSE;       /* highpass filter ? */
static double hpcut, hporder;
static Boolean lpspec = FALSE;       /* lowpass filter ? */
static double lpcut, lporder;        /* low-pass cutoff frequency, order */


/* IL reporting */

void il_err(const char* s) {
  ILenum er = ilGetError();
  if(er) {
    printf("%s error: %s\n", iluErrorString(er));
  }
}
char* fmt2name(ILint fmt) {
  switch(fmt) {
  case IL_COLOR_INDEX: return "COLOR_INDEX";
  case IL_RGB: return "RGB";
  case IL_RGBA: return "RGBA";
  case IL_BGR: return "BGR";
  case IL_BGRA: return "BGRA";
  case IL_LUMINANCE: return "LUMINANCE";
  case IL_LUMINANCE_ALPHA: return "LUMINANCE_ALPHA";
  default: return "unknown";
  }
}
char* typ2name(ILint typ) {
  switch(typ) {
  case IL_BYTE: return "BYTE";
  case IL_UNSIGNED_BYTE: return "UNSIGNED_BYTE";
  case IL_SHORT: return "SHORT";
  case IL_UNSIGNED_SHORT: return "UNSIGNED_SHORT";
  case IL_INT: return "INT";
  case IL_UNSIGNED_INT: return "UNSIGNED_INT";
  case IL_FLOAT: return "FLOAT";
  case IL_DOUBLE: return "DOUBLE";
  default: return "unknown";
  }

}
void il_id() {
  // Display the image's dimensions to the user.
  ILint fmt, typ;
  printf("Width: %d  Height: %d  Depth: %d Bytespp: %d Bpp: %d\n",
	 ilGetInteger(IL_IMAGE_WIDTH),
	 ilGetInteger(IL_IMAGE_HEIGHT),
	 ilGetInteger(IL_IMAGE_DEPTH),
	 ilGetInteger(IL_IMAGE_BYTES_PER_PIXEL),
	 ilGetInteger(IL_IMAGE_BITS_PER_PIXEL));

  fmt = ilGetInteger(IL_IMAGE_FORMAT);
  typ = ilGetInteger(IL_IMAGE_TYPE);

  printf("Format: %s  Type: %s\n",
	 fmt2name(fmt), typ2name(typ));
}

/* Case-insensitive keyword matcher. */

int
pm_keymatch( char *str, char *keyword, int minchars )
{
  register int len;
  char c1, c2;

  len = strlen( str );
  if ( len < minchars )
    return 0;
  while ( --len >= 0 )
    {

      c1 = *str++;
      c2 = *keyword++;
      if ( c2 == '\0' )
	return 0;
      if (isupper( c1 ))
	c1 = tolower( c1 );
      if (isupper( c2 ))
	c2 = tolower( c2 );
      if ( c1 != c2 )
	return 0;
    }
  return 1;
}

void pperror( char* str )
{
  printf("%s",str);
  exit(1);
}

/*  INITGAUSS  --  Initialise random number generators.  As given in
		   Peitgen & Saupe, page 77. */

static void initgauss(unsigned int seed)
{
  int stemp;

    /* Range of random generator */
    arand = pow(2.0, 15.0) - 1.0;
    gaussadd = sqrt(3.0 * Nrand);
    gaussfac = 2 * gaussadd / (Nrand * arand);
    /* srand(seed); */
    stemp = seed;       /* observe that 'seed' is unsigned, ie nonnegative */
    seed_ran1(stemp);        /* seed the generator */
}

/*  GAUSS  --  Return a Gaussian random number.  As given in Peitgen
	       & Saupe, page 77. */

static double gauss()
{
    int i;
    double sum = 0.0;

    for (i = 1; i <= Nrand; i++) {
	sum += (ran1() * 0x7FFF);
    }
    return gaussfac * sum - gaussadd;
}

	/* fill array with 1/f gaussian noise */
static void fillarray(float *a, int n, double h, double scale)
{
 int x,y, k, i0, j0, rank, nx, ny, xcent, ycent;
 double rad, phase, rcos, rsin;

 /*   printf("h = %f scale = %f\n",h, scale);  */

    nx = n; ny = n;  /* horizontal and vertical dimension of array */
    xcent = (int)(nx / 2.0 - 0.5);  /* center dimensions of array */
    ycent = (int)(ny / 2.0 - 0.5);
    printf("filling random array..."); 
    fflush(stdout);
    
    /* fill in mx. in order of radius, so we can generate higher resolutions
       with the same overall aspect (if seed is held fixed) */

    for (rank = 0; rank <= xcent; rank++) {
    /* fill quadrants 2 and 4  */
      for (k=0;k<=rank;k++) {
       x = k; y = rank;
       phase = 2 * M_PI * ((ran1() * 0x7FFF) / arand);
       if ((x == 0) && (y == 0)) rad = 0; 
	 else rad = pow((double) (x*x + y*y), -(h+1) / 2) * gauss();
       rcos = rad * cos(phase)*scale; rsin = rad * sin(phase)*scale;
       Real(a, x, y) += rcos; 
       Imag(a, x, y) += rsin;
       if (!((x == 0) && (y == 0))) { 
	i0 = nx-x-1; j0 = ny-y-1;
	Real(a, i0,j0) += rcos;
	Imag(a, i0,j0) += rsin;
       }

       x = rank; y = k;
       phase = 2 * M_PI * ((ran1() * 0x7FFF) / arand);
       if ((x == 0) && (y == 0)) rad = 0; 
	 else rad = pow((double) (x*x + y*y), -(h+1) / 2) * gauss();
       rcos = rad * cos(phase)*scale; rsin = rad * sin(phase)*scale;
       Real(a, x, y) += rcos;
       Imag(a, x, y) += rsin;
       if (!((x == 0) && (y == 0))) { 
	i0 = nx-x-1; j0 = ny-y-1;
	Real(a, i0,j0) += rcos;
	Imag(a, i0,j0) += rsin;
       }
      } /* end for k */
    
    /* now handle quadrants 1 and 3 */
      for (k=0;k<=rank;k++) {
       x = k; y = rank;
       phase = 2 * M_PI * ((ran1() * 0x7FFF) / arand);
       if ((x == 0) && (y == 0)) rad = 0; 
	 else rad = pow((double) (x*x + y*y), -(h+1) / 2) * gauss();
       rcos = rad * cos(phase)*scale; 
       rsin = rad * sin(phase)*scale;
       Real(a, x, ny-y-1) += rcos; 
       Imag(a, x, ny-y-1) += rsin;
       Real(a, nx-x-1, y) += rcos;
       Imag(a, nx-x-1, y) += rsin;
       
       x = rank; y = k;
       phase = 2 * M_PI * ((ran1() * 0x7FFF) / arand);
       if ((x == 0) && (y == 0)) rad = 0; 
	 else rad = pow((double) (x*x + y*y), -(h+1) / 2) * gauss();
       rcos = rad * cos(phase)*scale; rsin = rad * sin(phase)*scale;
       Real(a, x, ny-y-1) += rcos; 
       Imag(a, x, ny-y-1) += rsin;
       Real(a, nx-x-1, y) += rcos;
       Imag(a, nx-x-1, y) += rsin;
      
      } /* end for k */
    } /* end for rank */
    
    Imag(a, nx / 2, 0) = 0;
    Imag(a, 0, ny / 2) = 0;
    Imag(a, nx / 2, ny / 2) = 0;


} /* end fillarray() */


/* f_filter()  -- filter array of noise  with peak or notch filter */
/*                since this happens before the inverse FFT, it is */
/*                a frequency-domain filter.       */
/*                f_type 1 bpass, -1 breject, 2 lopass, -2 hipass */

void f_filter(float *a, int n, double center, double Q, int f_type)
{
 int i, j, i0, j0;
 double rad, fac, p, sfac;
 int xsize, ysize;

    xsize = n;
    ysize = n;
    if (center == 0.0) center = -0.00001;  /* avoid a singularity */

		     /* sfac is radius scaling factor */
    sfac = 1.0/sqrt((double)(xsize*xsize/4 + ysize*ysize/4));  
    printf("filtering array..");
    fflush(stdout);
    for (i = 0; i <= n / 2; i++) {          /* do quadrants 2 and 4 */
	for (j = 0; j <= n / 2; j++) {
	    if (i != 0 || j != 0) {
		rad = sqrt((double) (i * i + j * j)) * sfac;
	    } else {
		rad = 0;
	    }
	    p = 1.0 / pow(Q * center, 2);
	    if (abs(f_type)==1)
	      fac = p / (p + pow((1.0-rad/center),2)) ; /* bandpass/rej. */
	    else
	      fac = 1.0 / (1.0 + pow((rad/center),Q) ); /* lo/hi-pass */
	    if (f_type < 0) fac = (1.0 - fac);  /* invert filter */
	    Real(a, i, j) *= fac;
	    Imag(a, i, j) *= fac;
	    i0 = (i == 0) ? 0 : n - i;
	    j0 = (j == 0) ? 0 : n - j;
	    Real(a, i0, j0) *= fac;
	    Imag(a, i0, j0) *= fac;
	}
    }
    printf(".");
    fflush(stdout);
    Imag(a, n / 2, 0) = 0;
    Imag(a, 0, n / 2) = 0;
    Imag(a, n / 2, n / 2) = 0;
    for (i = 1; i <= n / 2 - 1; i++) {     /* do quadrants 1 and 3 */
	for (j = 1; j <= n / 2 - 1; j++) {
	    rad = sfac * sqrt((double) (i * i + j * j));
	    p = 1.0 / pow(Q * center, 2);
	    if (abs(f_type)==1)
	      fac = p / (p + pow((1.0-rad/center),2)) ; /* bandpass/rej. */
	    else
	      fac = 1.0 / (1.0 + pow((rad/center),Q) ); /* lo/hi-pass */
	    if (f_type < 0) fac = (1.0 - fac);  /* invert filter */
	    Real(a, i, n - j) *= fac;
	    Imag(a, i, n - j) *= fac;
	    Real(a, n - i, j) *= fac;
	    Imag(a, n - i, j) *= fac;
	}
    }

} /* end f_filter() */


/*  SPECTRALSYNTH  --  Spectrally  synthesised  fractal  motion in two
		       dimensions.  This algorithm is given under  the
		       name   SpectralSynthesisFM2D  on  page  108  of
		       Peitgen & Saupe. */

static void spectralsynth(float **x, unsigned int n)
{
    int i,j,nsize[2];
    unsigned long bl;
    float *a;

    bl = ((((unsigned long) n) * n) + 1) * 2 * sizeof(float);
    a = (float *) malloc( (size_t) (n+4)*(n+4)*2 * sizeof(float));
    if (a == (float *) 0) {
	fprintf(stderr,"Cannot allocate %d x %d result array (%lu bytes).\n",
	   n, n, bl);
	exit(1);
	}
    else
       printf("Success allocating %d x %d result array (%lu bytes).\n",
	   n, n, bl);
    *x = a;

    for (j=0;j<n;j++)   {       /* initialize array to zeros */
      for (i=0;i<n;i++)   {
	Real(a,i,j) = 0;
	Imag(a,i,j) = 0;
      }
    }

 /*   printf("factors = %d\n",d_factors); */
    for(i=0;i<d_factors;i++) {
     /*   printf("i:%d pwr:%5.3f scl:%5.3f\n",i,dim[i],dscale[i]); */
       fillarray(a, n, 3.0-dim[i], dscale[i]);   /* put 1/f noise into array */
      }    
      
    if ((i<20) && (j<20)) {
     printf("Real portion of freq. domain array:\n");
     for (j=0;j<n;j++) {
       for (i=0;i<n;i++) {
	 printf("%1.1e ",Real(a,i,j));
       }
       printf("\n");
     }
    }
    if (bpspec) f_filter(a,n,bpcenter,bpQ,1);   /* bandpass filtering */
    if (brspec) f_filter(a,n,brcenter,brQ,-1);  /* bandreject filtering */
    if (lpspec) f_filter(a,n,lpcut,lporder,2);  /* lowpass filter */
    if (hpspec) f_filter(a,n,hpcut,hporder,-2); /* highpass filter */

    nsize[0] = n;
    nsize[1] = n;          /* Dimension of frequency domain array */
    printf("Calculating inverse FFT\n");

			   /* Take inverse 2D Fourier transform */
    fftnf(2, nsize, &Real(a,0,0), &Imag(a,0,0), -2, 1.0);

    
/*    printf("returned from fft\n");
    for (j=0;j<n;j++)   {
      for (i=0;i<n;i++)   {
	printf("%2.3f/%2.3f ", Real(a,i,j),Imag(a,i,j) );
      }
      printf("\n");
    }
  */
}

/*  INITSEED  --  Generate initial random seed, if needed.  */

static void initseed()
{
  int i,stmp;

  stmp = time(NULL) ^ 0xF37C;
			   /* rseed is global var. to store the seed value */
  seed_ran1(stmp);
  for (i=0;i<8;i++)
    ran1();
  
  rseed = (U)(1000000.0*ran1());

}


/* genscape -- Write the pixmap from [n x n] elevation array. */

#define ZTYPE unsigned short

static void genscape(float *a, unsigned int n)
{
  int i, j;
  int ii, jj;
  int xsize, ysize;
  ZTYPE *fulldata, *rowdata;
  ILuint name;
	
  xsize = n; ysize = n;
  ii = 0; jj = 0;             /* pacify the compiler */

  if ( (fulldata = (ZTYPE*)malloc(xsize*ysize*sizeof(ZTYPE))) == NULL) {
    fprintf(stderr,"Could not allocate memory.\n");
    exit(1);
  }

  printf("genscape\n");

  for (j = 0; j < ysize; j++) {     /* loop over rows */
    if (peakspec) jj = (ysize + j + ystart) % ysize;
    else jj = j;
    
    rowdata = &(fulldata[n*j]);
    for (i = 0; i < xsize; i++) {  /* loop over pixels this row */
      if (peakspec) ii = (xsize + i + xstart) % xsize;   
      else ii = i;
      rowdata[i] = 65535*Real(a,ii,jj);  /* rowdata holds this row */
    } /* end for i */
  } /* end for j */

  printf("saving...");
  fflush(stdout);

  // begin IL save
    
  ilInit();
  iluInit();
  il_err("ini");

  printf("FormatSet %d  TypeSet %d\n",
	 ilGetInteger(IL_FORMAT_SET), ilGetInteger(IL_TYPE_SET));

  ilGenImages(1, &name);

  ilBindImage(name);
  il_err("genbin");

  printf("n=%d, meshsize=%d\n", n, meshsize);

  ilTexImage(meshsize, meshsize, 1, 1, IL_LUMINANCE, IL_UNSIGNED_SHORT, fulldata);
  il_err("texim");
  il_id();

  ilEnable(IL_FILE_OVERWRITE);
  ilSaveImage(output_filename);
  il_err("save");

  ilDeleteImages(1, &name);

  // end IL save

  printf("file written\n");   
 
}

/*  PLANET  --  Make the fBm landscape.  */

static Boolean planet(int meshsize)
{
    float *a = (float *) 0;
    int i, j, n;
    double r;
    U num_craters;
    int imax=0, jmax=0;             /* index of maximum picture element */

    if (!seedspec) {
	initseed();     /* stores a seed in global "rseed" */
    }
    initgauss(rseed);

    printf("-seed %d\n",rseed);
    /*img.rseed = rseed;*/


		/* generate the height array via IFFT on 1/f scaled noise */
	spectralsynth(&a, meshsize);
	if (a == (float *) 0) {
	    return FALSE;
	}

	n = meshsize;
	printf("rescaling");
	fflush(stdout);

	printf(".");
	fflush(stdout);

	if (limitspec) {        /* rescale to [limit_lo..limit_hi] */
	    rescale(&a, meshsize, limit_lo, limit_hi);
	}  else {
	    rescale(&a, meshsize, 0, 1);
	}
	printf(".");
	fflush(stdout);

	/* Apply power law scaling if non-unity scale is requested. */
	/* also, rescale to[-1..1] for crater program */

/* printf("power = %f\n",powscale); */

	if (powscale != 1.0) {
	    for (i = 0; i < meshsize; i++) {
		for (j = 0; j < meshsize; j++) {
		    r = Real(a, i, j);
		    if (r != 0)
		     Real(a, i, j) = sgn1(r)*pow(fabs(r), powscale);
		}
	    }
	}
	printf(".");
	fflush(stdout);

	  /* rescale all data to lie in the range [-1..1]  */
	rescale(&a, meshsize, -1, 1);
	printf(".");
	fflush(stdout);
    
    if (include_craters) {
	num_craters = (U) (700 * pow(((double)meshsize)/256,0.7)*c_density);
	printf("adding %d craters...", num_craters);
	distribute_craters(a, num_craters, meshsize, wrap, ch_scale);
    }
	rescale(&a, meshsize, 0, 1);            /* rescale to 0..1 */
	findpeak(a, meshsize, &imax, &jmax);     /* find peak location */
	printf("\n");
	
	xstart = imax - (int)((float)screenxsize * xfrac);
	ystart = jmax - (int)((float)screenysize * yfrac);

    genscape(a, meshsize);
    if (a != (float *) 0) {
	free((char *) a);
    }
     return TRUE;
}



double sgn1(double x)
{
  if (x > 0) 
    return(1.0);
  else if (x < 0)
    return(-1.0);
  else
    return(0);
}

void rescale(float **a, int meshsize, float min, float max)
{
 int i,j;
 float r;
 float rmin, rmax, scale, offset;

	rmin = Real(a[0],0,0);
	rmax = rmin;
	for (i = 0; i < meshsize; i++) {     /* find current rmin, rmax */
	    for (j = 0; j < meshsize; j++) {
		r = Real(a[0], i, j);
		rmin = min(rmin, r);
		rmax = max(rmax, r);
	    }
	}
	scale = (max-min) / (rmax - rmin);
	offset = min;

	/* rescale all data to lie in the range [min..max]  */

	for (i = 0; i < meshsize; i++) {
	    for (j = 0; j < meshsize; j++) {
		Real(a[0], i, j) = (scale * (Real(a[0], i, j)-rmin))+ offset;
	    }
	}
   /*   printf("\nrescale: min=%f  max = %f\n",rmin,rmax); */
} /* end rescale() */

/* findpeak() --  find peak location */

void findpeak(float *a, int meshsize, int *imax, int *jmax)
{

 int i,j;
 float r;
 float rmax;

	rmax = Real(a,0,0);
	imax[0] = 0;
	jmax[0] = 0;
	for (i = 0; i < meshsize; i++) {     /* find current rmin, rmax */
	    for (j = 0; j < meshsize; j++) {
		r = Real(a, i, j);
		if (r > rmax) {
		  rmax = r;
		  imax[0] = i;
		  jmax[0] = j;
		}
	    }
	}

} /* end findpeak() */


/* Warn if meshsize is prime or has large prime factor. */

void fac_warn(int mesh)
{
 int fac, flimit,num, maxf;

#define div(a,b)  (  (b)*floor((a)/(b)) == (a))

   fac = 2;
   num = mesh;
   maxf = 0;
   flimit = (1+num/2);

   do {
     if (div(num,fac)) {
       num /= fac;
       if (fac > maxf) maxf = fac;
     } else {
       fac++;
     }
   } while (fac < flimit);
   if ((num > 1) && (mesh > 150)) {
	printf("\nAdvisory: your meshsize (%d) is a prime number.\n",mesh);
	printf("If the IFFT is too slow, choose a different meshsize.\n\n");
   } else {
     if (maxf > 150) {
	printf("\nAdvisory: meshsize %d has a large prime factor (%d).\n",mesh,maxf);
	printf("If the IFFT is too slow, choose a different meshsize.\n\n");
	}
     } /* end else */

} /* end fac_warn() */


/*  MAIN  --  Main gforge program: read command line, set vars.  */

int main(int argc, char **argv)
{
  int i;
  char *usage = "  Usage: \n\
   gforge [-mesh <n>] [-dimension <f> [-adim <f> <sf>]] [-power <f> ] \n\
   [-peak <x> <y>] [-seed <n>] [-name <fname>] [-craters [<f> <g>]] \n\
   [-limit <fl> <fh>] [-wrapoff] \n\
   [-bpf <f Q>] [-brf <f Q>] [-lpf <f O>] [-hpf <f O>]  [-version]\n\
   -------\n\
   Default behavior is to generate a 128x128 TGA-format heightfield.\n\
   See the Readme file and the man page for more details.\n";

  Boolean dimspec = FALSE, meshspec = FALSE, powerspec = FALSE,
    namespec = FALSE, typespec = FALSE, adimspec = FALSE,
    in_namespec = FALSE;

  float ftmp;   /* temporary float value */

  /* -------------------------------------------------- */
  /*       scan command line args                */

  i=1;
  d_factors = 1;
  /* while ((i < argc) && (argv[i][1] != '\0')) {  */
  while ((i < argc) && (argv[i][1] != '\0')) { 

    if (pm_keymatch(argv[i], "-dimension", 2)) {
      i++;
      if ((i == argc) || (sscanf(argv[i], "%lf", &dim[0])  != 1))
	{fprintf(stderr,GVERSION"%s\n",usage);exit(1);}
      if (dim[0] <= 0.0) {
	pperror("fractal dimension must be greater than 0\n");
      }
      dscale[0] = 1;
      dimspec = TRUE;
    } else if (pm_keymatch(argv[i], "-adim", 2)) {
      if (d_factors > 9) pperror("sheesh! maximum of 10 dimensions\n");
      i++;
      if ((i == argc) || (sscanf(argv[i], "%lf", &dim[d_factors]) != 1))
	{fprintf(stderr,GVERSION"%s\n",usage);exit(1);}
      if (dim[d_factors] < 0.0) pperror("require dimension > 0\n");
      i++;
      if ((i==argc) || (sscanf(argv[i],"%lf",&dscale[d_factors]) != 1))
	{fprintf(stderr,GVERSION"%s\n",usage);exit(1);}
      adimspec = TRUE;    /* may be set more than once */
      d_factors++;
    } else if (pm_keymatch(argv[i], "-bpfilter", 3)) {
      i++;
      if ((i == argc) || (sscanf(argv[i],"%lf", &bpcenter) != 1))
	{fprintf(stderr,GVERSION"%s\n",usage);exit(1);}
      i++;
      if ((i == argc) || (sscanf(argv[i],"%lf", &bpQ) != 1))
	{fprintf(stderr,GVERSION"%s\n",usage);exit(1);}
      bpspec = TRUE;      
    } else if (pm_keymatch(argv[i], "-brfilter", 3)) {
      i++;
      if ((i == argc) || (sscanf(argv[i],"%lf", &brcenter) != 1))
	{fprintf(stderr,GVERSION"%s\n",usage);exit(1);}
      i++;
      if ((i == argc) || (sscanf(argv[i],"%lf", &brQ) != 1))
	{fprintf(stderr,GVERSION"%s\n",usage);exit(1);}
      brspec = TRUE;      
    } else if (pm_keymatch(argv[i], "-hpfilter", 3)) {
      i++;
      if ((i == argc) || (sscanf(argv[i],"%lf", &hpcut) != 1))
	{fprintf(stderr,GVERSION"%s\n",usage);exit(1);}
      i++;
      if ((i == argc) || (sscanf(argv[i],"%lf", &hporder) != 1))
	{fprintf(stderr,GVERSION"%s\n",usage);exit(1);}
      hpspec = TRUE;      
    } else if (pm_keymatch(argv[i], "-lpfilter", 3)) {
      i++;
      if ((i == argc) || (sscanf(argv[i],"%lf", &lpcut) != 1))
	{fprintf(stderr,GVERSION"%s\n",usage);exit(1);}
      i++;
      if ((i == argc) || (sscanf(argv[i],"%lf", &lporder) != 1))
	fprintf(stderr,GVERSION"%s\n",usage);
      lpspec = TRUE;      
    } else if (pm_keymatch(argv[i], "-mesh", 2)) {
      i++;
      if ((i == argc) || (sscanf(argv[i], "%d", &meshsize) != 1))
	{fprintf(stderr,GVERSION"%s\n",usage);exit(1);}
      if (meshsize <= 0)
	pperror("A mesh size greater than zero is suggested!\n");
      meshspec = TRUE;
    } else if (pm_keymatch(argv[i], "-power", 3)) {
      i++;
      if ((i == argc) || (sscanf(argv[i], "%lf", &powscale) != 1))
	{fprintf(stderr,GVERSION"%s\n",usage);exit(1);}
      if (powscale <= 0.0) {
	pperror("power factor must be greater than 0\n");
      }
      powerspec = TRUE;
    } else if (pm_keymatch(argv[i], "-limit", 2)) {
      i++;
      if ((i == argc) || (sscanf(argv[i], "%lf", &limit_lo) != 1))
	{fprintf(stderr,GVERSION"%s\n",usage); exit(1);}
      i++;
      if ((i == argc) || (sscanf(argv[i], "%lf", &limit_hi) != 1))
	{fprintf(stderr,GVERSION"%s\n",usage); exit(1);}
      if (limit_hi <= limit_lo) {
	fprintf(stderr,"Got limit low: %1.1e and high: %1.1e\n",
		limit_lo, limit_hi);
	pperror("High limit must be greater than low limit.\n");
      }
      limitspec = TRUE;
    } else if (pm_keymatch(argv[i], "-seed", 2)) {
      if (seedspec) {
	pperror("already specified a random seed\n");
      }
      i++;
      if ((i == argc) || (sscanf(argv[i], "%d", &rseed) != 1))
	{fprintf(stderr,GVERSION"%s\n",usage); exit(1); }
      seedspec = TRUE;
    } else if (pm_keymatch(argv[i], "-peak", 3)) {
      if (peakspec) pperror("already specified peak location\n");
      i++;
      if ((i == argc) || (sscanf(argv[i], "%f", &xfrac) != 1))
	{fprintf(stderr,GVERSION"%s\n",usage); exit(1);}
      i++;
      if ((i == argc) || (sscanf(argv[i], "%f", &yfrac) != 1))
	pperror("-peak requires two arguments, xfrac and yfrac.\n");
      if ((xfrac<0)||(xfrac>1)||(yfrac<0)||(yfrac>1))
	pperror("-peak arguments lie in the range [0..1]\n");
      peakspec = TRUE;
    } else if (pm_keymatch(argv[i], "-name", 2)) {
      i++;
      if (i == argc) {
	fprintf(stderr,GVERSION"%s\n",usage); exit(1); }
      strcpy(output_filename,argv[i]);
      namespec = TRUE;
    } else if (pm_keymatch(argv[i], "-input", 2)) {
      i++;
      if (i == argc)
	{fprintf(stderr,GVERSION"%s\n",usage); exit(1);}
      strcpy(input_filename,argv[i]);
      in_namespec = TRUE;
    } else if (pm_keymatch(argv[i], "-craters", 1)) {
      include_craters = TRUE;
      i++;
      if ((i==argc) || (sscanf(argv[i],"%f",&ftmp) != 1) || (ftmp==0))
	i--;  /* oops, didn't have a numerical argument */
      else {
	c_density = ftmp;   /* crater density argument */
	i++;                /* check for 2nd arg. */
	if ((i==argc) || (sscanf(argv[i],"%f",&ftmp) != 1) || (ftmp==0))
	  i--;  /* oops, didn't have another numerical argument */
	else
	  ch_scale = ftmp;  /* crater amplitude argument */
      }
    } else if (pm_keymatch(argv[i], "-wrapoff",2)) {
      wrap = FALSE;
    } else if (pm_keymatch(argv[i], "-version",2)) {
      fprintf(stderr,GVERSION" - "GAUTHOR"\n");
      exit(1); 
    }
    /*
      else if (pm_keymatch(argv[i], "-type", 2)) {
      if (typespec) {
      pperror("already specified a filetype\n");
      }
      i++;
      if (i == argc) {fprintf(stderr,GVERSION"%s\n",usage);exit(1);}

      if (pm_keymatch(argv[i], "TGA", 3))
      pp_filetype = TGA;
      else if (pm_keymatch(argv[i], "PGM", 3))
      pp_filetype = PGM;
      else if (pm_keymatch(argv[i], "PG8", 3))
      pp_filetype = PG8;
      else if (pm_keymatch(argv[i], "OCT", 3))
      pp_filetype = OCT;
      else if (pm_keymatch(argv[i], "MAT", 3))
      pp_filetype = MAT;
      else if (pm_keymatch(argv[i], "PNG", 3))
      pp_filetype = PNG;
      else pperror("Don't understand that file type.\n");
      typespec = TRUE;
      } 
    */
    else {
      fprintf(stderr,"%s not understood.\n",argv[i]);
      fprintf(stderr,GVERSION"%s\n",usage);
      exit(1);
    }
    i++;
  } /* end while */

  /* -------------------------------------------------- */

  fac_warn(meshsize); /* warn if meshsize has large prime factor */

  /* Set defaults when explicit specifications were not given.  */

  if (!dimspec) {
    if (adimspec) pperror("use of -adim requires also the -dim option\n");
    dim[0] = 2.15;
    dscale[0] = 1.0;
    d_factors = 1;      /* number of powers defined */

    fracdim = 2.15;
  }
  if (!powerspec) {
    powscale = 1.2;
  }
  if (!meshspec) {
    meshsize = 128;
  }
  /*
  if (!typespec) {
    pp_filetype = TGA;
  }
  */
  if (!namespec) {
    /*
    if ((pp_filetype == PGM) || (pp_filetype == PG8)  )
      strcpy(output_filename,"output.pgm");
    else if (pp_filetype == TGA)
      strcpy(output_filename,"output.tga");
    else if (pp_filetype == PNG)
      strcpy(output_filename,"output.png");
    else if (pp_filetype == OCT)
      strcpy(output_filename,"output.oct");
    else if (pp_filetype == MAT)
      strcpy(output_filename,"output.mat");
    else
    */
    strcpy(output_filename,"output.png");  /* unknown type (!?) */
  }

  if (!wrap && peakspec) {
    pperror("-wrapoff and -peak options are mutually exclusive!\n");
  }
    
  screenxsize = meshsize;
  screenysize = meshsize;

  /* print out parameters so we know how we generated this file */

  printf("Parameters: -dim %.2f -power %.2f -mesh %d -name %s\n",
	 dim[0], powscale, meshsize, output_filename);
    
  if (adimspec) {
    for (i=1;i<d_factors;i++) {
      printf("-ad %0.2f %0.2f ",dim[i],dscale[i]);
    }
  }
    
  if (include_craters)
    printf("-craters %1.1f %1.2f ",c_density,ch_scale);

  if (!wrap)
    printf("-wrapoff ");

  if (!wrap && !include_craters) 
    printf("(-wrapoff option applies only to craters!) ");

  if (peakspec) 
    printf("-peak %0.2f %0.2f \n",xfrac,yfrac);
    
  if (limitspec)
    printf("-limit %1.2f %1.2f ",limit_lo, limit_hi);

  if (bpspec) printf("-bp %1.2f %1.1f ",bpcenter,bpQ);
  if (brspec) printf("-br %1.2f %1.1f ",brcenter,brQ);
  if (lpspec) printf("-lp %1.2f %1.1f ",lpcut,lporder);
  if (hpspec) printf("-hp %1.2f %1.1f ",hpcut,hporder);

  /*
    img.dim= (double *)malloc(10*sizeof(double));
    img.dscale = (double *)malloc(10*sizeof(double));
		       
    for (i=0;i<d_factors;i++) {
    img.dim[i] = dim[i];
    img.dscale[i] = dscale[i];
    }
    strcpy(img.fname,output_filename);
    img.type = pp_filetype;
    img.powscale = powscale;
    img.xfrac = xfrac;
    img.yfrac = yfrac;
    img.rseed = rseed;
    img.d_factors = d_factors;
    img.peakspec = peakspec;
    img.adimspec = adimspec;
    img.meshsize = meshsize;
    img.craterspec = include_craters;
    img.craterdens = c_density;
    img.craterscale = ch_scale;
    img.limitspec = limitspec;
    img.limit_hi = limit_hi;
    img.limit_lo = limit_lo;
  */
  return(planet(meshsize) ? 0 : 1);   /* run the algorithm; output results */

} /* end main() */
