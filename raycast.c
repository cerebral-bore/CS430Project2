#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 	Jesus Garcia
	Project 2 - Raycast - 10/4/16
	CS430 - Prof. Palmer
	
	"In this project you will write code to raycast mathematical primitives based on a scene input file
	into a pixel buffer. You will then write the pixel buffer to a PPM formatted file using the code
	you wrote in Project 1 (P3 or P6 format).
	
	Your program should be resistant to errors and should not segfault or produce undefined
	behavior. If an error occurs, it should print a message to stderr with “Error:” prefixed to a
	descriptive error message before returning a non-zero error code. I have a test suite designed to
	test the robustness of your program."
	
	Your program (raycast) should have this usage pattern:
		raycast width height input.json output.ppm
		
	• Our	format	is	always	a	list	of	objects;	any	other	input	should	result	in	an	error.
	• The	number	of	objects	in	the	list	will	not	exceed	128;	you	may	allow	more	at	your	
			option.
	• The	first	field	in	an	object	will	be	“type”;	you	may	consider	arbitrary	order	for	this	
			field	at	your	option.
	• Other	fields	may	appear	in	any	order.
	• Vectors	will	always	be	given	as	a	list	of	three	numbers.
	• You	may	assume	that	strings	do	not	have	non-ascii	characters	(e.g.	Unicode)	or	
			escaped	characters.
	• You	may	assume	the	file	itself	is	ASCII	and	not	Unicode
	
*/

