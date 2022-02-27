/* Program to do "calculations" on numeric CSV data files.
   Skeleton program written by Alistair Moffat, ammoffat@unimelb.edu.au,
   September 2020, with the intention that it be modified by students
   to add functionality, as required by the assignment specification.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <math.h>
#include <assert.h>

/* these #defines provided as part of the initial skeleton */

#define MAXCOLS	20	/* maximum number of columns to be handled */
#define MAXROWS	999	/* maximum number of rows to be handled */
#define LABLEN  20	/* maximum length of each column header */
#define LINELEN 100	/* maximum length of command lines */

#define ERROR	(-1)	/* error return value from some functions */

#define O_NOC	'-'	/* the "do nothing" command */
#define O_IND	'i'	/* the "index" command */
#define O_ANA 	'a'	/* the "analyze" command */
#define O_DPY 	'd'	/* the "display" command */
#define O_PLT 	'p'	/* the "plot" command */
#define O_SRT 	's'	/* the "sort" command */

#define CH_COMMA ','	/* comma character */
#define CH_CR    '\r'	/* pesky CR character in DOS-format files */
#define CH_NL    '\n'	/* newline character */

/* if you wish to add further #defines, put them below this comment */
#define VF 8   // amount of space taken by each csv value, one space + %7.1f 
#define NBANDS 10	// number of bands
#define OFFSET 10e-6	// offset value for min and max
#define MAX_ELEM 60		// maxn width of bar

/* and then, here are some types for you to work with */
typedef char head_t[LABLEN+1];

typedef double csv_t[MAXROWS][MAXCOLS];

/****************************************************************/

/* function prototypes */

void get_csv_data(csv_t D, head_t H[],  int *dr,  int *dc, int argc,
	char *argv[]);
void error_and_exit(char *msg);
void print_prompt(void);
int  get_command(int dc, int *command, int ccols[], int *nccols);
void handle_command(int command, int ccols[], int nccols,
			csv_t D, head_t H[], int dr, int dc);
void do_index(csv_t D, head_t H[], int dr, int dc, int ccols[], int nccols);

/* add further function prototypes below here */
void do_analyze(csv_t D, head_t H[], int dr, int dc, int ccols[], int nccols);
int is_sorted(csv_t D, int dr, int c);
void do_display(csv_t D, head_t H[], int dr, int dc, int ccols[], int nccols);
void print_layered_headings(head_t H[], int ccols[], int nccols);
int same_values(csv_t D, int crow, int trow, int ccols[], int nccols);
void print_display_row(csv_t D, int row, int insts, int ccols[], int nccols);

void do_sort(csv_t D, head_t H[], int dr, int dc, int ccols[], int nccols);
int row_cmp(csv_t D, int row1, int row2, int ccols[], int nccols);
void row_swap(csv_t D, int row1, int row2, int dc);

void do_plotting(csv_t D, head_t H[], int dr, int dc, int ccols[], int nccols);
void fill_bcounts(csv_t D, int bcounts[][NBANDS], double bvalues[], int dr, 
int ccols[], int nccols);
int scale_factor(int bcounts[][NBANDS]);
void print_histogram(int bcounts[][NBANDS], double bvalues[], 
double min, int scale, int ccols[], int nccols);


/****************************************************************/

/* main program controls all the action
*/
int
main(int argc, char *argv[]) {

	head_t H[MAXCOLS];	/* labels from the first row in csv file */
	csv_t D;		/* the csv data stored in a 2d matrix */
	int dr=0, dc=0;		/* number of rows and columns in csv file */
	int ccols[MAXCOLS];
	int nccols;
	int command;

	/* this next is a bit of magic code that you can ignore for
	   now, it reads csv data from a file named on the
	   commandline and saves it to D, H, dr, and dc
	   */
	get_csv_data(D, H, &dr, &dc, argc, argv);
	
	/* ok, all the input data has been read, safe now to start
	   processing commands against it */

	print_prompt();
	while (get_command(dc, &command, ccols, &nccols) != EOF) {
		handle_command(command, ccols, nccols,
			D, H, dr, dc);
		print_prompt();
	}

	/* all done, so pack up bat and ball and head home */
	printf("\nTa daa!!!\n");
	return 0;
}

/****************************************************************/

/* prints the prompt indicating ready for input
*/
void
print_prompt(void) {
	printf("> ");
}

/****************************************************************/

