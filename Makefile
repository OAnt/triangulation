EXTRA_BIN:=
EXTRA_CFLAGS="-DNDEBUG"
INCLUDES=-I include -I dependencies/include
CFLAGS=-Wall -g -fPIC -x c $(EXTRA_CFLAGS) 
SRCDIR=src
EXT_SRCDIR=src/ext
OBJDIR=build/objects
EXT_OBJDIR=build/objects/ext
SODIR=build/lib
SOOUT=$(SODIR)/libxxx.so
SOURCES=$(wildcard $(SRCDIR)/*.c)
OBJECTS=$(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SOURCES))
EXT_SOURCES=$(wildcard $(EXT_SRCDIR)/*.c)
EXT_OBJECTS=$(patsubst $(EXT_SRCDIR)/%.c, $(EXT_OBJDIR)/%.o, $(EXT_SOURCES))
TEST_LD_FLAGS:=-L$(SODIR) -lxxx $(shell pkg-config --libs check) -lm
TEST_CFLAGS:=$(CFLAGS) $(shell pkg-config --cflags check)
TESTOUT=build/test
TESTSRC=tests
TESTS=$(wildcard $(TESTSRC)/*.c)
CC=cc

all: build $(SOOUT)

build:
	mkdir -p $(OBJDIR) $(EXT_OBJDIR) $(SODIR)

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(EXT_OBJECTS): $(EXT_OBJDIR)/%.o : $(EXT_SRCDIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJDIR)/spatial_index.o: $(SRCDIR)/spatial_index.c
	$(CC) $(CFLAGS) -Wno-unused-function $(INCLUDES) -c $< -o $@

$(SOOUT): $(OBJECTS) $(EXT_OBJECTS)
	$(CC) -shared $(OBJECTS) $(EXT_OBJECTS) -o $@ $(EXTRA_CFLAGS)

$(TESTOUT): $(TESTSRC)
	$(CC) $(TESTS) $(TEST_CFLAGS) $(TEST_LD_FLAGS) $(INCLUDES) -o $@ 

tests_: build $(SOOUT) $(TESTOUT)
	LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$(SODIR) $(EXTRA_BIN) ./$(TESTOUT)

clean:
	rm -rf build
