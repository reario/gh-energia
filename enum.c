
#include <stdio.h>
#include <time.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#ifdef LIBSTAT
#include <gsl/gsl_statistics.h>
#endif

unsigned long long int i,j,N;

double xi,sumxi,sumxi2,sigma;
#define linemax 32
#define Nxi 1200000

int main () {

  char line[linemax];
  char val[7];
  FILE *fp;

  //double values[Nxi];
  //double freq[Nxi];

  //  printf("start\n");
  fp=(FILE *)fopen("./volt.log","r");

  if ( fp==NULL) { printf("errore di apertura file\n"); return 1;}
  
  sumxi=0;sumxi2=0;N=0;
  //memset(freq,0,sizeof(double)*Nxi);

  while ( fgets(line, linemax, fp) != NULL) {
    strncpy(val,line,(size_t)5);
    val[5]='\0';
    xi=atof(val);
    if (xi>200) {
      //     values[N]=xi;
      sumxi=sumxi+xi;
      sumxi2=sumxi2+pow(xi,2);
      N=N+1;
      sigma=sqrt(N*sumxi2-pow(sumxi,2))/N;
    }
  }
  
#ifdef LIBSTAT
  //printf("%lld\n",N);
  double mean, variance, largest, smallest;
  mean     = gsl_stats_mean(values, 1, N);
  variance = gsl_stats_variance(values, 1, N);
  largest  = gsl_stats_max(values, 1, N);
  smallest = gsl_stats_min(values, 1, N);
  printf ("The sample mean is %g\n", mean);
  printf ("The estimated variance is %g\n", sqrt(variance));
  printf ("The largest value is %g\n", largest);
  printf ("The smallest value is %g\n", smallest);
#endif

#if 0
  for (i=0;i<N;i++) {
    for (j=0;j<Nxi;j++) {

	if ( (values[i]==values[j])) { 	  
	  freq[i]=freq[i]+1; 
	}
    }
    printf("%3.1f %3.0f\n",values[i],freq[i]);
  }
#endif
  printf("%f %f %lld\n",sigma,sumxi/N, N);
  return 0;
}
