#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "functions.h"

volatile sig_atomic_t quit = 0; //a flag to quit the progrma
volatile sig_atomic_t stop = 0; //a flag to stop when user is responsing  

void sigint_handle(int signum){
    ///_|> descry: this function will handle the Ctrl+C which is SIGINT
    ///_|> signum: given signal number (SIGINT = 2) 
    ///_|> returning: this function will not return anything
    stop = 1;
    char ans; 
    printf("\nReceived SIGINT. Do you want to quit the program and proceed? (Y/N)\n");
    scanf(" %c", &ans);
    if(ans == 'Y' || ans == 'y'){
        quit = 1;
    }else{
        stop = 0;
    }
} 

void sigtstp_handle(int signum){
    ///_|> descry: this function will handle Ctrl+Z which is SIGTSTP
    ///_|> signum: given signal number (SIGTSTP = 20)
    ///_|> returning: this function will not return anything
    printf("\nIgnored SIGTSTP. The program will not be run in the background.");
    fflush(stdout);
}

void display_graph(int *data, int height, int width, const char *symbol){
    ///_|> descry: this function will display the graph layout of memory graph and cpu graph
    ///_|> data: pointer pointing to either memory data or cpu data
    ///_|> height: given height for the y axis (10)
    ///_|> width: given width for x axis (samples)
    ///_|> symbol: pointer pointing to the symbol of the graph ("#" or ":")
    ///_|> returning: this function will not return anything

    for(int row = height; row >= 0; row--){
        printf("        |"); 
        for(int col = 0; col < width; col++){
            if(data[col] == row){
                printf("%s", symbol);
            }else{
                printf(" ");
            }
        }
        printf("\n");
    }
    if(strcmp(symbol, "#") == 0){
	    printf("   0 GB ");
    }else if(strcmp(symbol, ":") == 0){
	    printf("    0%%  ");
    }
    for(int col = 0; col < width + 1; col++){
        printf("-");
    }
    printf("\n\n");
}


