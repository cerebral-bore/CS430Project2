#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

/* 	

*/

double line=1;

// Struct Definitions
typedef struct{
    int r, g, b;
	
}	PixData;
typedef struct{
    double height;
    double width;
	
}	Camera;
typedef struct{
    double center[3];
    double color[3];
    double radius;
	
}	Sphere;
typedef struct{
    double position[3];
    double color[3];
    double normal[3];

}	Plane;
typedef struct{
    int objType;
    Camera cam;
    Sphere sphere;
    Plane plane;
	
}	Object;

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
	fscanf(json, "%lf", &value);
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

void read_scene(char* filename, Object* object) {
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
	
	int i = 0;

	while (1) {
		
		Camera cam;
		Sphere sphere;
		Plane plane;
		
		c = fgetc(json);
		// This if statement trigger would mean the json contains basically nothing
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
				object[i].objType=0;
			} else if (strcmp(value, "sphere") == 0) {
				object[i].objType = 1;
			} else if (strcmp(value, "plane") == 0) {
				object[i].objType = 2;
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
					
					if ((strcmp(key, "width") == 0) || (strcmp(key, "height") == 0) || (strcmp(key, "radius") == 0)) {
						double value = next_number(json);
						// Width in the json means its the camera object
						if (strcmp(key, "width")==0) {
							cam.width=value;
						// Similiarly, height will mean its the camera object as well
						} else if (strcmp(key, "height") == 0) {
							cam.height=value;
						// Once again, radius is exclusive to circle objects
						} else if (strcmp(key, "radius") == 0) {
							sphere.radius=value;
						}
					} else if ((strcmp(key, "color") == 0) || (strcmp(key, "position") == 0) || (strcmp(key, "normal") == 0)) {
						// Depending on the position in the object array, this is store the
						// Color components in the given object data
						double* value = next_vector(json);
						if (strcmp(key, "color") == 0) {
							if (object[i].objType == 1) {
								sphere.color[0] = value[0]; sphere.color[1] = value[1]; sphere.color[2] = value[2];
							}
							if (object[i].objType == 2) {
								plane.color[0] = value[0]; plane.color[1] = value[1]; plane.color[2] = value[2];
							}
						}
						if (strcmp(key, "position") == 0) {
							// If parsed object is...
								// Sphere
							if (object[i].objType == 1) {
								sphere.center[0] = value[0]; sphere.center[1]  = value[1]; sphere.center[2]  = value[2];
							}
								// Plane
							if (object[i].objType == 2) {
								plane.position[0]  = value[0]; plane.position[1]  = value[1]; plane.position[2]  = value[2];
							}
						}
						// Normal is purely a 'plane' attribute so we only check for plane objType
						if (strcmp(key, "normal") == 0) {
							if (object[i].objType == 2) {
								plane.normal[0]  = value[0]; plane.normal[1]  = value[1]; plane.normal[2]  = value[2];
							}
						}
					// If the property is undefined for this project
					} else {
						fprintf(stderr, "Error: Unknown property, \"%s\", on line %d.\n", key, line);
					}
					skip_ws(json);
				// If values aren't properly split up through comma's
				} else {
					fprintf(stderr, "Error: Unexpected value on line %d\n", line);
					exit(1);
				}
				// Store the final objects in the loop iteration
				object[i].cam=cam;
				object[i].sphere=sphere;
				object[i].plane=plane;
			}
			
			skip_ws(json);
			c = next_c(json);
			
			if (c == ',') {
				// noop
				skip_ws(json);
			// Waiting for the EOF of the JSON
			} else if (c == ']') {
				fclose(json);
				return;
			} else {
				fprintf(stderr, "Error: Expecting ',' or ']' on line %d.\n", line);
				exit(1);
			}
		}
		// This will iterate the position in the object array
		// We will continue on until the return is hit when we hit the EOF
		i++;
	}
}


static inline double sqr(double num){ return num*num; }
static inline void normalize(double* v){ double Rd = sqr(v[0])+sqr(v[1])+sqr(v[2]); }


void makeP3PPM(PixData* pixData, int width, int height, char* filename){

	// Opening the file to be written
    FILE *fh = fopen(filename,"w+");
	
	// Writing the P3 ppm Header
    fprintf(fh, "P3\n");
	fprintf(fh, "# Jesus Garcia Project 2\n");
	fprintf(fh, "%d %d\n", width, height);
	fprintf(fh, "255\n");
	
    for(int j = 0; j< width*height; j++){
        fprintf(fh, "%d ", pixData[j].r);
        fprintf(fh, "%d ", pixData[j].g);
        fprintf(fh, "%d \n", pixData[j].b);
    }
	
    fclose(fh);//closing file handle
}