// Parser.c
	int line = 1;

	// next_c() wraps the getc() function and provides error checking and line
	// number maintenance
	int next_c(FILE* json) {
	  int c = fgetc(json);
	#ifdef DEBUG
	  printf("next_c: '%c'\n", c);
	#endif
	  if (c == '\n') {
		line += 1;
	  }
	  if (c == EOF) {
		fprintf(stderr, "Error: Unexpected end of file on line number %d.\n", line);
		exit(1);
	  }
	  return c;
	}


	// expect_c() checks that the next character is d.  If it is not it emits
	// an error.
	void expect_c(FILE* json, int d) {
	  int c = next_c(json);
	  if (c == d) return;
	  fprintf(stderr, "Error: Expected '%c' on line %d.\n", d, line);
	  exit(1);    
	}


	// skip_ws() skips white space in the file.
	void skip_ws(FILE* json) {
	  int c = next_c(json);
	  while (isspace(c)) {
		c = next_c(json);
	  }
	  ungetc(c, json);
	}


	// next_string() gets the next string from the file handle and emits an error
	// if a string can not be obtained.
	char* next_string(FILE* json) {
	  char buffer[129];
	  int c = next_c(json);
	  if (c != '"') {
		fprintf(stderr, "Error: Expected string on line %d.\n", line);
		exit(1);
	  }  
	  c = next_c(json);
	  int i = 0;
	  while (c != '"') {
		if (i >= 128) {
		  fprintf(stderr, "Error: Strings longer than 128 characters in length are not supported.\n");
		  exit(1);      
		}
		if (c == '\\') {
		  fprintf(stderr, "Error: Strings with escape codes are not supported.\n");
		  exit(1);      
		}
		if (c < 32 || c > 126) {
		  fprintf(stderr, "Error: Strings may contain only ascii characters.\n");
		  exit(1);
		}
		buffer[i] = c;
		i += 1;
		c = next_c(json);
	  }
	  buffer[i] = 0;
	  return strdup(buffer);
	}

	double next_number(FILE* json) {
	  double value;
	  fscanf(json, "%f", &value);
	  // Error check this..
	  return value;
	}

	double* next_vector(FILE* json) {
	  double* v = malloc(3*sizeof(double));
	  expect_c(json, '[');
	  skip_ws(json);
	  v[0] = next_number(json);
	  skip_ws(json);
	  expect_c(json, ',');
	  skip_ws(json);
	  v[1] = next_number(json);
	  skip_ws(json);
	  expect_c(json, ',');
	  skip_ws(json);
	  v[2] = next_number(json);
	  skip_ws(json);
	  expect_c(json, ']');
	  return v;
	}


	void read_scene(char* filename) {
	  int c;
	  FILE* json = fopen(filename, "r");

	  if (json == NULL) {
		fprintf(stderr, "Error: Could not open file \"%s\"\n", filename);
		exit(1);
	  }
	  
	  skip_ws(json);
	  
	  // Find the beginning of the list
	  expect_c(json, '[');

	  skip_ws(json);

	  // Find the objects

	  while (1) {
		c = fgetc(json);
		if (c == ']') {
		  fprintf(stderr, "Error: This is the worst scene file EVER.\n");
		  fclose(json);
		  return;
		}
		if (c == '{') {
		  skip_ws(json);
		
		  // Parse the object
		  char* key = next_string(json);
		  if (strcmp(key, "type") != 0) {
		fprintf(stderr, "Error: Expected \"type\" key on line number %d.\n", line);
		exit(1);
		  }

		  skip_ws(json);

		  expect_c(json, ':');

		  skip_ws(json);

		  char* value = next_string(json);

		  if (strcmp(value, "camera") == 0) {
		  } else if (strcmp(value, "sphere") == 0) {
		  } else if (strcmp(value, "plane") == 0) {
		  } else {
		fprintf(stderr, "Error: Unknown type, \"%s\", on line number %d.\n", value, line);
		exit(1);
		  }

		  skip_ws(json);

		  while (1) {
		// , }
		c = next_c(json);
		if (c == '}') {
		  // stop parsing this object
		  break;
		} else if (c == ',') {
		  // read another field
		  skip_ws(json);
		  char* key = next_string(json);
		  skip_ws(json);
		  expect_c(json, ':');
		  skip_ws(json);
		  if ((strcmp(key, "width") == 0) ||
			  (strcmp(key, "height") == 0) ||
			  (strcmp(key, "radius") == 0)) {
			double value = next_number(json);
		  } else if ((strcmp(key, "color") == 0) ||
				 (strcmp(key, "position") == 0) ||
				 (strcmp(key, "normal") == 0)) {
			double* value = next_vector(json);
		  } else {
			fprintf(stderr, "Error: Unknown property, \"%s\", on line %d.\n",
				key, line);
			//char* value = next_string(json);
		  }
		  skip_ws(json);
		} else {
		  fprintf(stderr, "Error: Unexpected value on line %d\n", line);
		  exit(1);
		}
		  }
		  skip_ws(json);
		  c = next_c(json);
		  if (c == ',') {
		// noop
		skip_ws(json);
		  } else if (c == ']') {
		fclose(json);
		return;
		  } else {
		fprintf(stderr, "Error: Expecting ',' or ']' on line %d.\n", line);
		exit(1);
		  }
		}
	  }
	}
	
	int errCheck(int args, char *argv[]){
	
	// Initial check to see if there are 3 input arguments on launch
	if (args != 5) {
		fprintf(stderr, "Error: Program requires usage: 'raycast width height input.json output.ppm'");
		exit(1);
	}
	
	// Check the file extension of input and output files
	char *extIn;
	char *extOut;
	if(strrchr(argv[3],'.') != NULL){
		extIn = strrchr(argv[3],'.');
	}
	if(strrchr(argv[4],'.') != NULL){
		extOut = strrchr(argv[4],'.');
	}
	
	// Check to see if the inputfile is in .ppm format
	if(strcmp(extIn, ".json") != 0){
		printf("Error: Input file not a json");
		exit(2);
	}
	
	// Check to see if the outputfile is in .ppm format
	if(strcmp(extOut, ".ppm") != 0){
		printf("Error: Output file not a PPM");
		exit(3);
	}

	return(0);
}

	int main(int args, char** argv){
		errCheck(args, argv);
		
		read_scene(argv[3]);
		return 0;
	}