include Makefile.inc

CC 			= g++
DIRS		= compression html openssl sections sqlite3
EXE			= img3_parser
OBJS		= IMG3_Functions.o
OBJLIBS	    = libimg3_compression.a libimg3_html.a libimg3_openssl.a libimg3_sections.a libimg3_sqlite3.a
LIBSDIR     = libs/
LIBS		= -L$(LIBSDIR) -limg3_compression -limg3_html -limg3_openssl -limg3_sections -limg3_sqlite3 -lcrypto -lpthread
CFLAGS	    = -Icompression/include -Ihtml/include -Iopenssl/include -Isections/include -Isqlite3/include -Iincludes 

all : $(EXE)

img3_parser : IMG3_Main.cpp IMG3_Functions.o $(OBJLIBS)
	$(ECHO) $(CC) -o $@ IMG3_Main.cpp $(OBJS) $(LIBS) $(CFLAGS)
	$(CC) -o $@ IMG3_Main.cpp $(OBJS) $(LIBS) $(CFLAGS)

IMG3_Functions.o : $(OBJLIBS)
	$(ECHO) $(CC) -c IMG3_Functions.cpp $(CFLAGS)
	$(CC) -c IMG3_Functions.cpp $(CFLAGS)

libimg3_html.a : force_look
	$(ECHO) looking into subdir html : $(MAKE) $(MFLAGS)
	cd html; $(MAKE) $(MFLAGS)

libimg3_sqlite3.a : force_look
	$(ECHO) looking into subdir sqlite3 : $(MAKE) $(MFLAGS)
	cd sqlite3; $(MAKE) $(MFLAGS)

libimg3_compression.a: force_look
	$(ECHO) looking into subdir compression: $(MAKE) $(MFLAGS)
	cd compression; $(MAKE) $(MFLAGS)

libimg3_sections.a : force_look
	$(ECHO) looking into subdir sections : $(MAKE) $(MFLAGS)
	cd sections; $(MAKE) $(MFLAGS)

libimg3_openssl.a: force_look
	$(ECHO) looking into subdir openssl : $(MAKE) $(MFLAGS)
	cd openssl; $(MAKE) $(MFLAGS)

clean :
	$(ECHO) cleaning up
	-$(RM) -f $(EXE) $(OBJS) includes/*.gch
	-for d in $(OBJLIBS); do ($(RM) -f $(LIBSDIR)/$$d); done
	-for d in $(DIRS); do (cd $$d; $(MAKE) clean ); done

force_look :
	true

