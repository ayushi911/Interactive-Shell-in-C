CC = gcc

all:
	$(CC) Inshell.c -o out

clean:
	rm -rf *.o 
