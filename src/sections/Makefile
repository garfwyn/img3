include ../Makefile.inc

CC = g++
CFLAGS = -g -Iinclude -I../includes
LIBNAME = ../libs/libimg3_sections.a
OBJECTS = IMG3_FileInterface.o IMG3_FileSection.o

vpath %.h include

$(LIBNAME): $(OBJECTS)
	$(ECHO) $(AR) $(ARFLAGS) $(LIBNAME) $(OBJECTS)
	$(AR) $(ARFLAGS) $(LIBNAME) $(OBJECTS)
	$(ECHO) sections library generation completed.

IMG3_FileInterface.o: IMG3_FileInterface.cpp
	$(ECHO) $(CC) $(CFLAGS) -c $^
	$(CC) $(CFLAGS) -c $^

IMG3_FileSection.o: IMG3_FileSection.cpp
	$(ECHO) $(CC) $(CFLAGS) -c $^
	$(CC) $(CFLAGS) -c $^

clean :
	$(ECHO) cleaning sections directory...
	-$(RM) -f ./*.o include/*.gch

