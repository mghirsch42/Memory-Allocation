#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>

#define MAXPROCESSES 10
#define MAX_BUFFER 1024
#define SEPARATORS " \t\n"

// Helper Method Declarations
void request(char *a, int n, int algo, char *mem[]); 
int firstfit(int n, char *mem[]);
int nextfit(int n, char *mem[]);
int bestfit(int n, char *mem[]);
void release(char *a, char *mem[]);
void available(char *mem[]);
void assigned(char *mem[]);
void find(char *a, char *mem[]);

// Total amount of memory
int total;
// File from which to read commands
FILE *fp;
// Keep track of the last index for nextfit algorithm
int lastindex;


/**
 * Main Method
 */
int main(int argc, char **argv) {
	
	if(!argv[2] || !argv[3]) {
                printf("Please fix it\n");
        }

        // Variables of inputs
        total = atoi(argv[2]);
        char *file = argv[3];

	// Array representing memory
	char *mem[total];
	for (int i = 0; i < total; i++) {
		mem[i] = malloc(sizeof(char));
		mem[i] = NULL;
	}


	// Sanity check print
        printf("Memory: %d\n", total);
        printf("File: %s\n", file);

        // Open file
        if(!(fp = fopen(file, "r"))) {
                printf("Error opening file\n");
        }



	// Set the algorithm based
	int algo = 0;

	if(argv[1]) {
		if(!strcmp(argv[1], "FIRSTFIT")) {
			algo = 1;	
		}
		else if(!strcmp(argv[1], "NEXTFIT")) {
			algo = 2;
		}
		else if(!strcmp(argv[1], "BESTFIT")) {
			algo = 3;
		}
		else if(!strcmp(argv[1], "BUDDY")) {
			algo = 4;
		}
	} 
	else {
		printf("No arguments passed.\n");
	}

	// Prep input

	// Line buffer
        char linebuf[1000];

        // Read through file
        while(fgets(linebuf, 1000, fp)) {
		
		printf("%s", linebuf);

		char *linetoks[1000];
		char **linetok = linetoks;
		*linetok++ = strtok(linebuf, SEPARATORS);

		
                while((*linetok++ = strtok(NULL, SEPARATORS)));		

		// Process the line if it isn't a comment
		if(strcmp(linetoks[0], "#")) {
			if(!strcmp(linetoks[0], "REQUEST")) {
				request(linetoks[1],atoi(linetoks[2]), algo, mem);		
			}
			else if (!strcmp(linetoks[0], "RELEASE")) {
				release(linetoks[1], mem);
			}
			else if(!strcmp(linetoks[0], "LIST")  &&
				!strcmp(linetoks[1], "ASSIGNED")) {
				
				assigned(mem);
			}
			else if(!strcmp(linetoks[0], "LIST") &&
				!strcmp(linetoks[1], "AVAILABLE")) {
				available(mem);
			}
			else if(!strcmp(linetoks[0], "FIND")) {
				find(linetoks[1], mem);
			}

		}
        }
	
	// Close file
        fclose(fp);

}

/* Returns the index of the first fit.
   If no space found, returns -1.
   */
int firstfit(int n, char *mem[]) {


        int size = 0;
        int index = -1; 


        for (int i = 0; i < total; i++) {
                // We found empty memory
        
		if(!mem[i]) {
                        // is this the first space? Then save the index
                        if(index == -1) {
                                index = i;
                        }   
                        size++;

                        if(size >= n) {
                                return index;
                        }   
                }   
                else { // otherwise, this space sucks and reset
                        size = 0;
                        index = -1; 
                }   
        }
	return -1;
}

/* Returns the index of the next fit.
   If no space found, returns -1.
   */
int nextfit(int n, char *mem[]) {
	
        int size = 0;
        int index = -1; 
        for (int i = lastindex; i < total; i++) {

                // We found empty memory
                if(!mem[i]) {
                        // is this the first space? Then save the index
                        if(index == -1) {
                                index = i;
                        }   
                        size++;

                        if(size >= n) {
                                lastindex = index;
				return index;
				
                        }   
                }   
                else { // otherwise, this space sucks and reset
                        size = 0;
                        index = -1; 
                }   
        }   
	for (int i = 0; i < lastindex; i++) {

                // We found empty memory
                if(!mem[i]) {
                        // is this the first space? Then save the index
                        if(index == -1) {
                                index = i;
                        }   
                        size++;

                        if(size >= n) {
				lastindex = index;
                                return index;
                        }   
                }   
                else { // otherwise, this space sucks and reset
                        size = 0;
                        index = -1; 
                }   
	}
	
}

/* Returns the index of the best fit.
   If no space found, returns -1.
   */
