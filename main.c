#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>  //Header file for sleep(). man 3 sleep for details. 
#include <pthread.h> 
#include <sys/stat.h>


/* create thread argument struct for thr_func() */
typedef struct {
    long *CurrentStatus;
    long InitialValue;
    long TerminationValue;
} PROGRESS_STATUS;

/*function to get size of the file.*/
long int findSize(const char *file_name){

    struct stat statbuf; // declare stat variable
     
    // get the size using stat()
    if(stat(file_name,&statbuf) == 0)
        return (statbuf.st_size);
    else
        return -1;
}
  
// A normal C function that is executed as a thread  
// when its name is specified in pthread_create() 
void * progress_monitor(void *arg) { 

    PROGRESS_STATUS *prog_stat = (PROGRESS_STATUS *)arg;
    long bar, cur_progress;
    char display_bar[50];

    if(*prog_stat->CurrentStatus <= prog_stat->TerminationValue){

        // Calculate the progress for display:
        cur_progress = (*prog_stat->CurrentStatus - prog_stat->InitialValue)*50/ prog_stat->TerminationValue;
        printf("[Progress Monitor] %ld\n", cur_progress);

        int is10thProgress = 0;
        // Display the bar
        for(bar = 0; bar < cur_progress; bar ++){
            
            is10thProgress = (bar + 1)%10 == 0;
            display_bar[bar] = (is10thProgress) ? '+' : '-';

        }

    };
    printf("%s\n", display_bar);
    return NULL; 
} 

long wordcount(char* file_name){
    /* Open a file */
    FILE *fp;
    fp = fopen(file_name, "r");

    if ((fp = fopen(file_name, "r")) == NULL){
        perror("Cannot open file\n");
        exit(1);
    }

    long num_of_words = 0;

    /* Establish shared data to communicate with pThread */

    PROGRESS_STATUS prog_stat;

    //Allocate memory for integer
    prog_stat.CurrentStatus = (long*)malloc(sizeof(long));
    //check allocated memory
    if( prog_stat.CurrentStatus == NULL){
        printf("FAIL TO ALLOCATE MEMORY\n");
        return 0;
    }

    *prog_stat.CurrentStatus = prog_stat.InitialValue = 0;
    prog_stat.TerminationValue = findSize(file_name);

    printf("\n[Init pro_stat] {%ld, %ld, %ld}\n\n", *prog_stat.CurrentStatus, 
    prog_stat.InitialValue,  prog_stat.TerminationValue );


    /* Read and Count 1 char at a time until reaching the end of file */
    while(getc(fp) != EOF){

        // Increment num_of_words by 1
        num_of_words++;
        
        // Update a currentStats from main thread
        *prog_stat.CurrentStatus += 1;
        printf("[Word Count] {%ld, %ld, %ld}\n", *prog_stat.CurrentStatus, 
        prog_stat.InitialValue,  prog_stat.TerminationValue );


        /* Display progress bar */
        int status;
        pthread_t thread_id; 
        // Create pThread & check if there is error
        status = pthread_create(&thread_id, NULL, progress_monitor, &prog_stat); 

        if (status) {
            fprintf(stderr, "error: pthread_create, rc: %d\n", status);
            return EXIT_FAILURE;
        };


        pthread_join(thread_id, NULL);
        printf("[Main Thread After pthread finised] {%ld, %ld, %ld}\n\n", *prog_stat.CurrentStatus, 
        prog_stat.InitialValue,  prog_stat.TerminationValue );
      
    }

    // Close the fptr
    fclose(fp);

    return num_of_words; 

}

int main(int argc, char **argv) { 

    // Check if we have enough arguments
    if(argc != 2){
        printf("Please provide filename");
        return 1;
    }

    // Call wordcount function to count total num of words in tht file
    long total_words = wordcount(argv[1]);

    // Print result from wordcount
    printf("There are %ld words in %s.\n", total_words, argv[1]);

}