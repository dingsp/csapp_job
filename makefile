CC = clang

all:myshell

myshell:myshell.c signal.c sio.c job.c command.c
	${CC} -o $@ $^
