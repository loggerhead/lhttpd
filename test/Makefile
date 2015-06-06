CCFLAGS = -g
LDFLAGS = -L/usr/local/lib -luv -ljson-c -lhttp_parser -L.. -llhttpd

SOURCE = ${FILENAME}.c
EXECUTABLE = ${FILENAME}


all:
	$(CC) $(CCFLAGS) $(LDFLAGS) $(SOURCE) -o $(EXECUTABLE)

clean:
	rm -f *.o