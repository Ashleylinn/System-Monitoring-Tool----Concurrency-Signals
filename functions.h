#ifndef FUNCTIONS_H
#define FUNCTIONS_H

void Memory_graph(int samples, int tdelay, int pipeFD);
void cpuUtilization_graph(int samples, int tdelay, int pipeFD);
void countCores(int freqFD);
void max_Freq(int freqFD);

#endif