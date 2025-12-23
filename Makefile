# Makefile for ncurses_games
# Simple, readable build rules with helpful targets

# Compiler and flags
CXX      := g++
CXXFLAGS := -Wall -Wextra -Iinclude -std=c++11
# Enable debug flags when calling `make DEBUG=1`
ifeq ($(DEBUG),1)
CXXFLAGS += -g -O0
else
CXXFLAGS += -O2
endif

# Linker flags
LDFLAGS  := -lncurses

# Directories
SRCDIR   := src
BINDIR   := bin
OBJDIR   := obj

# Sources, objects and final target
SOURCES  := $(wildcard $(SRCDIR)/*.cpp)
OBJECTS  := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))
TARGET   := $(BINDIR)/ncurses_games

.PHONY: all help run clean distclean

# Default: build the program
all: $(TARGET)

# Link
$(TARGET): $(OBJECTS) | $(BINDIR)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

# Compile into object directory
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Ensure build directories exist
$(BINDIR) $(OBJDIR):
	mkdir -p $@

# Run the program (build first if necessary)
run: $(TARGET)
	./$(TARGET)

# Help message to explain targets and variables
help:
	@echo "Usage: make [target] [DEBUG=1]"
	@echo ""
	@echo "Common targets:"
	@echo "  all       Build the program (default)"
	@echo "  run       Build and run the program"
	@echo "  clean     Remove generated files (objects and binary)"
	@echo "  distclean Remove build directories as well (bin, obj)"
	@echo "  help      Show this message"
	@echo ""
	@echo "Use DEBUG=1 to enable debug flags (e.g. make DEBUG=1)"

# Remove generated files
clean:
	rm -f $(OBJDIR)/*.o
	rm -f $(TARGET)

# Remove build directories as well
distclean: clean
	rm -rf $(BINDIR) $(OBJDIR)
