#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int readCPUtimes(long *total, long *idle_time){
    ///_|> descry: this function will read the cpu from /proc/stat 
    ///_|> total: pointer pointing to the total time (user+nice+sys+idle+IOwait+irq+softirq)
    ///_|> idle_time: pointer that points to the idle
    ///_|> returning: this function will return 0 if read successful, or -1 if fail

    FILE *fp = fopen("/proc/stat", "r"); //using /proc/stat to get cpu info
    if(fp == NULL){
        perror("Cannot open /proc/stat");
        return -1;
    }

    char buffer[256];
    if(fgets(buffer, sizeof(buffer), fp) == NULL){
        perror("Error getting /proc/stat");
        fclose(fp);
        return -1;
    }
    fclose(fp);

    long user, nice, sys, idle, IOwait, irq, softirq;
    sscanf(buffer, "cpu %ld %ld %ld %ld %ld %ld %ld", &user, &nice, &sys, &idle, &IOwait, &irq, &softirq);

    *idle_time = idle;
    *total = user + nice + sys + idle + IOwait + irq + softirq;

    return 0;
}

void cpuUtilization_graph(int samples, int tdelay, int pipeFD){
    ///_|> descry: this function will calculate cpu usage and write it to the pipe
    ///_|> samples: given samples 
    ///_|> tdelay: given tdelay in microseconds
    ///_|> pipeFD: a pipe file descriptor to write to the parent process
    ///_|> returning: this function will not return anything

    int Height = 10;

    for(int i = 0; i < samples; i++){
        long t1;
        long t2;
        long idle1;
        long idle2;

        if(readCPUtimes(&t1, &idle1) == -1){
            continue;
        }

        usleep(tdelay); //wait between the samples recommended by professor

        if(readCPUtimes(&t2, &idle2) == -1){
            continue;
        }

        long changeTotal = t2 - t1;
        long changeIdle = idle2 - idle1;
        if(changeTotal == 0){
            continue; 
        }
        
        //calculate cpu usage in percentage
        double CPU_usage = (double)(changeTotal - changeIdle) / changeTotal * 100.0;

        //calculate the dynamic position for cpu 
        int cpu_y_position = (int)((CPU_usage / 100.0) * Height);

        //write to the pipe of cpuFD[1] and read the information from the parent prcoess 
        write(pipeFD, &cpu_y_position, sizeof(int));
        write(pipeFD, &CPU_usage, sizeof(double));
    }   
}