// Finding where, if it exists, sphere intersection
double sphere_insxion(double* Ro, double* Rd, double rad, double* center ){

    // Mathematical functions as given by the Raycasting pdf
    double alpha = sqr(Rd[0]) + sqr(Rd[1]) + sqr(Rd[2]);
    double beta = 2 * (Rd[0] * (Ro[0]-center[0]) + Rd[1] * (Ro[1]-center[1]) + Rd[2] * (Ro[2]-center[2]));
    double gamma = sqr(Ro[0]-center[0]) + sqr(Ro[1]-center[1]) + sqr(Ro[2]-center[2]) - sqr(rad);

	// Finding Determinant
    double det = sqr(beta) - 4 * alpha * gamma;

	// Error check if there are no intersections
    if (det < 0) { return -1; }
    det = sqrt(det);

    // Quadratic equations can be + or - so we need both answers
    double t0 = (-beta - det) / (2 * alpha);// -
    if (t0 > 0){ return t0; }
    double t1 = (-beta + det) / (2 * alpha);// +
    if (t1 > 0){ return t1; }

	// Else: Return an error code (-1)
    return -1;
}


double plane_insxion(double* Ro, double* Rd, double* normal, double* position){
    //doing math
    double dist = -(normal[0] * position[0] + normal[1] * position[1] + normal[2] * position[2]); // distance from origin to plane
    double t = -(normal[0] * Ro[0] + normal[1] * Ro[1] + normal[2] * Ro[2] + dist) /
                (normal[0] * Rd[0] + normal[1] * Rd[1] + normal[2] * Rd[2]);

    if (t > 0){ return t; }

	// If there is no intersection
    return -1;
}


void raycast(Object* objects,char* picture_height,char* picture_width,char* output_file){

    int j,y,k=0;
    double cx=0;
    double cy=0;

    for(j=0;j< sizeof(objects);j++){
        if(objects[j].objType == 0){ break; }
    }


    double h = objects[j].cam.height;
    double w = objects[j].cam.width;
    int m = atoi(picture_height);
    int n = atoi(picture_width);
    double pixheight =h/m;
    double pixwidth =w/m;

    PixData p[m*n];

    for(int y=m;y>0;y--){
        for (int x=0;x<n;x++){
            double Ro[3]={0,0,0};
            double Rd[3] = {
				cx - (w/2) + pixwidth * (x + 0.5),
				cy - (h/2) + pixheight * (y + 0.5),
				1
            };
            normalize(Rd);
            double best_t=INFINITY;
            Object object;

            // Loop thru the object array
            for(int i=0;i<sizeof(objects);i++) {
                double t = 0;
				
                switch (objects[i].objType) {
                    case 1:
                        t = sphere_insxion(Ro, Rd, objects[i].sphere.radius, objects[i].sphere.center);
                        break;
                    case 2:
                        t = plane_insxion(Ro, Rd, objects[i].plane.normal, objects[i].plane.position);
                        break;
                    case 0:
                        break;
                    default:
                        exit(-1);
                }

                if(t>0 && t<best_t){
                    best_t=t;
                    object=objects[i];
                }
            }

			if(best_t > 0 && best_t != INFINITY){
				
				if(object.objType==1){
					p[k].r=(int)(object.sphere.color[0]*255);
					p[k].g=(int)(object.sphere.color[1]*255);
					p[k].b=(int)(object.sphere.color[2]*255);
				}
				if(object.objType==2){
					p[k].r=(int)(object.plane.color[0]*255);
					p[k].g=(int)(object.plane.color[1]*255);
					p[k].b=(int)(object.plane.color[2]*255);
				}
			} else{
				
				p[k].r=0;
				p[k].g=0;
				p[k].b=0;
			}
            k++;
        }
    }
    //writing to ppm file
    makeP3PPM(p,atoi(picture_height),atoi(picture_width),output_file);
}

int main(int c, char** argv) {
    Object objects[129];
    read_scene(argv[3],objects);
    raycast(objects,argv[1],argv[2],argv[4]);
    return 0;
}