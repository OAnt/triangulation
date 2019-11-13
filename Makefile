INCLUDES=-Iinclude
CFLAGS=-Wall -g -fPIC
SRCDIR=src
OBJDIR=build/objects
SODIR=build/lib
SOOUT=$(SODIR)/libxxx.so
SOURCES=$(wildcard $(SRCDIR)/*.c)
OBJECTS=$(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SOURCES))

all: build $(SOOUT)

build:
	mkdir -p $(OBJDIR) $(SODIR)

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	cc $(CFLAGS) $(INCLUDES) -c $< -o $@

$(SOOUT): $(OBJECTS)
	cc -shared $< -o $@

clean:
	rm $(OBJECTS)
	rmdir build