int main(int argc, char **argv){
    //check if the process is the top(head) of the process in its group (only parent process handle signals)
    if(getpid() == getpgid(0)){
        signal(SIGINT, sigint_handle);
        signal(SIGTSTP, sigtstp_handle);
    }
   
    int memory = 0;
    int cpu = 0;
    int cores = 0;
    int samples = 20; //default value 
    int tdelay = 500000; //default value

    for (int i = 1; i < argc; i++) {
        if(strcmp(argv[i], "--memory") == 0){
            memory = 1;
        }else if(strcmp(argv[i], "--cpu") == 0){
            cpu = 1;
        }else if(strcmp(argv[i], "--cores") == 0){
            cores = 1;
        }else if (strncmp(argv[i], "--samples=", 10) == 0) {
            samples = atoi(argv[i] + 10);
        } else if (strncmp(argv[i], "--tdelay=", 9) == 0) {
            tdelay = atoi(argv[i] + 9);
        } else if (i == 1) { // Positional argument for samples
            samples = atoi(argv[i]);
        } else if (i == 2) { // Positional argument for tdelay
            tdelay = atoi(argv[i]);
        }
    }

    if (samples <= 0) {
        fprintf(stderr, "Error for the number of samples. Must be > 0.\n");
        return 1;
    }
    if (tdelay <= 0) {
        fprintf(stderr, "Error for the tdelay. Must be > 0.\n");
        return 1;
    }
    
    if(!memory && !cpu && !cores){ //if there is no arguments (./myMonitoringTool)
        memory = 1;
        cpu = 1;
        cores = 1;
    }
    
    //create pipe file descriptors for memory, cpu, and cores 
    int memoryFD[2];
    int cpuFD[2];
    int coresFD[2];
    if(pipe(memoryFD) == -1){
        perror("pipe failed");
        exit(1);
    }
    if(pipe(cpuFD) == -1){
        perror("pipe failed");
        exit(1);
    }
    if(pipe(coresFD) == -1){
        perror("pipe failed");
        exit(1);
    }

    pid_t pid_memory;
    pid_t pid_cpu;
    pid_t pid_cores; 

    //if memory == 1 when there is no flag or having --memory flag
    if(memory){
        pid_memory = fork(); //create a process
        if(pid_memory < 0){ 
            perror("fork failed");
            exit(1);
        } else if(pid_memory == 0){ //child process
            signal(SIGINT, SIG_IGN); //ignore SIGINT in child process
            signal(SIGTSTP, SIG_IGN); //ignore SIGTSTP in child process
            close(memoryFD[0]);
            Memory_graph(samples, tdelay, memoryFD[1]);
            exit(0);
        }
    }
    
    //if cpu == 1 when there is no flag or having --cpu flag
    if(cpu){
        pid_cpu = fork(); //create a process
        if(pid_cpu < 0){
            perror("fork failed");
            exit(1);
         }else if(pid_cpu == 0){ //child process
            signal(SIGINT, SIG_IGN); //ignore SIGINT in child process
            signal(SIGTSTP, SIG_IGN); //ignore SIGTSTP in child process
            close(cpuFD[0]);
            cpuUtilization_graph(samples, tdelay, cpuFD[1]);
            exit(0);
        }
    }

    //if cores == 1 when there is no flag or having --cores flag
    if(cores){
        pid_cores = fork(); //create a process
        if(pid_cores < 0){
            perror("fork failed");
            exit(1);
         }else if(pid_cores == 0){ //child process
            signal(SIGINT, SIG_IGN); //ignore SIGINT in child process
            signal(SIGTSTP, SIG_IGN); //ignore SIGTSTP in child process
            close(coresFD[0]);
            dup2(coresFD[1], STDOUT_FILENO);

            int freqFD[2]; 
            pipe(freqFD);

            pid_t pid_freq = fork(); //create a grand-child process
            if(pid_freq == 0){ //if is grand-child process
                close(freqFD[0]);
                max_Freq(freqFD[1]);
                close(freqFD[1]);
                exit(0);
            }
            //parent process of the grand-child
            close(freqFD[1]);
            countCores(freqFD[0]);
            close(freqFD[0]);
            waitpid(pid_freq, NULL, 0);
            exit(0);
        }
    }  

    //close the FDs for writing 
    close(memoryFD[1]);
    close(cpuFD[1]);
    close(coresFD[1]);

    //if calling the cores command line argument (ex, ./myMonitoringTool --cores)
    if(cores && !memory && !cpu){
        close(coresFD[1]);
        char corebuff[1024];
        ssize_t readFD = read(coresFD[0], corebuff, sizeof(corebuff) - 1);
        if(readFD > 0){
            corebuff[readFD] = '\0';
            printf("%s\n", corebuff);
        }
        waitpid(pid_cores, NULL, 0); 
        return 0;
    }

    //store data for memory and cpu graphs
    int memory_data[samples];
    int cpu_data[samples];

    //initialize the arrays
    for(int i = 0; i < samples; i++){
        memory_data[i] = -1;
        cpu_data[i] = -1;
    }

    int position = 0;//initialize to start at the first column
    
    for(int i = 0; i < samples && !quit; i++){ 
        //stop for the user to response
        while(stop){
            usleep(100000);
        }
        int memory_num = -1;
        int cpu_num = -1;
        double used_memory = 0.0;
        double total_memory = 0.0;
        double CPU_usage = 0.0;
        
        //read from the memory pipe of memoryFD[0]
        if(memory){ 
            read(memoryFD[0], &memory_num, sizeof(int));
            read(memoryFD[0], &used_memory, sizeof(double));
            read(memoryFD[0], &total_memory, sizeof(double));
            memory_data[position] = memory_num; 
        }

        //read from the cpu pipe of cpuFD[0]
        if(cpu){
            read(cpuFD[0], &cpu_num, sizeof(int));
            read(cpuFD[0], &CPU_usage, sizeof(double));
            cpu_data[position] = cpu_num;
        }
 
        printf("\033[H\033[J");
        printf("Nbr of samples: %d -- every %d microSecs (%0.2f secs)\n\n", samples, tdelay, (tdelay / 1000000.0));

        //print out the label for memory grpah
        if(memory){
            printf("V Memory  %.2f GB\n", used_memory);
            printf("  %.f GB |\n", total_memory); 
            display_graph(memory_data, 10, samples, "#");
        }

        //print out the label for cpu graph
        if(cpu){
            printf("V CPU ");
            printf("  %.2f %%\n", CPU_usage);
            printf("  100%%  |\n");
            display_graph(cpu_data, 10, samples, ":");
        }

        position = (position + 1) % samples;
    }


    if(cores && !quit){
        char corebuff[1024];
        ssize_t readFD = read(coresFD[0], corebuff, sizeof(corebuff) - 1);
        if(readFD > 0){
            corebuff[readFD] = '\0';
            printf("%s\n", corebuff);
        }
        waitpid(pid_cores, NULL, 0);
    }
    
    //if the user press Ctrl-C and answer Y, then kill all the child processes and quit the program
    if(quit){
        if(memory){
            kill(pid_memory, SIGTERM);
        }
        if(cpu){
            kill(pid_cpu, SIGTERM);
        }
        if(cores){
            kill(pid_cores, SIGTERM);
        }
    }

    //wait for all the child processes to finsih 
    if(memory){
        waitpid(pid_memory, NULL, 0);
    }
    if(cpu){
        waitpid(pid_cpu, NULL, 0);
    }
    return 0;
}