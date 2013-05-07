.PHONY: clean
CC=gcc
#Don't really need 64 bit for this thing
#CFLAGS=-m32 -ggdb
CFLAGS=-ggdb
#CFLAGS=-m32
LIBS=-lgc

myscheme: myscheme.c types.h reader.o types.o eval.o writer.o except.o
	$(CC) $(CFLAGS) $(LIBS) -Wall -ansi -o myscheme reader.o types.o except.o writer.o eval.o myscheme.c

reader.o: reader.h reader.c
	$(CC) $(CFLAGS) -c reader.c

writer.o: writer.h writer.c
	$(CC) $(CFLAGS) -c writer.c

eval.o: eval.h eval.c
	$(CC) $(CFLAGS) -c eval.c

types.o: types.c types.h
	$(CC) $(CFLAGS) -c types.c

except.o: except.c except.h
	$(CC) $(CFLAGS) -c except.c

clean:
	rm *.o
	rm myscheme