/* read a line of input into the array passed as argument
   returns false if there is no input available
   all whitespace characters are removed
   all arguments are checked for validity
   if no arguments, the numbers 0..dc-1 are put into the array
*/
int
get_command(int dc, int *command, int columns[], int *nccols) {
	int i=0, c, col=0;
	char line[LINELEN];
	/* comand is in first character position */
	if ((*command=getchar()) == EOF) {
		return EOF;
	}
	/* and now collect the rest of the line, integer by integer,
	   sometimes in C you just have to do things the hard way */
	while (((c=getchar())!=EOF) && (c!='\n')) {
		if (isdigit(c)) {
			/* digit contributes to a number */
			line[i++] = c;
		} else if (i!=0)  {
			/* reached end of a number */
			line[i] = '\0';
			columns[col++] = atoi(line);
			/* reset, to collect next number */
			i = 0;
		} else {
			/* just discard it */
		}
	}
	if (i>0) {
		/* reached end of the final number in input line */
		line[i] = '\0';
		columns[col++] = atoi(line);
	}

	if (col==0) {
		/* no column numbers were provided, so generate them */
		for (i=0; i<dc; i++) {
			columns[i] = i;
		}
		*nccols = dc;
		return !EOF;
	}

	/* otherwise, check the ones that were typed against dc,
	   the number of cols in the CSV data that was read */
	for (i=0; i<col; i++) {
		if (columns[i]<0 || columns[i]>=dc) {
			printf("%d is not between 0 and %d\n",
				columns[i], dc);
			/* and change to "do nothing" command */
			*command = O_NOC;
		}
	}
	/* all good */
	*nccols = col;
	return !EOF;
}

/****************************************************************/

/* this next is a bit of magic code that you can ignore for now
   and that will be covered later in the semester; it reads the
   input csv data from a file named on the commandline and saves
   it into an array of character strings (first line), and into a
   matrix of doubles (all other lines), using the types defined
   at the top of the program.  If you really do want to understand
   what is happening, you need to look at:
   -- The end of Chapter 7 for use of argc and argv
   -- Chapter 11 for file operations fopen(), and etc
*/
void
get_csv_data(csv_t D, head_t H[],  int *dr,  int *dc, int argc,
		char *argv[]) {
	FILE *fp;
	int rows=0, cols=0, c, len;
	double num;

	if (argc<2) {
		/* no filename specified */
		error_and_exit("no CSV file named on commandline");
	}
	if (argc>2) {
		/* confusion on command line */
		error_and_exit("too many arguments supplied");
	}
	if ((fp=fopen(argv[1], "r")) == NULL) {
		error_and_exit("cannot open CSV file");
	}

	/* ok, file exists and can be read, next up, first input
	   line will be all the headings, need to read them as
	   characters and build up the corresponding strings */
	len = 0;
	while ((c=fgetc(fp))!=EOF && (c!=CH_CR) && (c!=CH_NL)) {
		/* process one input character at a time */
		if (c==CH_COMMA) {
			/* previous heading is ended, close it off */
			H[cols][len] = '\0';
			/* and start a new heading */
			cols += 1;
			len = 0;
		} else {
			/* store the character */
			if (len==LABLEN) {
				error_and_exit("a csv heading is too long");
			}
			H[cols][len] = c;
			len++;
		}
	}
	/* and don't forget to close off the last string */
	H[cols][len] = '\0';
	*dc = cols+1;

	/* now to read all of the numbers in, assumption is that the input
	   data is properly formatted and error-free, and that every row
	   of data has a numeric value provided for every column */
	rows = cols = 0;
	while (fscanf(fp, "%lf", &num) == 1) {
		/* read a number, put it into the matrix */
		if (cols==*dc) {
			/* but first need to start a new row */
			cols = 0;
			rows += 1;
		}
		/* now ok to do the actual assignment... */
		D[rows][cols] = num;
		cols++;
		/* and consume the comma (or newline) that comes straight
		   after the number that was just read */
		fgetc(fp);
	}
	/* should be at last column of a row */
	if (cols != *dc) {
		error_and_exit("missing values in input");
	}
	/* and that's it, just a bit of tidying up required now  */
	*dr = rows+1;
	fclose(fp);
	printf("    csv data loaded from %s", argv[1]);
	printf(" (%d rows by %d cols)\n", *dr, *dc);
	return;
}
 
/****************************************************************/

void
error_and_exit(char *msg) {
	printf("Error: %s\n", msg);
	exit(EXIT_FAILURE);
}

/****************************************************************/

/* the 'i' index command
*/
void
do_index(csv_t D, head_t H[], int dr, int dc,
		int ccols[], int nccols) {
	int i, c;
	for (i=0; i<nccols; i++) {
		c = ccols[i];
		printf("    column %2d: %s\n", c, H[c]);
	}
}


/*****************************************************************
******************************************************************
Below here is where you do most of your work, and it shouldn't be
necessary for you to make any major changes above this point (except
for function prototypes, and perhaps some new #defines).
Below this point you need to write new functions that provide the
required functionality, and modify function handle_command() as you
write (and test!) each one.
Tackle the stages one by one and you'll get there.
Have Fun!!!
******************************************************************
*****************************************************************/

