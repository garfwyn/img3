include ../Makefile.inc

CC 		= g++
CFLAGS 	= -Iinclude -I../includes
LIBNAME = ../libs/libimg3_html.a
OBJECTS = IMG3_HtmlTag.o IMG3_HtmlParser.o

vpath %.h include

$(LIBNAME): $(OBJECTS)
	$(ECHO $(AR) $(ARFLAGS) $(LIBNAME) $(OBJECTS)
	$(AR) $(ARFLAGS) $(LIBNAME) $(OBJECTS)
	$(ECHO) html library generation completed.

IMG3_HtmlParser.o: IMG3_HtmlParser.cpp
	$(ECHO) $(CC) $(CFLAGS) -c $^
	$(CC) $(CFLAGS) -c $^

IMG3_HtmlTag.o: IMG3_HtmlTag.cpp
	$(ECHO) $(CC) $(CFLAGS) -c $^
	$(CC) $(CFLAGS) -c $^

clean :
	$(ECHO) cleaning html directory...
	-$(RM) -f ./*.o include/*.gch

