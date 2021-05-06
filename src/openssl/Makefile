include ../Makefile.inc

CC 		= g++
CFLAGS 	= -Iinclude -I../includes
LIBNAME = ../libs/libimg3_openssl.a
OBJECTS = IMG3_OpensslInterface.o

vpath %.h include

$(LIBNAME): $(OBJECTS)
	$(ECHO) $(AR) $(ARFLAGS) $(LIBNAME) $(OBJECTS)
	$(AR) $(ARFLAGS) $(LIBNAME) $(OBJECTS)
	$(ECHO) openssl library generation completed.

IMG3_OpensslInterface.o: IMG3_OpensslInterface.cpp
	$(ECHO) $(CC) $(CFLAGS) -c $^
	$(CC) $(CFLAGS) -c $^

clean :
	$(ECHO) cleaning openssl directory...
	-$(RM) -f ./*.o include/*.gch

