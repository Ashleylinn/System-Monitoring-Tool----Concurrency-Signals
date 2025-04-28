#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/sysinfo.h>

void countCores(int freqFD){
    ///_|> descry: this function will count the number of cores
    ///_|> freqFD: given the pipe of frequency file descriptor to get the maxFreq 
    ///_|> returning: this function will not return anything

    FILE *fp = fopen("/proc/cpuinfo", "r"); //using /proc/cpuinfo 
    if(fp == NULL){
        perror("Cannot open /proc/cpuinfo");
        return;
    }

    int count = 0;
    char buffer[256];

    while(fgets(buffer, sizeof(buffer), fp)){
        if(strncmp(buffer, "processor", 9) == 0){
            count++;
        }
    }
    fclose(fp);

    long maxFreq = 0;
    read(freqFD, &maxFreq, sizeof(long)); //read from the pipe to get the maximum frequency in order to print at the same line
    printf("V Number of Cores: %d @ %.2f GHz\n", count, maxFreq * 0.000001); 

    //display the graph
    int cols = 4;
    int rows = (count +  1) / 4;
    for(int i = 0; i < rows; i++){
        for(int j = 0; j < cols; j++){
            if(i * cols + j < count){
                printf(" +---+ ");
            }
        }
        printf("\n");
        for(int j = 0; j < cols; j++){
            if(i * cols + j < count){
                printf(" |   | ");
            }
        }
        printf("\n");
        for(int j = 0; j < cols; j++){
            if(i * cols + j < count){
                printf(" +---+ ");
            }
        }
        printf("\n");
    }
    printf("\n\n");
}

void max_Freq(int freqFD){ 
    ///_|> descry: this function will get the maximum frequency 
    ///_|> freqFD: given pipe to write the max frequency
    ///_|> returning: this function will not return anything

    FILE *fp = fopen("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq", "r"); //get max frequency 
    long maxFreq = 0;
    if(fp != NULL){
        if(fscanf(fp, "%ld", &maxFreq) == 1){
            write(freqFD, &maxFreq, sizeof(long)); //wrtie max freq to the pipe

        }
        fclose(fp);
    }else{
        perror("Cannot open cpuinfo_max_freq");
    }
}

