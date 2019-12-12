#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <cuda.h>
#define TX 16
#define TY 32
#define NPTSX 200
#define NPTSY 200

__global__
void blur(int *R, int *G, int *B, int *Rnew, int *Gnew, int *Bnew, int colsize, int rowsize) {

  int col = blockIdx.x * blockDim.x + threadIdx.x;
  int row = blockIdx.y * blockDim.y + threadIdx.y;
  int i = row * colsize + col;

  if (row != 0 && row != (rowsize-1) && col != 0 && col != (colsize-1)){
  					Rnew[i] = (R[(row+1)*colsize+col]+R[(row-1)*colsize+col]+R[row*colsize+col+1]+R[row*colsize+col-1])/4;
  					Gnew[i] = (G[(row+1)*colsize+col]+G[(row-1)*colsize+col]+G[row*colsize+col+1]+G[row*colsize+col-1])/4;
  					Bnew[i] = (B[(row+1)*colsize+col]+B[(row-1)*colsize+col]+B[row*colsize+col+1]+B[row*colsize+col-1])/4;
  				}
  				else if (row == 0 && col != 0 && col != (colsize-1)){
  					Rnew[i] = (R[(row+1)*colsize+col]+R[row*colsize+col+1]+R[row*colsize+col-1])/3;
  					Gnew[i] = (G[(row+1)*colsize+col]+G[row*colsize+col+1]+G[row*colsize+col-1])/3;
  					Bnew[i] = (B[(row+1)*colsize+col]+B[row*colsize+col+1]+B[row*colsize+col-1])/3;
  				}
  				else if (row == (rowsize-1) && col != 0 && col != (colsize-1)){
  					Rnew[i] = (R[(row-1)*colsize+col]+R[row*colsize+col+1]+R[row*colsize+col-1])/3;
  					Gnew[i] = (G[(row-1)*colsize+col]+G[row*colsize+col+1]+G[row*colsize+col-1])/3;
  					Bnew[i] = (B[(row-1)*colsize+col]+B[row*colsize+col+1]+B[row*colsize+col-1])/3;
  				}
  				else if (col == 0 && row != 0 && row != (rowsize-1)){
  					Rnew[i] = (R[(row+1)*colsize+col]+R[(row-1)*colsize+col]+R[row*colsize+col+1])/3;
  					Gnew[i] = (G[(row+1)*colsize+col]+G[(row-1)*colsize+col]+G[row*colsize+col+1])/3;
  					Bnew[i] = (B[(row+1)*colsize+col]+B[(row-1)*colsize+col]+B[row*colsize+col+1])/3;
  				}
  				else if (col == (colsize-1) && row != 0 && row != (rowsize-1)){
  					Rnew[i] = (R[(row+1)*colsize+col]+R[(row-1)*colsize+col]+R[row*colsize+col-1])/3;
  					Gnew[i] = (G[(row+1)*colsize+col]+G[(row-1)*colsize+col]+G[row*colsize+col-1])/3;
  					Bnew[i] = (B[(row+1)*colsize+col]+B[(row-1)*colsize+col]+B[row*colsize+col-1])/3;
  				}
  				else if (row==0 &&col==0){
  					Rnew[i] = (R[row*colsize+col+1]+R[(row+1)*colsize+col])/2;
  					Gnew[i] = (G[row*colsize+col+1]+G[(row+1)*colsize+col])/2;
  					Bnew[i] = (B[row*colsize+col+1]+B[(row+1)*colsize+col])/2;
  				}
  				else if (row==0 &&col==(colsize-1)){
  					Rnew[i] = (R[row*colsize+col-1]+R[(row+1)*colsize+col])/2;
  					Gnew[i] = (G[row*colsize+col-1]+G[(row+1)*colsize+col])/2;
  					Bnew[i] = (B[row*colsize+col-1]+B[(row+1)*colsize+col])/2;
  				}
  				else if (row==(rowsize-1) &&col==0){
  					Rnew[i] = (R[row*colsize+col+1]+R[(row-1)*colsize+col])/2;
  					Gnew[i] = (G[row*colsize+col+1]+G[(row-1)*colsize+col])/2;
  					Bnew[i] = (B[row*colsize+col+1]+B[(row-1)*colsize+col])/2;
  				}
  				else if (row==(rowsize-1) &&col==(colsize-1)){
  					Rnew[i] = (R[row*colsize+col-1]+R[(row-1)*colsize+col])/2;
  					Gnew[i] = (G[row*colsize+col-1]+G[(row-1)*colsize+col])/2;
  					Bnew[i] = (B[row*colsize+col-1]+B[(row-1)*colsize+col])/2;
  				}

  R[row * colsize + col] = Rnew[row * colsize + col];
  G[row * colsize + col] = Gnew[row * colsize + col];
  B[row * colsize + col] = Bnew[row * colsize + col];
}

