include config.mk

all: $(TARGET)

# Clean target to remove compiled files
clean:
	rm -rf $(OBJDIR)
	rm -f $(TARGET)
	rm -rf $(TESTOBJDIR)
	rm -rf $(TESTBINDIR)

# Build the main target
$(TARGET): $(OBJS)
	ar rcs $@ $^

# Object file compilation
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MF $(@:.o=.d) -o $@ -c $<

# Include dependency files
-include $(OBJDIR)/*.d

$(TESTOBJDIR):
	@mkdir $@

$(TESTBINDIR):
	@mkdir $@

# Rule to compile test objects
$(TESTOBJDIR)/%.o: $(TESTDIR)/%.cpp | $(TESTOBJDIR)
	$(CXXC) $(CXXFLAGS) -o $@ -c $<

# Build test binaries from object files
$(TESTBINDIR)/%: $(TESTOBJDIR)/%.o $(TARGET) | $(TESTBINDIR)
	$(CXXC) $(CXXFLAGS) -o $@ $< $(TARGET) $(LDLIBS)

# Run all tests
test: $(TESTBINS) | $(OBJS)
	@echo "Running tests..."
	@for test in $^; do \
		echo "Running $$test..."; \
		./$$test && echo "TEST $$test OK" || echo "TEST $$test FAIL"; \
	done

.PHONY: all clean test
