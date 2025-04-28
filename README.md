# System Monitoring Tool -- Concurrency & Signals

This program will present the information in real-time concurrently in graphical form  by displaying the memory, cpu, and cores. This program is intend to run un a Linux OS (IA lab machines).


## How I solve the problem

I solve this problem by first splitting my assignment 1 into smaller pieces to differnet C files which makes it modular. And also, since it is an extension of assignment 1, so I try to figure out what will be best way to display the graph using processes effectively. Then, I think it is better to display the graph from the main function which is the parent process and get all the data and calculation from the child processes. To make each task run concurrently, I create 4 processes using fork() for each task (memory utilization, cpu utilization, count cores, and get maximum frequency). I use pipe() to get the output from the child process and send it back to the parent process to display the data. For handling the signals, I use two global variables quit and stop to handle when the user is repsonsing so to quit the program safely. 

## Implementation  

<code><sys/sysinfo.h></code>
 * Gets memory usage using sysinfo()

<code>/proc/stat</code>
 * Gets cpu usage and calculates CPU utilization through the differences in the total time and the initial time between samples

<code>/proc/cpuinfo</code>
 * Gets the number of cores in the system

<code><signal.h></code>
 * Used for sig_atomic_t, SIG_IGN, SIGTERM, etc.

<code><sys/wait.h></code>
 * Used for waiting the child processes
 * waitpid(pid_t, int *, int)

<code>usleep()</code>
 * In the <unistd.h> library and takes the argument in microseconds
 * Allow precise control over the timing of each sample collection 
 * The usleep is used in displaying the memory graph and cpu graph which is showing the delay between samples
 * When compiling the program, it may encounter the warning since it has been marked as deprecated in POSX.1-2008. However, it will not affect the functionality as it provide precision sampling in system matrics.
 * I use -D_GNU_SOURCE in the CC to make it silence when using -std=c99

### Functions Documentation

<code>void Memory_graph(int samples, int tdelay, int pipeFD)</code>
 * Gets the memory usage and write it to the pipe

<code>int readCPUtimes(long *total, long *idle_time)</code>
 * Get the data from <code>/proc/stat</code> 
 * Get the total time and extract the idle for calculating the cpu usage

<code>void cpuUtilization_graph(int samples, int tdelay, int pipeFD)</code>
 * Calculate the CPU utilization by getting the data from readCPUtimes()
 * Taking two samples and gets each CPU times  
 * Write the CPU usage to the pipe 

<code>void countCores(int freqFD)</code>
 * Display the number of of cores and maximum frequency 
 * The data is read from <code>/proc/cpuinfo</code> for counting cores
 * Read the frequency from the pipe 

<code>void max_Freq(int freqFD)</code>
 * Read from <code>/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq</code>
 * Write the data to the pipe 

<code>void sigint_handle(int signum)</code>
  * Handle the signal for SIGINT (Ctrl-C)
  * The user will make a choice of Y or N for quitting the program

<code>void sigtstp_handle(int signum)</code>
  * Handle the singal for SIGTSTP (Ctrl-Z)
  * It will ignore when the program is running interactively 

<code>void display_graph(int *data, int height, int width, const char *symbol)</code>
 * Display the structure of the memory graph and cpu graph

<code>int main(int argc, char **argv)</code>
 * The main function handles the command line arguments
 * Create processes for each task
 * Communicate the output through pipe

## Pseudo-code
* Signal handling (SIGINT and SIGTSTP) only in the parent process 
* Parse the command line arguments:
  -  check if there is flags (--memory, --cpu, --cores, --samples, --tdelay)
  -  check if ssamples and tdelay are positional arguments
* If samples less than 0
  - Send error that must be larger than 0
* If tdelay less than 0
  - Send error that must be larger than 0
* If there is no command line arguments 
  - show the default behavior samples = 20 and tdelay = 500000
* Create pipe for each (memory, cpu, cores)
* if memory 
  - fork()
  - in the child process
     - ignore the signal
     - call to get the data of memory usage and write it to the pipe
* if cpu 
  - fork()
  - in the child process
     - ignore the signal
     - call to get the data of cpu usage and write it to the pipe
* if cores 
  - fork()
  - in the child process
     - fork() create a grand-child process for frequency
        - in the grand-child process
           - get the maximum freuquency and write to the pipe
     - call the function to count the number of cores and write to the pipe
     - wait for the child processes to finish
