CC=gcc
CFLAGS=-std=c89 -I$(INCDIR) -Wall -Wextra -Wpedantic
LDLIBS=
DEBUG_FLAGS = -ggdb3
RELEASE_FLAGS = -O2

# Default build is debug
BUILD ?= debug

ifeq ($(BUILD), debug)
    CFLAGS += $(DEBUG_FLAGS)
else ifeq ($(BUILD), release)
    CFLAGS += $(RELEASE_FLAGS)
endif

# Test source files
TESTSRCS = $(wildcard tests/*.c)

# Test executables
TESTS = $(TESTSRCS:.c=)

# Directories
SRCDIR = src
INCDIR = inc
OBJDIR = obj

SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))

TARGET=target.out

################################################################################

all: $(TARGET) compile_flags.txt

# Clean target to remove compiled files
clean:
	rm -rf $(OBJDIR)
	rm -f $(TARGET)
	rm -f $(TESTS)
	rm -f compile_flags.txt

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

$(OBJDIR)/%.o : $(SRCDIR)/%.c 
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ -c $<

# Compile the test executables
$(TESTS): $(TESTSRCS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $@.c $(LDLIBS)

# Run the tests
test: $(TESTS) $(TESTSRCS)
	for test in $(TESTS); do \
		./$$test && \
		echo "TEST $$test OK" || \
		echo "TEST $$test FAIL"; \
	done

compile_flags.txt: Makefile
	echo "$(CFLAGS)" | tr ' ' '\n'> compile_flags.txt

.PHONY: all clean
