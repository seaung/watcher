CC = g++
TOPDIR = $(shell pwd)

OBJDIR = $(TOPDIR)/obj/
SRCDIR = $(TOPDIR)/src/

ifdef DEBUG
	CFLAGS = -std=c++0x -Wall -g -O0 #-I../../include -I$(INCDIR)
else
	CFLAGS = -std=c++0x -Wall #-I../../include -I$(INCDIR)
endif

SRCLIST = $(wildcard $(SRCDIR)*.cpp)
OBJLIST = $(basename $(SRCLIST))
OBJTEMP1 = $(addsuffix .o ,$(OBJLIST))
OBJTEMP2 = $(notdir $(OBJTEMP1))
OBJ = $(addprefix $(OBJDIR),$(OBJTEMP2))
BINDIR = $(TOPDIR)/bin/
BIN = $(BINDIR)watcher

all:CHECKDIR APP_NAME

CHECKDIR:
	mkdir -p $(OBJDIR) $(BINDIR)

APP_NAME:$(BIN)

$(BIN):$(OBJ)
	$(CC) $^ -o $@ $(CFLAGS)

$(OBJDIR)%.o:$(SRCDIR)%.cpp
	$(CC) $(CFLAGS) $^ -o $@

.PHONY:clean

clean:
	rm -rf $(OBJDIR) $(BINDIR)

