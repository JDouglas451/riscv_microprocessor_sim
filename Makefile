# Directories
WORKDIR = build/
SRCDIR = src/

CFLAGS = -DNDEBUG

librsk.so: riscv64.o
	gcc $(CFLAGS) -fPIC -c -o $(WORKDIR)rskapi.o $(SRCDIR)rsk.c
	gcc $(CDEBUG) -shared -o $(WORKDIR)librsk.so $(WORKDIR)rskapi.o $(WORKDIR)riscv64.o

riscv64.o:
	gcc $(CFLAGS) -fPIC -c -o $(WORKDIR)riscv64.o $(SRCDIR)riscv64.c

debug: CFLAGS = -g -Wall -Werror
debug: librsk.so

run:
	@python src/rsh.py build/librsk.so

clean:
	@rm -r $(WORKDIR)*