int main (int argc, const char * argv[]) {

	static int const maxlen = 200, rowsize = 512, colsize = 428, linelen = 12;
	char str[maxlen], lines[5][maxlen];
	FILE *fp, *fout;
	int nlines = 0;
	unsigned int h1, h2, h3;
	char *sptr;
	int R[rowsize][colsize], G[rowsize][colsize], B[rowsize][colsize];
  int Rnew[(rowsize*colsize)], Gnew[(rowsize*colsize)], Bnew[(rowsize*colsize)];
  int row = 0, col = 0, nblurs, lineno=0, k;
	struct timeval tim;

  gettimeofday(&tim, NULL);
  double inputt1=tim.tv_sec+(tim.tv_usec/1000000.0);
	fp = fopen("David.ps", "r");

	while(! feof(fp))
	{
		fscanf(fp, "\n%[^\n]", str);
		if (nlines < 5) {strcpy((char *)lines[nlines++],(char *)str);}
		else {
			for (sptr=&str[0];*sptr != '\0';sptr+=6){
				sscanf(sptr,"%2x",&h1);
				sscanf(sptr+2,"%2x",&h2);
				sscanf(sptr+4,"%2x",&h3);

				if (col==colsize){
					col = 0;
					row++;
				}
				if (row < rowsize) {
					R[row][col] = h1;
					G[row][col] = h2;
					B[row][col] = h3;
				}
				col++;
			}
		}
	}
	fclose(fp);
  gettimeofday(&tim, NULL);
  double inputt2=tim.tv_sec+(tim.tv_usec/1000000.0);
  printf("Time to read in input file: %f\n",(inputt2-inputt1));


	nblurs = 160;
  int *d_R, *d_G, *d_B, *d_Rnew, *d_Gnew, *d_Bnew;
	int size = sizeof(int)*colsize*rowsize;

  /*printf("Number of blurs: %d\n\n", nblurs);*/

  gettimeofday(&tim, NULL);
  double allocatet1=tim.tv_sec+(tim.tv_usec/1000000.0);

	cudaMalloc((void **) &d_R, size);
	cudaMalloc((void **) &d_G, size);
	cudaMalloc((void **) &d_B, size);
	cudaMalloc((void **) &d_Rnew, size);
	cudaMalloc((void **) &d_Gnew, size);
	cudaMalloc((void **) &d_Bnew, size);

  gettimeofday(&tim, NULL);
  double allocatet2=tim.tv_sec+(tim.tv_usec/1000000.0);
  printf("Time to allocate memory: %f\n", (allocatet2-allocatet1));


  gettimeofday(&tim, NULL);
  double datat1=tim.tv_sec+(tim.tv_usec/1000000.0);

  cudaMemcpy(d_R, &R, size, cudaMemcpyHostToDevice);
	cudaMemcpy(d_G, &G, size, cudaMemcpyHostToDevice);
	cudaMemcpy(d_B, &B, size, cudaMemcpyHostToDevice);

  gettimeofday(&tim, NULL);
  double datat2=tim.tv_sec+(tim.tv_usec/1000000.0);
  //printf("Time to transfer data: %f\n", (datat2-datat1));

  dim3 dimGrid(ceil(colsize/(float)TX), ceil(rowsize/(float)TY), 1);
  dim3 dimBlock(TX, TY, 1);

	gettimeofday(&tim, NULL);
	double blurt1=tim.tv_sec+(tim.tv_usec/1000000.0);

  for (int j = 0; j <nblurs; j++){
    blur<<<dimBlock, dimGrid>>>(d_R, d_G, d_B, d_Rnew, d_Gnew, d_Bnew, colsize, rowsize);
  }

  gettimeofday(&tim, NULL);
	double blurt2=tim.tv_sec+(tim.tv_usec/1000000.0);
  printf("Time to blur image: %f\n", (blurt2-blurt1));

  gettimeofday(&tim, NULL);
  double datat3=tim.tv_sec+(tim.tv_usec/1000000.0);
  cudaMemcpy(&Rnew, d_Rnew, size, cudaMemcpyDeviceToHost);
  cudaMemcpy(&Gnew, d_Gnew, size, cudaMemcpyDeviceToHost);
  cudaMemcpy(&Bnew, d_Bnew, size, cudaMemcpyDeviceToHost);
  gettimeofday(&tim, NULL);
  double datat4=tim.tv_sec+(tim.tv_usec/1000000.0);
  printf("Time to transfer data: %f\n", ((datat2-datat1) + (datat4-datat3)));

  gettimeofday(&tim, NULL);
	double outputt1=tim.tv_sec+(tim.tv_usec/1000000.0);
  for (int row=0;row<rowsize;row++){
    for (int col=0;col<colsize;col++){
      int count1 = row*colsize+col;
      R[row][col] = Rnew[count1];
      G[row][col] = Gnew[count1];
      B[row][col] = Bnew[count1];
    }
  }


  cudaFree(d_R); cudaFree(d_G); cudaFree(d_B); cudaFree(d_Rnew); cudaFree(d_Gnew); cudaFree(d_Bnew);

	fout= fopen("DavidBlur.ps", "w");
	for (k=0;k<nlines;k++) fprintf(fout,"\n%s", lines[k]);
	fprintf(fout,"\n");
	for(row=0;row<rowsize;row++){
		for (col=0;col<colsize;col++){
			fprintf(fout,"%02x%02x%02x",R[row][col],G[row][col],B[row][col]);
			lineno++;
			if (lineno==linelen){
				fprintf(fout,"\n");
				lineno = 0;
			}
		}
	}
	fclose(fout);

  gettimeofday(&tim, NULL);
  double outputt2=tim.tv_sec+(tim.tv_usec/1000000.0);
  printf("Time to output image: %f\n", (outputt2-outputt1));
    return 0;
}
