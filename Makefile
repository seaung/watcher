CC = g++
TOPDIR = $(shell pwd)

OBJDIR = $(TOPDIR)/obj/
SRCDIR = $(TOPDIR)/src/
INCDIR = $(TOPDIR)/include/

ifdef DEBUG
	CFLAGS = -c -std=c++0x -Wall -g -O0 -I../../include -I$(INCDIR)
else
	CFLAGS = -c -std=c++0x -Wall -I../../include -I$(INCDIR)
endif

SRCLIST = $(wildcard $(SRCDIR)*.cpp)
OBJLIST = $(basename $(SRCLIST))
OBJTEMP1 = $(addsuffix .o ,$(OBJLIST))
OBJTEMP2 = $(notdir $(OBJTEMP1))
OBJ = $(addprefix $(OBJDIR),$(OBJTEMP2))
BINDIR = $(TOPDIR)/bin/
BIN = $(BINDIR)watcher

all:CHECKDIR APP_NAME distr

CHECKDIR:
	mkdir -p $(OBJDIR) $(BINDIR)

APP_NAME:$(BIN)

$(BIN):$(OBJ)
	$(CC) $^ -o $@ $(CFLAGS)

$(OBJDIR)%.o:$(SRCDIR)%.cpp
	$(CC) $(CFLAGS) $^ -o $@

distr:
	cp $(BIN) ../../bin

.PHONY:clean

clean:
	rm -rf $(OBJDIR) $(BINDIR)


.PHONY:clean
clean:
	rm -rf  *
