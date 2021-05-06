include ../Makefile.inc

CC 		= g++
CFLAGS 	= -Iinclude -I../includes
LIBNAME = ../libs/libimg3_compression.a
OBJECTS = IMG3_ZipInterface.o IMG3_LzssInterface.o 
 
vpath %.h include

$(LIBNAME): $(OBJECTS)
	$(ECHO) $(AR) $(ARFLAGS) $(LIBNAME) $(OBJECTS)
	$(AR) $(ARFLAGS) $(LIBNAME) $(OBJECTS)
	$(ECHO) zip library generation completed,

IMG3_ZipInterface.o: IMG3_ZipInterface.cpp
	$(ECHO) $(CC) $(CFLAGS) -c $^
	$(CC) $(CFLAGS) -c $^

IMG3_LzssInterface.o: IMG3_LzssInterface.cpp
	$(ECHO) $(CC) $(CFLAGS) -c $^
	$(CC) $(CFLAGS) -c $^

clean :
	$(ECHO) cleaning compression directory
	-$(RM) -f ./*.o include/*.gch

