#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAXSTR 255
#define MAXARG 5

//Done by Adam Shandi and Nazim Zerrouki

char** split(char cmd[], char* file);
void nestedFork(char mash1[], char mash2[], char mash3[], char** ptr1, char** ptr2, char** ptr3);
double totalTime(int* process_times);
void printStatus(char cmd[], int process_num, int time);

int main(int argc, char* argv[]) {
	char mash1[MAXSTR];
	char mash2[MAXSTR];
	char mash3[MAXSTR];
	char file[MAXSTR];

	printf("mash1->");
	fgets(mash1, MAXSTR, stdin);
	strtok(mash1, "\n");
	char cmd1[MAXSTR];
	strcpy(cmd1, mash1);
	printf("The 1st command is %s\n", mash1);

	printf("mash2->");
	fgets(mash2, MAXSTR, stdin);
	strtok(mash2, "\n");
	char cmd2[MAXSTR];
	strcpy(cmd2, mash2);
	printf("The 2nd command is %s\n", mash2);

	printf("mash3->");
	fgets(mash3, MAXSTR, stdin);
	strtok(mash3, "\n");
	char cmd3[MAXSTR];
	strcpy(cmd3, mash3);
	printf("The 3rd command is %s\n", mash3);

	printf("file->");
	fgets(file, MAXSTR, stdin);
	strtok(file, "\n");
	printf("The file is %s", file);
	
	char** ptr1 = split(mash1, file);
	char** ptr2 = split(mash2, file);
	char** ptr3 = split(mash3, file);
	printf("\n");
	nestedFork(cmd1, cmd2, cmd3, ptr1, ptr2, ptr3);
	return 0;	
}

char** split(char cmd[], char* file) {
	char* ptr = strtok(cmd, " ");
	char** array = malloc((MAXARG + 2) * sizeof(char*));
	int i = 0;
	while (ptr != NULL) {
		array[i++] = ptr;
		//printf("%s", array[i-1]);
		ptr = strtok(NULL, " ");
	}
	
	array[i] = file;
	strtok(array[i], "\n");
	array[i+1] = NULL;
	return array;
}

void nestedFork(char mash1[], char mash2[], char mash3[], char** ptr1, char** ptr2, char** ptr3) {
	pid_t p1, p2, p3;
	pid_t* p_threads = malloc(3 * sizeof(pid_t));
	int* process_times = malloc(3 * sizeof(int));
	int status;
	p1 = fork();
	int i = 3;
	int k = 3;
	if (p1 < 0) exit(1);
	if (p1 == 0) {
		if (strlen(mash1) > 0) {
			if (execvp(ptr1[0], ptr1) == -1) {
				printf("CMD1: [SHELL 1] STATUS CODE = -1\n");
				exit(-1);
			}
		}
		else if (strcmp(mash1, "\n") == 0){
			k--;
			i--;
		}
	}
	if (p1 > 0) {	//parent forks
		p2 = fork();
		if (p2 < 0) exit(1);
		if (p2 == 0) { //child code
			if (strlen(mash2) > 0) {
				if (execvp(ptr2[0], ptr2) == -1) {
					printf("CMD2: [SHELL 2] STATUS CODE = -1\n"); //Handles a dynamic number of arguments by storing all strings inside a 2D array
					exit(-1);
				}
			}
			else if (strcmp(mash2, "\n") == 0) {
				k--;
				i--;
			}
		}
		if (p2 > 0) {
			p3 = fork();
			if (p3 < 0) exit(1);
			if (p3 == 0) {
				if (strlen(mash3) > 0) {
					if (execvp(ptr3[0], ptr3) == -1) {
						printf("CMD3: [SHELL 3] STATUS CODE = -1\n");
						exit(-1);
					
					}
				}
				else if (strcmp(mash3, "\n") == 0) {
					k--;
					i--;
				}
			}
			if (p3 > 0) {
				// Waits for all processes to finish.
				while (i > 0) {
					clock_t begin = clock();
					if (i == 3) printf("First process finished...\n");
					else if (i == 2) printf("Second process finished...\n");
					else printf("Third process finished...\n");
					int pid = wait(&status);
					clock_t end = clock();
					process_times[3-i] = (int)((int)end-begin/CLOCKS_PER_SEC); // Calculates execution time of each process.
					if (p1 == pid) p_threads[3-i] = p1; 
					else if (p2 == pid) p_threads[3-i] = p2; 
					else if (p3 == pid) p_threads[3-i] = p3; 
					i--;
					
				}
				
			}
		}
	}	
	// EC1: Sorts through processes so that commands 1-3 can be displayed in order with their correct execution times. 
	for (int i = 0; i < k; i++) {
		for (int j = 0; j < k; j++) {
			if (p_threads[j] == p1 && i == 0) {
				printStatus(mash1, 1, process_times[j]);
			}
			else if (p_threads[j] == p2 && i == 1) {
				printStatus(mash2, 2, process_times[j]);
			}
			else if (p_threads[j] == p3 && i == 2) {
				printStatus(mash3, 3, process_times[j]);
			}
		}
	}
	printf("Child process ids: %d %d %d\n", p1, p2, p3);
	int total_time = totalTime(process_times);
	printf("Total elapsed time: %dms\n", total_time);
}



void printStatus(char cmd[], int process_num, int time) {
	printf("-----CMD %d: %s", process_num, cmd);
	for (int i = 0; i < 80-12-strlen(cmd); i++) {
		printf("-");
	}
	printf("\nResult took: %dms\n", time);
}


double totalTime(int* process_times) {
	int max = 0;
	for (int i = 0; i < 3; i++) {
		if (max <= process_times[i]) max = process_times[i];
	}
	return max;
}



