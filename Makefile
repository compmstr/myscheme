.PHONY: clean
CC=gcc
CFLAGS=-ggdb
LIBS=

myscheme: myscheme.c types.h reader.o types.o eval.o writer.o
	$(CC) $(CFLAGS) $(LIBS) -Wall -ansi -o myscheme reader.o types.o writer.o eval.o myscheme.c

reader.o: reader.h reader.c
	$(CC) $(CFLAGS) -c reader.c

writer.o: writer.h writer.c
	$(CC) $(CFLAGS) -c writer.c

eval.o: eval.h eval.c
	$(CC) $(CFLAGS) -c eval.c

types.o: types.c types.h
	$(CC) $(CFLAGS) -c types.c

clean:
	rm *.o
	rm myscheme
