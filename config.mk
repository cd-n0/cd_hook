CC = gcc
CXXC = g++
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic
CXXFLAGS = -std=c++11 -Wall -Wextra -Wpedantic
LDLIBS = 
DEBUG_FLAGS = -ggdb
RELEASE_FLAGS = -O2

# Directories
SRCDIR = src
OBJDIR = obj
TESTDIR = tests
TESTOBJDIR = $(TESTDIR)/obj
TESTBINDIR = $(TESTDIR)/bin

# Source and object files
SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))
TESTSRCS = $(wildcard $(TESTDIR)/*.cpp)
TESTOBJS = $(patsubst $(TESTDIR)/%.cpp,$(TESTOBJDIR)/%.o,$(TESTSRCS))
TESTBINS = $(patsubst $(TESTOBJDIR)/%.o,$(TESTBINDIR)/%,$(TESTOBJS))

BUILD ?= debug
ifeq ($(BUILD), debug)
	CFLAGS += $(DEBUG_FLAGS)
else ifeq ($(BUILD), release)
	CFLAGS += $(RELEASE_FLAGS)
endif

TARGET = libcdhook.a
