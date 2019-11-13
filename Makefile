INCLUDES=-Iinclude
CFLAGS=-Wall -g -fPIC
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
	cc -shared $< -o $@

$(TESTOUT): $(TESTSRC)
	cc $(TEST_CFLAGS) $(TEST_LD_FLAGS) $(INCLUDES) -o $@ $(TESTS)

tests_: $(SOOUT) $(TESTOUT)
	./$(TESTOUT)

clean:
	rm $(OBJECTS)
	rmdir build