/* this function examines each incoming command and decides what
   to do with it, kind of traffic control, deciding what gets
   called for each command, and which of the arguments it gets
*/
void
handle_command(int command, int ccols[], int nccols,
			csv_t D, head_t H[], int dr, int dc) { 
	if (command==O_NOC) {
		/* the null command, just do nothing */
	} else if (command==O_IND) {
		do_index(D, H, dr, dc, ccols, nccols);
	} else if (command==O_ANA) {
		do_analyze(D, H, dr, dc, ccols, nccols);
	} else if (command==O_DPY) {
		do_display(D, H, dr, dc, ccols, nccols);
	} else if (command==O_SRT) {
		do_sort(D, H, dr, dc, ccols, nccols);
	} else if (command==O_PLT) {
		do_plotting(D, H, dr, dc, ccols, nccols);
	} else {
		printf("command '%c' is not recognized"
			" or not implemented yet\n", command);
	}
	return;
}

/* the 'a' command 
   output the max, min, avg for each of the selected columns, and also the 
   median value for any sorted columns */
void
do_analyze(csv_t D, head_t H[], int dr, int dc, int ccols[], int nccols) {
	int i, j, c, sorted;
	double max, min, tot;
	for (i=0; i<nccols; i++) {
		c = ccols[i];
		max = min = D[0][c];  // initialise max and min to value in the 1st row
		tot = 0;
		sorted = is_sorted(D, dr, c);	// check if the selected col is sorted
		for (j=0; j<dr; j++) {
			if (D[i][c] > max) {	// a bigger value is found
				max = D[j][c];	// replace existing max with new max 
			} else if (D[j][c] < min) {		// a smaller value is found
				min = D[j][c];	// replace existing min with new min
			} 
			tot += D[j][c];
		}
		printf("          %7s", H[c]);	// column heading 
		if (sorted) {	// note the fact 
			printf(" (sorted)");
		}
		printf("\n");
		printf("    max = %7.1f\n", max);
		printf("    min = %7.1f\n", min);
		printf("    avg = %7.1f\n", tot/dr);
		if (sorted) {	// compute median if the selected is col is sorted 
			printf("    med = %7.1f\n", D[dr/2][c]);
		}
		printf("\n");
	} 
}

/* return 1 if the column c is sorted, and 0 otherwise */
int is_sorted(csv_t D, int dr, int c) {
	int i;
	for (i=0; i<dr-1; i++) {
		if (D[i][c] > D[i+1][c]) {
			return 0;
		}
	}
	return 1;
}

/* the 'd' command 
   print out values from the specified columns, and indicating how many 
   consecutive rows have those values */
void 
do_display(csv_t D, head_t H[], int dr, int dc, int ccols[], int nccols) {
	int i, instances=1;
	print_layered_headings(H, ccols, nccols);
	for (i=0; i<dr; i++) {	
		/* consecutive rows have the same values on selected columns */
		if (i!=dr-1 && same_values(D, i, i+1, ccols, nccols)) {
			instances++;

		/* only print output if the last row contains different values compared 
		to the previous row*/
		} else if (i==dr-1 && instances==1) {	
			print_display_row(D, i, instances, ccols, nccols);

		/* consective rows have different values, so it's time to print the 
		values and the number of instances */
		} else {	
			print_display_row(D, i, instances, ccols, nccols);
			instances = 1;
		}
	}
}

/* print layered headings for the 'd' command */
void 
print_layered_headings(head_t H[], int ccols[], int nccols) {
	int i, j, c;
	for (i=nccols-1; i>=0; i--) {
		c = ccols[i];
		/* output column headings are right-aligned over the numbers 
		they refer to */
		for (j=0; j<i*VF; j++) {		 
			printf(" ");
		}
		printf("%8s\n", H[c]);
	}
}

/* return 1 if the two rows have the same values for the selected columns, and 
0 otherwise */
int 
same_values(csv_t D, int crow, int trow, int ccols[], int nccols) {
	int i, c; 
	for (i=0; i<nccols; i++) {	// compare each selected column
		c = ccols[i];
		if (D[crow][c]!=D[trow][c]) {
			return 0;
		}
	}
	return 1;
}

/* print one row of output for the 'd' command */
void 
print_display_row(csv_t D, int row, int insts, int ccols[], int nccols) {
	int i, c;
	for (i=0; i<nccols; i++) {
		c = ccols[i];
		printf(" %7.1f", D[row][c]);
	}
	printf("    ( %d instances)\n", insts);
}

