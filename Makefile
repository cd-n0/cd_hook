CC=gcc
CXXC=g++
CFLAGS=-I$(INCDIR) -Wall -Wextra
LDLIBS=
DEBUG_FLAGS = -ggdb
RELEASE_FLAGS = -O2

# Default build is debug
BUILD ?= debug

ifeq ($(BUILD), debug)
    CFLAGS += $(DEBUG_FLAGS)
else ifeq ($(BUILD), release)
    CFLAGS += $(RELEASE_FLAGS)
endif

# Test source files
TESTSRCS = $(wildcard tests/*.cpp)

# Test executables
TESTS = $(TESTSRCS:.cpp=.out)

# Directories
SRCDIR = src
OBJDIR = obj
INCDIR = inc

SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))
INCLUDES = $(wildcard $(INCDIR)/*.h)

TARGET=cd_hook.a

################################################################################

all: $(TARGET) compile_flags.txt

# Clean target to remove compiled files
clean:
	rm -rf $(OBJDIR)
	rm -f $(TARGET)
	rm -f $(TESTS)
	rm -f compile_flags.txt

$(TARGET): $(OBJS)
	ar rcs $@ $^

$(OBJDIR)/%.o : $(SRCDIR)/%.c $(INCLUDES)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ -c $<

# Compile the test executables
$(TESTS): tests/%.out: tests/%.cpp $(TARGET)
	$(CXXC) $(CFLAGS) -o $@ $< $(TARGET) $(LDLIBS)

# Run the tests
test: $(TESTS) $(TESTSRCS)
	for test in $(TESTS); do \
		./$$test && \
		echo "TEST $$test OK" || \
		echo "TEST $$test FAIL"; \
	done

compile_flags.txt: Makefile
	echo "$(CFLAGS)" | tr ' ' '\n'> compile_flags.txt

.PHONY: all clean attach
