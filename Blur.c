#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <omp.h>
#define CHUNKSIZE = 20

int main (int argc, const char * argv[]) {
	static int const maxlen = 200, rowsize = 521, colsize = 428, linelen = 12;
	char str[maxlen], lines[5][maxlen];

	/* declares pointer of FILE type */
	FILE *fp, *fout;
	int nlines = 0;
	int chunk;

	/* h1, h2, and h3 can hold zero and positive numbers */
	unsigned int h1, h2, h3;

	/* pointer to a character */
	char *sptr;

	int R[rowsize][colsize], G[rowsize][colsize], B[rowsize][colsize];
	int Rnew[rowsize][colsize], Gnew[rowsize][colsize], Bnew[rowsize][colsize];
	int row = 0, col = 0, nblurs, lineno=0, k;


	struct timeval tim;
	
	/* Pointing fp to a file David.ps to read from it*/
	fp = fopen("David.ps", "r");
 
 	/* while not at the end of the file */
	while(! feof(fp))
	{
		/* reads from file fp, skips \n, puts value in string str */
		fscanf(fp, "\n%[^\n]", str);


		if (nlines < 5) {

			/* copies (char *)str into (char *)lines[nlines++] */
			strcpy((char *)lines[nlines++],(char *)str);
		}
		else{

			/* for: sptr = address of str[0], sptr not equal to '\0', sptr += 6 */
			for (sptr=&str[0];*sptr != '\0';sptr+=6){

				/* uses string stored in sptr (red value), takes hexadecimal integer %2x, then transfers it to variable &h1 */
				sscanf(sptr,"%2x",&h1);
				/* uses string stored in sptr+2 (green value), takes hexadecimal integer %2x, then transfers it to variable &h2 */
				sscanf(sptr+2,"%2x",&h2);
				/* uses string stored in sptr+4 (blue value), takes hexadecimal integer %2x, then transfers it to variable &h3 */
				sscanf(sptr+4,"%2x",&h3);
				
				/* if at end of column, column = 0 and move to next row */
				if (col==colsize){
					col = 0;
					row++;
				}

				/* if at end of row, set R, G, and B values to h1, h2, and h3 respectively */
				if (row < rowsize) {
					R[row][col] = h1;
					G[row][col] = h2;
					B[row][col] = h3;
				}

				/* increase the column */
				col++;
			}
		}
	}

	/* closes the stream of the fp file */
	fclose(fp);
	

	nblurs = 10;

	/* gets the time of day (what did you think it'd do??) */
	gettimeofday(&tim, NULL);

	/* create variable t1 = current seconds + microseconds */
	double t1=tim.tv_sec+(tim.tv_usec/1000000.0);
	
	/* for value k = 0 to k = nblurs */
	for(k=0;k<nblurs;k++){

		#pragma omp parallel shared(R, G, B, Rnew, Gnew, Bnew, rowsize, colsize, chunk) private(row, col) num_threads(5) 
		{
			#pragma omp for collapse(2) schedule(static, chunk) nowait
		

				/* for row = 0 to row = rowsize */
				for(row=0;row<rowsize;row++){

					/* for col = 0 to col = colsize */
					for (col=0;col<colsize;col++){	

						/* if not on edge of image (in middle of image effectively) */
						if (row != 0 && row != (rowsize-1) && col != 0 && col != (colsize-1)){
							/* Rnew, Bnew, and Gnew at that position = average of adjacent pixels */
							Rnew[row][col] = (R[row+1][col]+R[row-1][col]+R[row][col+1]+R[row][col-1])/4;
							Gnew[row][col] = (G[row+1][col]+G[row-1][col]+G[row][col+1]+G[row][col-1])/4;
							Bnew[row][col] = (B[row+1][col]+B[row-1][col]+B[row][col+1]+B[row][col-1])/4;
						}
				
						/* if at the top of image (but not top left or top right) */
						else if (row == 0 && col != 0 && col != (colsize-1)){
							/* Rnew, Bnew, and Gnew at that position = average of pixels to left, right, and bot */
							Rnew[row][col] = (R[row+1][col]+R[row][col+1]+R[row][col-1])/3;
							Gnew[row][col] = (G[row+1][col]+G[row][col+1]+G[row][col-1])/3;
							Bnew[row][col] = (B[row+1][col]+B[row][col+1]+B[row][col-1])/3;
						}
						/* if at the bottom of image (but not bottom left or bottom right) */
						else if (row == (rowsize-1) && col != 0 && col != (colsize-1)){
							/* Rnew, Bnew and Gnew at that position = averga of pixels to left, right, and top */
							Rnew[row][col] = (R[row-1][col]+R[row][col+1]+R[row][col-1])/3;
							Gnew[row][col] = (G[row-1][col]+G[row][col+1]+G[row][col-1])/3;
							Bnew[row][col] = (B[row-1][col]+B[row][col+1]+B[row][col-1])/3;
						}
						/* if at the left border of image (but not top left or bottom left) */
						else if (col == 0 && row != 0 && row != (rowsize-1)){
							Rnew[row][col] = (R[row+1][col]+R[row-1][col]+R[row][col+1])/3;
							Gnew[row][col] = (G[row+1][col]+G[row-1][col]+G[row][col+1])/3;
							Bnew[row][col] = (B[row+1][col]+B[row-1][col]+B[row][col+1])/3;
						}
						/* if at the right border of image (but not top right or bottom right) */
						else if (col == (colsize-1) && row != 0 && row != (rowsize-1)){
							Rnew[row][col] = (R[row+1][col]+R[row-1][col]+R[row][col-1])/3;
							Gnew[row][col] = (G[row+1][col]+G[row-1][col]+G[row][col-1])/3;
							Bnew[row][col] = (B[row+1][col]+B[row-1][col]+B[row][col-1])/3;
						}
				
						/* if at top left of image */
						else if (row==0 &&col==0){
							Rnew[row][col] = (R[row][col+1]+R[row+1][col])/2;
							Gnew[row][col] = (G[row][col+1]+G[row+1][col])/2;
							Bnew[row][col] = (B[row][col+1]+B[row+1][col])/2;
						}
						/* if at top right of image */
						else if (row==0 &&col==(colsize-1)){
							Rnew[row][col] = (R[row][col-1]+R[row+1][col])/2;
							Gnew[row][col] = (G[row][col-1]+G[row+1][col])/2;
							Bnew[row][col] = (B[row][col-1]+B[row+1][col])/2;
						}
						/* if at bottom left of image */
						else if (row==(rowsize-1) &&col==0){
							Rnew[row][col] = (R[row][col+1]+R[row-1][col])/2;
							Gnew[row][col] = (G[row][col+1]+G[row-1][col])/2;
							Bnew[row][col] = (B[row][col+1]+B[row-1][col])/2;
						}
						/* if at bottom right of image */
						else if (row==(rowsize-1) &&col==(colsize-1)){
							Rnew[row][col] = (R[row][col-1]+R[row-1][col])/2;
							Gnew[row][col] = (G[row][col-1]+G[row-1][col])/2;
							Bnew[row][col] = (B[row][col-1]+B[row-1][col])/2;
						}		
					}
				}
		}

		/* change every pixel in R to value in Rnew at that position */
		for(row=0;row<rowsize;row++){
			for (col=0;col<colsize;col++){
			    R[row][col] = Rnew[row][col];
			    G[row][col] = Gnew[row][col];
			    B[row][col] = Bnew[row][col];
			}
		}
	}

	/* get the current time of day */
	gettimeofday(&tim, NULL);

	/* get seconds at this current time of day */
	double t2=tim.tv_sec+(tim.tv_usec/1000000.0);

	/* print the number of seconds the process has taken */
	printf("%.6lf seconds elapsed\n", t2-t1);
	
	/* fout = DavidBlur.ps file, ready to write to file */
	fout= fopen("DavidBlur.ps", "w");

	/* for k = 0 to k = nlines, print fout + "\n" + lines[k] */
	for (k=0;k<nlines;k++) fprintf(fout,"\n%s", lines[k]);
	fprintf(fout,"\n");
	
	/* for every row, col value */
	for(row=0;row<rowsize;row++){
		for (col=0;col<colsize;col++){

			/* print RGB value at that position */
			fprintf(fout,"%02x%02x%02x",R[row][col],G[row][col],B[row][col]);
			
			lineno++;

			/* if at end of lines */
			if (lineno==linelen){
				fprintf(fout,"\n");
				lineno = 0;
			}
		}
	}

	fclose(fout);
    return 0;
}