/* the 's' command */
void do_sort(csv_t D, head_t H[], int dr, int dc, int ccols[], int nccols) {
	int i, j, c;
	printf("    sorted by:");
	for (i=0; i<nccols; i++) {	// print selected columns
		c = ccols[i];
		printf(" %s", H[c]);
		if (i!=nccols-1) {
			printf(",");
		}
	}
	printf("\n");
	/* use insertion sort to sort the csv rows on selected columns */
	for (i=1; i<dr; i++) {
		for (j=i-1; j>=0 && row_cmp(D, j+1, j, ccols, nccols)==-1; j--) {
			row_swap(D, j+1, j, dc);	// swap values if out of order
		}
	}
}


/* returns -1 if row1 should come before row2, 
   returns 0 if row 1 and row 2 if all relevant columns are tied,
   returns 1 if row 1 should come after row 2 */
int row_cmp(csv_t D, int row1, int row2, int ccols[], int nccols) {
	int i, c;
	for (i=0; i<nccols; i++) {
		c = ccols[i];
		if (D[row1][c] < D[row2][c]) {	// row1 should come before row2
			return -1;
		} else if (D[row1][c] > D[row2][c]) {	// row1 should come after row2
			return 1;
		}
	}
	return 0;	// the two rows have the same values, original order is retained
}

/* swap the values of all columns for the two input rows */
void row_swap(csv_t D, int row1, int row2, int dc) {
	int i;
	double tmp;
	for (i=0; i<dc; i++) {	// swap column by column
		tmp = D[row1][i];
		D[row1][i] = D[row2][i];
		D[row2][i] = tmp;
	}
}

/* creates a frequency histogram of all data in the selected columns as a 
   "sideways" bar chart */ 
void 
do_plotting(csv_t D, head_t H[], int dr, int dc, int ccols[], int nccols) {
	int i, j, c, scale, bcounts[MAXCOLS][NBANDS];
	double max, min, bvalues[NBANDS];
	max = min = D[0][ccols[0]];	// initialise max and min

	/* find max and min values of selected columns */
	for (i=0; i<dr; i++) {
		for (j=0; j<nccols; j++) {
			c = ccols[j];
			if (D[i][c]>max) {
				max = D[i][c];
			} else if (D[i][c]<min) {
				min = D[i][c];
			}
		}
	}
	if (min==max) {
		printf("all selected elements are %7.1f\n", min);
		return;
	}
	/* determine and store band values */
	for (i=0; i<NBANDS; i++) {
		bvalues[i] = min + (i+1)*(max+OFFSET-min+OFFSET)/NBANDS;
	}
	fill_bcounts(D, bcounts, bvalues, dr, ccols, nccols);// determine bar width
	scale = scale_factor(bcounts);	// determine scale factor
	print_histogram(bcounts, bvalues, min, scale, ccols, nccols);
}

/* bcounts records the width of the bar for each selected column under each 
   band. this function also initialises all entries to zero and subseqently 
   fills bcounts */
void 
fill_bcounts(csv_t D, int bcounts[][NBANDS], double bvalues[], int dr,
 int ccols[], int nccols) {
	int i, j, k, c, processed;
	/* initialise all entries in bcounts to zero */
	for (i=0; i<MAXCOLS; i++) {	
		for (j=0; j<NBANDS; j++) {
			bcounts[i][j] = 0;
		}
	}
	/* fill out bcounts */
	for (i=0; i<dr; i++) {
		for (j=0; j<nccols; j++) {
			c = ccols[j];
			processed = 0;
			/* check if selected column as already been processed, so as to 
			avoid double counting */
			for (k=j-1; k>=0 && j!=0; k--) {
				if (c==ccols[k]) {
					processed = 1;	
					break;
				}
			}
			/* if the selected column is yet to be processed, add the values 
			in the column to the corresponding bar */
			for (k=0; k<NBANDS && !processed; k++) {
				if (D[i][c]<bvalues[k]) {
					bcounts[c][k]++;
					break;
				}
			}
		}
	}
}
/* returns the integer scale factor */
int 
scale_factor(int bcounts[][NBANDS]) {
	int i, j;
	double max=0;
	for (i=0; i<MAXCOLS; i++) {	// find the width of the longest bar
		for (j=0; j<NBANDS; j++) {
			if (bcounts[i][j]>max) {
				max = bcounts[i][j];
			}
		}
	}
	return (int)ceil(max/MAX_ELEM);	
}

/* prints the histogram */
void 
print_histogram(int bcounts[][NBANDS], double bvalues[NBANDS], double min, 
int scale, int ccols[], int nccols) {
	int i, j, k, c;

	printf("\n %7.1f +\n", min);	// first value on the scale
	for (i=0; i<NBANDS; i++) {
		for (j=0; j<nccols; j++) {
			c = ccols[j];
			printf(" %7d |", c);	// the selected columns
			for (k=0; k<(bcounts[c][i]/scale); k++) {	// the bars
				printf("]");
			}
			printf("\n");
		}
		printf(" %7.1f +\n", bvalues[i]);	// the band values 
	}
	printf("     scale = %d\n", scale);	// the integer scale factor
}