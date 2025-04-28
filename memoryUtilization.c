#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sysinfo.h>

void Memory_graph(int samples, int tdelay, int pipeFD){
    ///_|> descry: this function will write to the pipe of total memory and memory usage 
    ///_|> samples: given samples 
    ///_|> tdelay: given tdelay in microseconds
    ///_|> pipeFD: given pipe to write the memory data to the parent process
    ///_|> returning: this function will not return anything

    struct sysinfo info; //get information from sysinfo struct
    int Height = 10;

    for(int i = 0; i < samples; i++){
        //error occurs if sysinfo() returns -1
        if(sysinfo(&info) == -1){ 
            perror("sysinfo");
            int error = -1;
            write(pipeFD, &error, sizeof(int)); 
            continue;
        }

        //otherwise sysinfo() returns 0 and get the memory info
        double total_memory = info.totalram / (1024.0 * 1024.0 * 1024.0);
        double free_memory = info.freeram / (1024.0 * 1024.0 * 1024.0);
        double used_memory = total_memory - free_memory; 

        //calculate the dynamic position for memory
        int memory_y_position = (int)((used_memory / total_memory) * Height);

        //write to the pipe of memoryFD[1] and read from the parent process
        write(pipeFD, &memory_y_position, sizeof(int)); 
        write(pipeFD, &used_memory, sizeof(double));
        write(pipeFD, &total_memory, sizeof(double));

        usleep(tdelay);
    }
}
