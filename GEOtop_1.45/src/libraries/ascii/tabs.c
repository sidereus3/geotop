
/* STATEMENT:
 
 GEOtop MODELS THE ENERGY AND WATER FLUXES AT THE LAND SURFACE
 GEOtop 1.225-9 'Moab' - 24 Aug 2012
 
 Copyright (c), 2012 - Stefano Endrizzi 
 
 This file is part of GEOtop 1.225-9 'Moab'
 
 GEOtop 1.225-9 'Moab' is a free software and is distributed under GNU General Public License v. 3.0 <http://www.gnu.org/licenses/>
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE
 
 GEOtop 1.225-9 'Moab' is distributed as a free software in the hope to create and support a community of developers and users that constructively interact.
 If you just use the code, please give feedback to the authors and the community.
 Any way you use the model, may be the most trivial one, is significantly helpful for the future development of the GEOtop model. Any feedback will be highly appreciated.
 
 If you have satisfactorily used the code, please acknowledge the authors.
 
 */
	
#include "tabs.h"

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

short readline_par(FILE *f, long comment_char, long sepfield_char, long sepvect_char, long maxcharstring, long maxnumvect, long *key, 
				   long *keylength, long *string, long *stringlength, double *number, long *numberlength, short *endoffile){
	
	long i, j, k, h, **string2;
	int c;
	char *keyword;
		
	*endoffile = 0;
	
	for (i=0; i<maxcharstring; i++) {
		key[i] = 0;
		string[i] = 0;
	}
	
	for (i=0; i<maxnumvect; i++) {
		number[i] = (double)number_novalue;
	}
	
	//read first character	
	do{
		c = fgetc(f);
		if(c == -1) *endoffile = 1;
	}while ( *endoffile == 0 && ( c <= 46 || ( c >= 59 && c <= 64 ) || c == 96 || c >= 123 ) && c != comment_char );
	
	//end of file reached 
	if( *endoffile == 1){
		return -1;
		
	//first character is the comment tag
	}else if( c == comment_char ){
		
		//read until end of line
		do{
			c=fgetc(f);
		}while (c != 10 && c != -1);
		
		if(c == -1) *endoffile = 1;
		
		return -1;
		
	}else{
		
		//read keyword
		i=0;//all the keyword
		
		key[i] = (long)c;
		i++;
		
		do{

			do{
				c=fgetc(f);
				if(c == -1) *endoffile = 1;
			}while ( *endoffile == 0 && ( c <= 46 || ( c >= 59 && c <= 64 ) || c == 96 || c >= 123 ) && c != sepfield_char &&  c != 10 );
			
			if (*endoffile == 0 && c != sepfield_char && c != 10) {
				if(i < maxcharstring){
					key[i] = (long)c;
					i++;
				}else if(i == maxcharstring){
					keyword = find_string(key, maxcharstring);
					printf("Warning: Keyword %s has keyword string longer than %ld characters, only the first %ld characters can be read\n",keyword,maxcharstring,maxcharstring);
					free(keyword);
					i++;
				}
			}

		}while (*endoffile == 0 && c != sepfield_char && c != 10);
								
		*keylength = i;
				
		if (*endoffile == 1 || c == 10) {
			
			*number = (double)number_novalue;
			*stringlength = 1;
			string[0] = comment_char;
			
			return 0;
		}
		
		//read argument
			
		i=0;
		
		do{
			
			do{
				c=fgetc(f);
				if(c == -1) *endoffile = 1;
			}while ( *endoffile == 0 && ( c <= 42 || c == 44 || ( c >= 59 && c <= 64 ) || c == 96 || c >= 123 ) && c != 10 && c != sepvect_char && (i > 0 || c != 34) );
			
			if (*endoffile == 0 && c != 10) {
				if(i < maxcharstring){
					string[i] = (long)c;
					i++;
				}else if(i == maxcharstring){
					keyword = find_string(key, *keylength);
					printf("Warning: Keyword %s has argument string longer than %ld characters, only the first %ld characters are read\n",keyword,maxcharstring,maxcharstring);
					free(keyword);
					i++;
				}
			}
			
		}while (*endoffile == 0 && c != 10);
		
		//i now represents the length of the string
		
		//it is a number (the discriminating is the ")
		if( string[0] != 34 ){
			
			//only numbers can be scalar and vectors, check if there are separator characters
			
			//precounting to allocate
			*numberlength = 0;
			for (j=0; j<i; j++) {
				if (string[j] == sepvect_char)  *numberlength = (*numberlength) + 1;
			}
			*numberlength = (*numberlength) + 1;

			string2 = (long**)malloc((*numberlength)*sizeof(long*));
			
			j = 0;//index of character in the string variable
			h = 0;//index of numerical vector
			
			do{
				
				k = 0;//relative length of number string
				while ( string[j+k] != sepvect_char && j+k < i ) {
					k++;
				}
								
				string2[h] = (long*)malloc(k*sizeof(long));
				
				k = 0;//relative length of number string
				while ( string[j+k] != sepvect_char && j+k < i ) {
					string2[h][k] = string[j+k];
					k++;
				}
				
				number[h] = find_number(string2[h], k);
				
				free(string2[h]);
				
				j = j+k+1;
								
				h++;

			}while ( j < i && h < maxnumvect );
			
			if (h == maxnumvect){
				keyword = find_string(key, *keylength);
				printf("Warning: Keyword %s has number of vector components higher than %ld, only the first %ld components are read\n",keyword,maxnumvect,maxnumvect);
				free(keyword);
			}
							
			free(string2);
										
			*stringlength = 1;
			string[0] = comment_char;
						
			return 1;
			
		}else{			
			
			*stringlength = i-1;
			
			*number = (double)number_novalue;
			
			*numberlength = 1;
			
			return 2;
			
		}
	}
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

double find_number(long *vector, long lengthvector){
	
	double N = 0.0, Nexp = 0.0;
	long i, ie, ids, cnt;
	
	//looking for the E or e
	ie=-1;
	i=0;
	do{
		if(vector[i]==69 || vector[i]==101) ie=i;
		i++;
	}while (ie==-1 && i<lengthvector);
	if(ie==-1) ie=lengthvector;
		
	//looking for decimal separator
	ids=-1;
	i=0;
	do{
		if(vector[i]==46) ids=i;
		i++;
	}while (ids==-1 && i<lengthvector);
	if(ids==-1 && ie>-1) ids=ie;
	if(ids==-1) ids=lengthvector;

	
	//integer part
	cnt=0;
	for (i=ids-1; i>=0; i--) {
		if (vector[i]>=48 && vector[i]<=57) {
			N += (double)(vector[i]-48) * pow(10.0, (double)cnt);
			cnt++;
		}
	}
	
	//fractional part
	cnt=-1;
	for (i=ids+1; i<=ie-1; i++) {
		if (vector[i]>=48 && vector[i]<=57) {
			N += (double)(vector[i]-48) * pow(10.0, (double)cnt);
			cnt--;
		}
	}
	
	//exponential part
	cnt=0;
	for (i=lengthvector-1; i>=ie+1; i--) {
		if (vector[i]>=48 && vector[i]<=57) {
			Nexp += (double)(vector[i]-48) * pow(10.0, (double)cnt);
			cnt++;
		}
	}
	
	//sign of the exponential part
	if (ie<lengthvector-1) {
		if (vector[ie+1]==45) Nexp*=(-1.);
	}
	
	//exponential format
	N = N * pow(10.0, Nexp);
	
	//sign
	if (vector[0]==45) N*=(-1.);
	
	return(N);
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

char *find_string(long *vector, long lengthvector){
	
	char *string;
	long i;
	
	string = (char*)malloc((lengthvector+1)*sizeof(char));
	for (i=0; i<lengthvector; i++) {
		string[i] = vector[i];
	}
	string[lengthvector]=0;
	return(string);
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

double *find_number_vector(double *vector, long lengthvector){
	
	double *number_vector;
	long i;
	
	number_vector = (double*)malloc(lengthvector*sizeof(double));
	for (i=0; i<lengthvector; i++) {
		number_vector[i] = vector[i];
	}
	return(number_vector);
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

long *find_string_int(long *vector, long lengthvector){
	
	long *string;
	long i;
	
	string = (long*)malloc(lengthvector*sizeof(long));
	for (i=0; i<lengthvector; i++) {
		string[i] = vector[i+1];
	}
	return(string);
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

short readline(FILE *f, long comment_char, long sep_char, long **string, long *string_length, long *components, long maxcomponents, 
			   long maxstringlength, short *endoffile){
	
	long i, j;
	char *c;
			
	*endoffile = 0;
	
	//initialization of vector that are already allocated
	for (i=0; i<maxcomponents; i++) {
		string_length[i] = 0;
		for (j=0; j<maxstringlength; j++){
			string[i][j] = 0;
		}
	}
	
	//allocate character
	c = (char*)malloc(sizeof(char));

	//read first character	
	do{
		c[0] = fgetc(f);
		if(c[0] == -1) *endoffile = 1;
	}while ( *endoffile == 0 && ( c[0] <= 42 || ( c[0] >= 58 && c[0] <= 64 ) || c[0] == 96 || c[0] >= 123 ) && c[0] != comment_char );
	
	//end of file reached 
	if( *endoffile == 1){
		
		free(c);
		return -1;
				
		//first character is the comment tag
	}else if( c[0] == comment_char ){
				
		//read until end of line
		do{
			c[0]=fgetc(f);
		}while (c[0]!=10 && c[0]!=-1);
		
		if(c[0] == -1) *endoffile = 1;
		
		free(c);
		return -1;
		
	}else{
		
		//read argument
		j=0;
				
		do{
			
			i=0;
			
			if(j==0){
				string[j][i] = c[0];
				i++;
			}
			
			do{
				do{
					c[0]=fgetc(f);
					if(c[0] == -1) *endoffile = 1;
				}while ( *endoffile == 0 && ( c[0] <= 42 || ( c[0] >= 58 && c[0] <= 64 ) || c[0] == 96 || c[0] >= 123 ) && c[0] != 10 && c[0] != sep_char);
				
				if (*endoffile == 0 && c[0] != 10 && c[0] != sep_char) {
					if(i < maxstringlength){
						string[j][i] = c[0];
						i++;
					}
				}
				
			}while (*endoffile == 0 && c[0] != 10 && c[0] != sep_char);
			
			string_length[j] = i;
			
			j++;
									
		}while( *endoffile == 0 && c[0] != 10 && j < maxcomponents );
		
		*components = j;
				
		free(c);
		return 1;
	}
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

char **readline_of_strings(FILE *f, long comment_char, long sep_char, long *components, short *endoffile, short *success){
	
	long i, n;
	long **string, *string_length;
	char **line_of_strings=NULL;
	
	n = (long)max_components;
	string_length = (long*)malloc(n*sizeof(long));
	string = (long**)malloc(n*sizeof(long*));
	
	n = (long)max_string_length;
	for (i=0; i<max_components; i++) {
		string[i] = (long*)malloc(n*sizeof(long));
	}
	
	*success = readline(f, comment_char, sep_char, string, string_length, components, max_components, max_string_length, endoffile);
	
	if(*success == 1){
		
		line_of_strings = (char**)malloc((*components)*sizeof(char*));
		
		for (i=0; i<(*components); i++) {
			line_of_strings[i] = find_string(string[i], string_length[i]);
		}
		
	}
	
	for (i=0; i<max_components; i++) {	
		free(string[i]);
	}
	
	free(string_length);
	free(string);
	
	return line_of_strings;
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

double *readline_of_numbers(FILE *f, long comment_char, long sep_char, long *components, short *endoffile, short *success){
	
	long i, n;
	long **string, *string_length;
	double *line_of_numbers=NULL;
	
	n = (long)max_components;
	string_length = (long*)malloc(n*sizeof(long));
	string = (long**)malloc(n*sizeof(long*));
	
	n = (long)max_string_length;
	for (i=0; i<max_components; i++) {
		string[i] = (long*)malloc(n*sizeof(long));
	}	
	
	*success = readline(f, comment_char, sep_char, string, string_length, components, max_components, max_string_length, endoffile);
		
	if(*success == 1){

		line_of_numbers = (double*)malloc(*components*sizeof(double));
				
		for (i=0; i<(*components); i++) {
			line_of_numbers[i] = find_number(string[i], string_length[i]);
		}		
		
	}
		
	for (i=0; i<max_components; i++) {	
		free(string[i]);
	}
	
	free(string_length);
	free(string);	
	
	return line_of_numbers;
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

char **ReadHeader(FILE *f, char *filename, long *num_cols){
	
	short endoffile, success;
	char **Header;
	
	do{
		Header = readline_of_strings(f, 33, 44, num_cols, &endoffile, &success);
	}while (endoffile==0 && success!=1);
	
	if (endoffile == 1){
		printf("Error!! File %s contains only the header\n",filename);
		t_error("Impossible To Continue");
	}
	
	return Header;
	
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

long *ColumnCoder(char *filename, char **ColDescr, long max_num_cols, char **header, long num_cols_header, FILE *flog){
	
	long *coder, i, j;
	char *lowercaseColDescr;
	
	//allocation
	coder = (long*)malloc(max_num_cols*sizeof(long));
	
	//initialization
	for( i=0; i<max_num_cols; i++){
		coder[i] = -1;
	}
	
	for( i=0; i<max_num_cols; i++){
		lowercaseColDescr = assign_string(ColDescr[i]);
		convert_string_in_lower_case(lowercaseColDescr);
				
		for (j=0; j<num_cols_header; j++) {
			convert_string_in_lower_case(header[j]);	
			
			if (strcmp(lowercaseColDescr, header[j]) == 0 && coder[i] == -1 && strcmp(string_novalue, header[j]) != 0 ) {
				coder[i] = j;
				fprintf(flog,"Column %ld in file %s assigned to %s\n",j+1,filename,ColDescr[i]);
				printf("Column %ld in file %s assigned to %s\n",j+1,filename,ColDescr[i]);
			}else if (strcmp(lowercaseColDescr, header[j]) == 0 && coder[i] != -1) {
				//printf("Keyword %s presented twice in file %s\n",ColDescr[i],filename);
				t_error("Column name DUPLICATED in a comma-separated-value tables!");
			}
		}
		free(lowercaseColDescr);
		
	}	
	
	return coder;
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

long count_lines(char *meteo_file_name, long comment_char, long sep_char){
	
	FILE *f;
	char **header;
	double *line;
	short success, endoffile;
	long  i, components, cont;
	
	f = fopen(meteo_file_name, "r");
	if (f==NULL){
		printf("File %s not existing\n",meteo_file_name);
		t_error("Fatal Error (10)");
	}
	
	//read header line
	do{
		header = readline_of_strings(f, comment_char, sep_char, &components, &endoffile, &success);
	}while(success != 1 && endoffile == 0);
	
	if (success == 1){
		for (i=0; i<components; i++) {
			free(header[i]);
		}
		free(header);
	}
	
	if (endoffile == 1){
		fclose(f);
		return 0;
	}
	
	//count lines
	cont = 0;
	do{
		line = readline_of_numbers(f, comment_char, sep_char, &components, &endoffile, &success);
		if (success == 1){
			free(line);	
			cont++;
		}
	}while( endoffile == 0);
	
	fclose(f);
	return cont;
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

double **read_datamatrix(FILE *f, char *filename, long comment_char, long sep_char, long number_lines, long components_header){
	
	double **data, *line;
	short endoffile, success;
	long i, cont, components;
	
	data = (double**)malloc(number_lines*sizeof(double*));
	
	cont = 0;
	do{
		line = readline_of_numbers(f, comment_char, sep_char, &components, &endoffile, &success);
						
		if (success == 1) {
			data[cont] = (double*)malloc(components_header*sizeof(double));
			if (components <= components_header) {
				for (i=0; i<components; i++) {
					data[cont][i] = line[i];
				}
				for (i=components; i<components_header; i++) {
					data[cont][i] = (double)number_novalue;
				}
			}else {
				for (i=0; i<components_header; i++) {
					data[cont][i] = line[i];
				}
			}
			
			free(line);
			cont++;
			
		}
	}while( cont < number_lines );
	
	return data;
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

double **read_txt_matrix(char *filename, long comment_char, long sep_char, char **Col_Descr, long ncolsCol_Descr, long *nlines, FILE *flog){
	
	/*Read header, and create a **double with the same columns as the header. Then fill with number_absent the columns
	 missing with respect to Col_Descr*/
	
	FILE *f;
	char **Header;
	double **Data, **Dataout;
	long *Coder, ncols, i, j;
	
	*nlines = count_lines(filename, comment_char, sep_char);
	
	f = fopen(filename, "r");
	if (f==NULL){
		printf("File %s not existing\n",filename);
		t_error("Fatale Error (11)");
	}
	Header = ReadHeader(f, filename, &ncols);
	
	Coder = ColumnCoder(filename, Col_Descr, ncolsCol_Descr, Header, ncols, flog);
	Data = read_datamatrix(f, filename, comment_char, sep_char, *nlines, ncols);
	fclose(f);
	
	Dataout = (double**)malloc((*nlines)*sizeof(double*));
	for (i=0; i<(*nlines); i++) {
		Dataout[i] = (double*)malloc(ncolsCol_Descr*sizeof(double));
		for (j=0; j<ncolsCol_Descr; j++) {
			if (Coder[j]!=-1) {
				Dataout[i][j] = Data[i][Coder[j]];
			}else {
				Dataout[i][j] = (double)number_absent;
			}				
		}
		free(Data[i]);
	}
	free(Data);
	free(Coder);
	
	for (j=0; j<ncols; j++) {
		free(Header[j]);
	}
	free(Header);
	
	return Dataout;
	
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

double **read_txt_matrix_2(char *filename, long comment_char, long sep_char, long ncolsCol_Descr, long *nlines){
	
	/*Read header, and create a **double with the same columns as Col_Descr, filling with number_novalue the missing columns*/
	
	FILE *f;
	char **Header;
	double **Dataout;
	long ncols, j;
		
	*nlines = count_lines(filename, comment_char, sep_char);
	
	f = fopen(filename, "r");
	if (f==NULL){
		printf("File %s not existing\n",filename);
		t_error("Fatale Error (13)");
	}	
	
	Header = ReadHeader(f, filename, &ncols);
	Dataout = read_datamatrix(f, filename, comment_char, sep_char, *nlines, ncolsCol_Descr);
	fclose(f);
				
	for (j=0; j<ncols; j++) {
		free(Header[j]);
	}
	free(Header);
	
	return Dataout;
	
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

char *assign_string(char *a){
	char *b;
	int n = 1+strlen(a);
	b = (char*)malloc(n*sizeof(char));
	b = strcpy(b, a);
	return b;
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

void convert_string_in_lower_case(char *s){

	long i, len=strlen(s);

	for(i=0;i<len;i++){
		if (s[i]>=65 && s[i]<=90) s[i] = s[i] + 32;
	}
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
