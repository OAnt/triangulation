EXTRA_BIN:=
EXTRA_CFLAGS="-DNDEBUG"
DEPS=dependencies
INCLUDES=-I include -I $(DEPS)/include
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

MLIB_GIT=.xxx_mlib
MLIB_TAG=V0.3.0

_DEPS=$(PWD)/$(DEPS)
$(MLIB_GIT):
	git clone https://github.com/P-p-H-d/mlib.git $(MLIB_GIT)

requirements: $(MLIB_GIT)
	mkdir -p $(_DEPS)
	cd $(MLIB_GIT); git checkout $(MLIB_TAG); make install PREFIX=$(_DEPS)
	rm -rf $(MLIB_GIT)

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