int bestfit(int n, char *mem[]) {
	int bestfit = -1;
	int bestsize = 0;
	int index = -1;
	int size = 0;

	for (int i = 0; i < total; i++) {
		
                // We found empty memory
                if(!mem[i]) {
			// is this the first space? Then save the index
                        if(index == -1) {
                                index = i;
                        }   
                        size++;
                 
                        if(size >= n) {
                                if( abs(size-n) < abs(bestsize-n)) {
					bestfit = index;
					bestsize = size;
				}
                        }
		}
                else if(index != -1 && size != 0){ // done with this chunk (if any chunk)
                        size = 0;
                        index = -1; 
                }   
	}
	return bestfit;
}

/* Returns the index of the buddy fit.
   If no space found, returns -1.
   */
int buddyfit(int n, char *mem[]) {
	
	int chunk = total;
	while(chunk >= n && chunk/2>=n) {
		chunk = chunk / 2;
	}
	
	printf("chunk: %d\n", chunk);
	for(int i = 0; i < total/chunk; i++) {
		for(int j = i*chunk; j < (i+1)*chunk; j++) {
			if(mem[j]) break;
			if(j == (i+1)*chunk-1) return i*chunk;
		}
	}


	return -1;
}

/* Allocates n bytes for a process named a */
void request(char* a, int n, int alg, char* mem[]) {
	

	int fit = 0;

	switch(alg) {
		case 1: 
			fit = firstfit(n, mem);
			break;
		case 2: ;
			fit = nextfit(n, mem);
			break;
		case 3: ;
			fit = bestfit(n, mem);
			break;
		case 4: ;
			fit = buddyfit(n, mem);
			break;
		default:
			printf("Unacceptable algorithm %d\n", alg);

	}
        
        if(fit == -1) {
                printf("FAIL REQUEST %s %d\n", a, n);
        } 
	else { 
	        for (int i = fit; i < fit+n; i++) {
			mem[i]=malloc(sizeof(char));
			strcpy(mem[i], a);
	
		}
		printf("ALLOCATED %s %d\n", a, n);
	}
}

/* 
 * Clears the memory of the requested process
 */
void release(char *a, char *mem[]) {

	int size = 0;
	int start = -1;

        // Free the space in the memory array
        for (int i = 0; i < total; i++) {
                if(mem[i]) {
			// If this part matches
			if(!strcmp(mem[i], a)){
				if(start == -1) {
					start = i;
				}
                		mem[i] = malloc(sizeof(char));
				mem[i] = NULL;
				size++;
			}	
		}
	}
	if(start != -1) {
		printf("FREE %s %d %d\n", a, size, start); 
	}
	else {
		printf("FAIL RELEASE %s\n", a);
	}
}

/**
 * Lists the memory that is available
 */
void available(char *mem[]) {
        
	int full = 0;	
	int index = -1;
        int size = 0;

        for (int i = 0; i < total; i++) {
                // we have an empty space
                if(!mem[i]) {
                        if(index == -1) {
                                index = i;
                        }
                        size++;
                }
		// we have a full space
                else {
			// is this the same
                        if(index != -1) {
                                printf("(%d, %d)\n", size, index);
                        }
                        index = -1;
                        size = 0;
			full++;
                }
        }
	if(index != -1) {
		printf("(%d, %d)\n", size, index);
		size = 0;
		index = -1;
	}
	if(full >= total-1) {
		printf("FULL\n");
	}
}

/**
 * Lists the memory that is assigned
 */
void assigned(char *mem[]) {

	int full= 0;
	char *name;
	int start = -1;
	int size = 0;

	for (int i = 0; i < total; i++) {
		if(mem[i]) {
			// We have a new process after not having a process
			if(start == -1) {
				name = mem[i];
				start = i;
				size=1;
				full = 1;
			}
			// We are continuing the same process
			else if(!strcmp(mem[i], name)) {
				size++;
			}
			else {	
				printf("(%s, %d, %d)\n", name, size, start);
				name = mem[i];
				start = i;
				size = 1;
			}
		}
		// There is nothing in that spot at memory
		else {
			if(start != -1) {
				printf("(%s, %d, %d)\n", name, size, start);
				name = NULL;
				start = -1;
				size = 0;
			}
		}
	}
	
	if(start != -1) {
		printf("(%s, %d, %d)\n", name, size, start);
	}

	if(!full) {
		printf("EMPTY\n");	
	}

}

/**
 * Print the location of the process
 */
void find(char* a, char *mem[]) {
	int start = -1;
	int size = 0;
	
	for (int i = 0; i < total; i++) {
		if(mem[i]) {
			if(!strcmp(mem[i], a)) {
				if(start == -1) start = i;
				size++;
			}
		}
	}
	if(start != -1) {
		printf("(%s, %d, %d)\n", a, size, start);
	}
	else {
		printf("NO PROCESS %s\n", a);
	}
}
