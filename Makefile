# Directories
WORKDIR = build/
SRCDIR = src/

# Compiler and linker settings variables
CDEBUG = -g -Wall -Werror -Wpedantic
CFINAL = -DNDEBUG

debug:
	gcc $(CDEBUG) -fPIC -c -o $(WORKDIR)rskapi.o $(SRCDIR)rsk.c
	gcc $(CDEBUG) -shared -o $(WORKDIR)libask.so $(WORKDIR)rskapi.o

final:
	gcc $(CFINAL) -fPIC -c -o $(WORKDIR)rskapi.o $(SRCDIR)rsk.c
	gcc $(CFINAL) -shared -o $(WORKDIR)libask.so $(WORKDIR)rskapi.o
	rm -r $(WORKDIR)*.o

clean:
	rm -r $(WORKDIR)*
