# Directories
WORKDIR = build/
SRCDIR = src/

# Compiler and linker settings variables
CDEBUG = -g -Wall -Werror -Wpedantic
CFINAL = -DNDEBUG

final:
	gcc $(CFINAL) -fPIC -c -o $(WORKDIR)rskapi.o $(SRCDIR)rsk.c
	gcc $(CFINAL) -shared -o $(WORKDIR)librsk.so $(WORKDIR)rskapi.o
	@find $(WORKDIR) -type f ! -name '*.so' -delete

debug:
	gcc $(CDEBUG) -fPIC -c -o $(WORKDIR)rskapi.o $(SRCDIR)rsk.c
	gcc $(CDEBUG) -shared -o $(WORKDIR)librsk.so $(WORKDIR)rskapi.o

tests:
	@echo "No tests\n"

# Run rsh.py
run:
	@python src/rsh.py build/librsk.so

# Run test programs
runtests:
	@echo "No tests\n"

clean:
	@rm -r $(WORKDIR)*
