EXTRA_BIN:=
EXTRA_CFLAGS="-DNDEBUG"
INCLUDES=-I include
CFLAGS=-Wall -g -fPIC -x c $(EXTRA_CFLAGS)
SRCDIR=src
OBJDIR=build/objects
SODIR=build/lib
SOOUT=$(SODIR)/libxxx.so
SOURCES=$(wildcard $(SRCDIR)/*.c)
OBJECTS=$(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SOURCES))
TEST_LD_FLAGS:=-L$(SODIR) -lxxx $(shell pkg-config --libs check)
TEST_CFLAGS:=$(CFLAGS) $(shell pkg-config --cflags check)
TESTOUT=build/test
TESTSRC=tests
TESTS=$(wildcard $(TESTSRC)/*.c)

all: build $(SOOUT)

build:
	mkdir -p $(OBJDIR) $(SODIR)

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	cc $(CFLAGS) $(INCLUDES) -c $< -o $@

$(SOOUT): $(OBJECTS)
	cc -shared $(OBJECTS) -o $@ $(EXTRA_CFLAGS)

$(TESTOUT): $(TESTSRC)
	cc $(TESTS) $(TEST_CFLAGS) $(TEST_LD_FLAGS) $(INCLUDES) -o $@ 

tests_: build $(SOOUT) $(TESTOUT)
	LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$(SODIR) $(EXTRA_BIN) ./$(TESTOUT)

clean:
	rm -rf build
