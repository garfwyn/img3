include ../Makefile.inc

CC 		= gcc
CPP 	= g++
CFLAGS 	= -g -Iinclude -I../includes
LIBNAME = ../libs/libimg3_sqlite3.a
OBJECTS = sqlite3.o IMG3_Sqlite3.o

vpath %.h include

$(LIBNAME): $(OBJECTS)
	$(ECHO) $(AR) $(ARFLAGS) $(LIBNAME) $(OBJECTS)
	$(AR) $(ARFLAGS) $(LIBNAME) $(OBJECTS)
	$(ECHO) sqlite3 library generation completed,

IMG3_Sqlite3.o: IMG3_Sqlite3.cpp
	$(ECHO) $(CPP) $(CFLAGS) -c $^
	$(CPP) $(CFLAGS) -c $^

sqlite3.o: sqlite3.c 
	$(ECHO) $(CC) $(CFLAGS) -c $^
	$(CC) $(CFLAGS) -c $^

clean :
	$(ECHO) cleaning sqlite3 directory
	-$(RM) -f ./*.o include/*.gch $(LIBNAME)

