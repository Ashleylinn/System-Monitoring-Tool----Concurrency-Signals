CC = gcc -D_GNU_SOURCE

CFLAGS = -Wall -Werror -std=c99

OBJ = main.o CoresANDFreq.o CPUutilization.o memoryUtilization.o

.PHONY: all
all: myMonitoringTool

myMonitoringTool: ${OBJ}
	${CC} ${CFLAGS} -o $@ $^

%.o: %.c
	${CC} ${CFLAGS} -c $< -o $@

.PHONY: clean
clean: 
	rm -f ${OBJ} myMonitoringTool

	 