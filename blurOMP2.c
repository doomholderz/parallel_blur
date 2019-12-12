// In this code I have both methods for creating DavidBlur.ps using parallelism, and also
// methods for getting the average timing for these implementations. The reason I have done
// this is if I had only 1 method for each parallel implementation, the repetitions I do 
// for each of them will blur the image 10x more than what it should be. 

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <omp.h>
#include <math.h>

int main (int argc, const char * argv[]) {
	static int const maxlen = 200, rowsize = 521, colsize = 428, linelen = 12;
	char str[maxlen], lines[5][maxlen];

	/* declares pointer of FILE type */
	FILE *fp, *fout;
	int nlines = 0;
	int chunk;
	int threadnum;

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
	
	chunk = 6;
	nblurs = 10;
	threadnum = 4;



	double mean;
	double mean2;
	double mean3;
	float values1[10];
	float values2[10];
	float values3[10];



	void nonParallelTest() {
	printf("Number of threads: %d, chunk size: %d\n",threadnum,chunk);

	// REGULAR NON-PARALLEL IMPLEMENTATION //
	for (int repetitions=0; repetitions<10; repetitions++) {

		/* gets the time of day (what did you think it'd do??) */
		gettimeofday(&tim, NULL);

		/* create variable t1 = current seconds + microseconds */
		double t1=tim.tv_sec+(tim.tv_usec/1000000.0);
	
		/* for value k = 0 to k = nblurs */
		for(k=0;k<nblurs;k++){
		

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

	mean += (t2 - t1);
	values1[repetitions] = (t2 - t1);
	

	}

	mean = mean / 10;

	float prestandard = 0;
	for (int z=0;z<10;z++) {
		float newval1 = 0;
		newval1 = values1[z] - mean;
		newval1 = newval1 * newval1;
		prestandard += newval1;
	}
	prestandard = sqrt((prestandard / 10));
	// prestandard is the standard deviation of the values

	printf("\nAverage time for non-parallel: %lf", (mean));
	printf("\nStandard deviation for non-parallel: %lf", prestandard);
	printf("\n\n");
}



// This function is used to create DavidBlur.ps using static parallel scheduling. The function
// is later used by staticParallelTest to get average time this takes to do all computations,
// and standard deviation of these timings.
void staticParallelOutput(){
	#pragma omp parallel for collapse(3) num_threads(threadnum)
		#pragma omp schedule(static, chunk) nowait
			for(k=0;k<nblurs;k++){
		
		// collapse to turn 2d array into 1d array
		

				for(row=0;row<rowsize;row++){
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
		for(row=0;row<rowsize;row++){
			for (col=0;col<colsize;col++){
			    R[row][col] = Rnew[row][col];
			    G[row][col] = Gnew[row][col];
			    B[row][col] = Bnew[row][col];
			}
		}
	}



// This function is used to create DavidBlur.ps using dynamic parallel scheduling. The function
// is later used by dynamicParallelTest to get average time this takes to do all computations,
// and standard deviation of these timings.
void dynamicParallelOutput(){
	#pragma omp parallel for collapse(3)
		#pragma omp schedule(dynamic, chunk) nowait
			for(k=0;k<nblurs;k++){
			for(row=0;row<rowsize;row++){
				for (col=0;col<colsize;col++){	
					if (row != 0 && row != (rowsize-1) && col != 0 && col != (colsize-1)){
						Rnew[row][col] = (R[row+1][col]+R[row-1][col]+R[row][col+1]+R[row][col-1])/4;
						Gnew[row][col] = (G[row+1][col]+G[row-1][col]+G[row][col+1]+G[row][col-1])/4;
						Bnew[row][col] = (B[row+1][col]+B[row-1][col]+B[row][col+1]+B[row][col-1])/4;
					}
					else if (row == 0 && col != 0 && col != (colsize-1)){
						Rnew[row][col] = (R[row+1][col]+R[row][col+1]+R[row][col-1])/3;
						Gnew[row][col] = (G[row+1][col]+G[row][col+1]+G[row][col-1])/3;
						Bnew[row][col] = (B[row+1][col]+B[row][col+1]+B[row][col-1])/3;
					}
					else if (row == (rowsize-1) && col != 0 && col != (colsize-1)){
						Rnew[row][col] = (R[row-1][col]+R[row][col+1]+R[row][col-1])/3;
						Gnew[row][col] = (G[row-1][col]+G[row][col+1]+G[row][col-1])/3;
						Bnew[row][col] = (B[row-1][col]+B[row][col+1]+B[row][col-1])/3;
					}
					else if (col == 0 && row != 0 && row != (rowsize-1)){
						Rnew[row][col] = (R[row+1][col]+R[row-1][col]+R[row][col+1])/3;
						Gnew[row][col] = (G[row+1][col]+G[row-1][col]+G[row][col+1])/3;
						Bnew[row][col] = (B[row+1][col]+B[row-1][col]+B[row][col+1])/3;
					}
					else if (col == (colsize-1) && row != 0 && row != (rowsize-1)){
						Rnew[row][col] = (R[row+1][col]+R[row-1][col]+R[row][col-1])/3;
						Gnew[row][col] = (G[row+1][col]+G[row-1][col]+G[row][col-1])/3;
						Bnew[row][col] = (B[row+1][col]+B[row-1][col]+B[row][col-1])/3;
					}
					else if (row==0 &&col==0){
						Rnew[row][col] = (R[row][col+1]+R[row+1][col])/2;
						Gnew[row][col] = (G[row][col+1]+G[row+1][col])/2;
						Bnew[row][col] = (B[row][col+1]+B[row+1][col])/2;
					}
					else if (row==0 &&col==(colsize-1)){
						Rnew[row][col] = (R[row][col-1]+R[row+1][col])/2;
						Gnew[row][col] = (G[row][col-1]+G[row+1][col])/2;
						Bnew[row][col] = (B[row][col-1]+B[row+1][col])/2;
					}
					else if (row==(rowsize-1) &&col==0){
						Rnew[row][col] = (R[row][col+1]+R[row-1][col])/2;
						Gnew[row][col] = (G[row][col+1]+G[row-1][col])/2;
						Bnew[row][col] = (B[row][col+1]+B[row-1][col])/2;
					}
					else if (row==(rowsize-1) &&col==(colsize-1)){
						Rnew[row][col] = (R[row][col-1]+R[row-1][col])/2;
						Gnew[row][col] = (G[row][col-1]+G[row-1][col])/2;
						Bnew[row][col] = (B[row][col-1]+B[row-1][col])/2;
					}		
				}	
			}
		}
		for(row=0;row<rowsize;row++){
			for (col=0;col<colsize;col++){
			    R[row][col] = Rnew[row][col];
			    G[row][col] = Gnew[row][col];
			    B[row][col] = Bnew[row][col];
			}
		}
	}



// this function repeats static implementation in staticParallelOutput() 10 times to find 
// average time to complete, and standard deviation of times. this is used specificly for 
// the report
	void staticParallelTest() {
	// STATIC PARALLEL IMPLEMENTATION //
	for (int repetitions=0; repetitions<10; repetitions++) {

	gettimeofday(&tim, NULL);
	double t3=tim.tv_sec+(tim.tv_usec/1000000.0);
	
	
	staticParallelOutput();


	gettimeofday(&tim, NULL);
	double t4=tim.tv_sec+(tim.tv_usec/1000000.0);
	printf("%.6lf seconds elapsed\n", t4-t3);

	mean2 += (t4 - t3);
	values2[repetitions] = (t4 - t3);
	}

	mean2 = mean2 / 10;

	float prestandard2 = 0;
	for (int z1=0;z1<10;z1++) {
		float newval2 = 0;
		newval2 = values2[z1] - mean2;
		newval2 = newval2 * newval2;
		prestandard2 += newval2;
	}
	prestandard2 = sqrt((prestandard2 / 10));

	printf("\nAverage time for static scheduling: %lf", (mean2));
	printf("\nStandard deviation for static scheduling: %lf", prestandard2);
	printf("\n\n");
}




// this function repeats dynamic implementation in dynamicParallelOutput() 10 times to find 
// average time to complete, and standard deviation of times. this is used specificly for 
// the report
void dynamicParallelTest(){

	for (int repetitions=0; repetitions<10; repetitions++) {

	gettimeofday(&tim, NULL);
	double t5=tim.tv_sec+(tim.tv_usec/1000000.0);


	dynamicParallelOutput();


	gettimeofday(&tim, NULL);
	double t6=tim.tv_sec+(tim.tv_usec/1000000.0);
	printf("%.6lf seconds elapsed\n", t6-t5);

	mean3 += (t6 - t5);
	values3[repetitions] = (t6 - t5);
	}

	mean3 = mean3 / 10;

	float prestandard3 = 0;
	for (int z=0;z<10;z++) {
		float newval3 = 0;
		newval3 = values3[z] - mean3;
		newval3 = newval3 * newval3;
		prestandard3 += newval3;
	}
	prestandard3 = sqrt((prestandard3 / 10));

	printf("\nAverage time for dynamic scheduling: %lf", (mean3));
	printf("\nStandard deviation for dynamic scheduling: %lf", prestandard3);
	printf("\n\n");
}











	// These 3 functions are used to collect data for timings for the report.
	nonParallelTest();
	staticParallelTest();
	dynamicParallelTest();

	// Uncomment staticParallelOutput to see DavidBlur.ps derived through static parallel computation
	//staticParallelOutput();

	// Uncomment dynamicParallelOutput to see DavidBlur.ps derived through dynamic parallel computation 
	//dynamicParallelOutput();











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