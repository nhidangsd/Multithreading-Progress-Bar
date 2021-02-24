#include <stdio.h> 
#include <stdlib.h> 
#include <wctype.h>
#include <pthread.h> 
#include <sys/stat.h>


/* create thread argument */
typedef struct {
    long *CurrentStatus;
    long InitialValue;
    long TerminationValue;
} PROGRESS_STATUS;


/* A function to get size of the file.*/
long findSize(const char *file_name){

    struct stat statbuf; // declare stat variable
     
    // get the size using stat()
    return (stat(file_name, &statbuf) == 0) ? (long)(statbuf.st_size) : -1;
}


/* A function that is executed as a thread 
when its name is specified in pthread_create() */
void * progress_monitor(void *arg) { 

    PROGRESS_STATUS *prog_stat = (PROGRESS_STATUS *)arg;
    long curPercent = 0; // track current state
    long prevPercent = -1; // keep track of previous state
    long i; // will be used in for loop
    char display; // a character to be printed out

    // Keep checking currentStatus until finished
    while(curPercent < 50){

        // Save the previous state and update the current state
        prevPercent = curPercent;
        curPercent = (*prog_stat->CurrentStatus - prog_stat->InitialValue)*50 / prog_stat->TerminationValue;

        // Print progress bar base on the different between the previous state and current state
        for(i = prevPercent+1; i <= curPercent; i++){
            display = (i % 10) ? '-' : '+'; // Print '+' for every10th
            printf("%c", display);
            fflush(stdout);
        }

    }
    printf("\n");
    return NULL; 

} 


/* A function that performs Word counting and calls pthread to 
display progress bar while reading the file */
long wordcount(char* file_name){
    
    /* Open the file & handle error if there is any */
    FILE *fp;
    if ((fp = fopen(file_name, "r")) == NULL){
        perror("error: could not open file\n");
        exit(1);
    }


    /* Establish shared data to communicate with pThread */
    PROGRESS_STATUS prog_stat;

    // Allocate the memory for CurrentStatus and handle error if there is any
    prog_stat.CurrentStatus = (long*) malloc(sizeof(long));
    if( prog_stat.CurrentStatus == NULL){
        printf("error: FAIL TO ALLOCATE MEMORY\n");
        exit(1);
    }

    // Initialize prog_stat
    *prog_stat.CurrentStatus = prog_stat.InitialValue = 0;
    prog_stat.TerminationValue = findSize(file_name);

    /* Create pThread to handle Progress Bar display & handle error if there is any */
    int status;
    pthread_t thread_id; 
    status = pthread_create(&thread_id, NULL, progress_monitor, &prog_stat); 
    if(status) {
        printf("error: pthread_create, thread_id: %d\n", status);
        exit(1);
    };


    /* Count the total words in the file */
    long total_words = 0;
    char c;
    int isNewWord = 0;

    // Read and Count 1 char at a time until reaching the end of file 
    while((c=getc(fp)) != EOF){

        // Update a currentStats from main thread
        *prog_stat.CurrentStatus += 1;
        
        // Increment word counter by 1 only if its a new word and not a whitespace
        if(isspace(c)){
            isNewWord = 0;
        }else if(isNewWord == 0){
            isNewWord = 1;
            total_words++;
        }

    }

    // Wait for pthread to finish before return the counter
    pthread_join(thread_id, NULL);
    
    // Close the fptr
    fclose(fp);

    return total_words; 
}


/* Program Driver */
int main(int argc, char **argv) { 

    // Check if we have enough arguments
    if(argc < 2){
        printf("error: no file specified\n");
        return 1;
    }else if(argc > 2){
        printf("error: cannot process more than 1 file at a time\n");
        return 1;
    }

    // Call wordcount function to count total total words in the file
    long total_words = wordcount(argv[1]);

    // Print result from wordcount
    printf("There are %ld words in %s.\n", total_words, argv[1]);
    return 0;
}
