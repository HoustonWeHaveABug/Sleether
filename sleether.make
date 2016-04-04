SLEETHER_C_FLAGS=-c -O2 -Wall -Wextra -Waggregate-return -Wcast-align -Wcast-qual -Wconversion -Wformat=2 -Winline -Wlong-long -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Wno-import -Wpointer-arith -Wredundant-decls -Wreturn-type -Wshadow -Wstrict-prototypes -Wswitch -Wwrite-strings

sleether.exe: sleether.o
	gcc -o sleether.exe sleether.o

sleether.o: sleether.c sleether.make
	gcc ${SLEETHER_C_FLAGS} -o sleether.o sleether.c