* in the parent proces
  - if only cores command line argument
     - read the data from the pipe
     - only display the cores graph
  - for each sample
     - if user is responsing
        - wait for the response
     - if memory
        - read data from pipe
        - display graph
     - if cpu
        - read data from pipe
        - display cpu graph
     - display memory graph
     - display cpu graph 
  - if quit
     - kill all the child processes
  - wait for all the child processes to finish


## How to run the program

1. Compile with Makefile

        make
  
2. Run
   
        ./myMonitoringTool or 
        ./myMonitoringTool with flags 
       (ex, ./myMonitoringTool --memory, ./myMonitoringTool 20 10000 --cpu

3. Clean 

        make clean  

  * will remove all the object files
 
<br> 

## CLAs

* <code>--memory</code>
  * Only generate the memory usage graph
* <code>--cpu</code>
  * Only generate the cpu usage graph
* <code>--cores</code>
  * Only generate the cores information and the maximum frequency 
* <code>--samples=N</code>
  * N samples will be collected and based on the N numbers of repetition 
  * Also taken as positional argument
  * The default value is 20
* <code>--tdelay=T</code>
  * Delay in samples between microseconds
  * Also taken as positional argument
  * The default value is 500000 microsec

<br> 

## Test cases
- <code>./myMonitoringTool</code>
  - it shows the default behavior which has a samples = 20 and tdelay = 500000 microseconds
  - display memory graph, cpu graph, and cores grpah
- <code>./myMonitoringTool --memory</code>
  -  it only display the memory graph with the default value of samples=20 and tdelay=500000
- <code>./myMonitoringTool --cpu</code>
   - it will only display the cpu graph with the default value of samples=20 and tdelay=500000
- <code>./myMonitoringTool --cores</code>
  - it will only shows the number of cores and the maximum frequency 
- <code>./myMonitoringTool 40 10000</code>
  - it will show all the graph with samples=40 and tdelay=10000
- <code>./showFDtables --samples=40 --tdelay=10000</code>
  - this will also show all the graph with the samples=40 and tdelay=10000 
- <code>./showFDtables 40 10000 --memory</code>
  - it will display the memory graph with the samples=40 and tdelay=10000
  - if user input samples and tdelay and include a flag, it will only display that graph with the provided samples and tdelay
- <code>./myMonitoringtool</code>
  - if there is a typo, it will shows no such file or directory
- <code>./myMonitoringtool samples=40 tdelay=10000</code>
  - this will show Error for the number of samples. Must be > 0.
  - should be --samples=N and --tdelay=T
- <code>./myMonitoringtool --memOry</code>
  - if there is typo in command line arguments, it will print out errors.
  

## Examples
<code>./myMonitoringTool</code>

![Image](https://drive.google.com/uc?export=view&id=18Wn6Z1jZaWc86rmdz0Vtw6K4LmZHdcC2)

<code>./myMonitoringTool --memory</code>

![Image](https://drive.google.com/uc?export=view&id=1gV_dnu4XK_xymqaXlcMkOkrCN3unW0ZJ)

<code>./myMonitoringTool --cpu</code>

![Image](https://drive.google.com/uc?export=view&id=1fW0KYdMkm20yRu636rh_ma68Etmrg_zP)

<code>./myMonitoringTool --cores</code>

![Image](https://drive.google.com/uc?export=view&id=1NJL3yCn6prFYgLKPzJ2z0-cMmHJ0zs9j)

<code>./myMonitoringTool 40 15000</code>

![Image](https://drive.google.com/uc?export=view&id=1cCvdCThsMqrCouBAjbeF-n5e6CvZY4Ji)

<code>./myMonitoringTool --samples=40 --tdelay=15000</code>

![Image](https://drive.google.com/uc?export=view&id=1VRX_5OXC3520K9aobqfHeK9HgvrN3nYu)





## References 
- https://man7.org/linux/man-pages/man2/sysinfo.2.html
- https://man7.org/linux/man-pages/man5/proc_cpuinfo.5.html
- https://stackoverflow.com/questions/24931456/how-does-sig-atomic-t-actually-work
- https://pubs.opengroup.org/onlinepubs/009695399/basedefs/signal.h.html
- https://pubs.opengroup.org/onlinepubs/009695299/basedefs/sys/wait.h.html
- https://docs.kernel.org/cpu-freq/cpufreq-stats.html
