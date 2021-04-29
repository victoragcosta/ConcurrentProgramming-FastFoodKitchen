# ============================== CONFIGURATIONS ============================== #
# Program name
EXECUTABLE=./bin/fast-food-kitchen.out
# All the intermediate paths to the executable folder
EXECUTABLE_FOLDERS=./bin

# Definitions folder
IDIR=./include
# Sources folder
SDIR=./src
# Where to put the objs
ODIR=./obj

# Compiler/linker
CC=g++
# Extension of the definitions files
DEF_EXT=hpp
# Extension of the source files
SRC_EXT=cpp
# Compilation flags
CFLAGS=-Wall -g -pthread -I $(IDIR)
# Libs flags
LIBS=-lm

# All definitions sources
DEPS != find $(IDIR)/ -name *.$(DEF_EXT)

# All sources in src
_OBJ != find $(SDIR)/ -name *.$(SRC_EXT) | sed -e 's/.$(SRC_EXT)/.o/g'
OBJ = $(patsubst $(SDIR)/%,$(ODIR)/%,$(_OBJ))

# All subfolders in src
_FOLDERS != find $(SDIR)/ -name *.$(SRC_EXT) | cut -d "/" -f 3 | sed -e '/.$(SRC_EXT)/d' | uniq
FOLDERS = $(patsubst %,$(ODIR)/%,$(_FOLDERS))

# ================================= TARGETS ================================== #

# Creates executable (linking)
$(EXECUTABLE): $(OBJ) | $(EXECUTABLE_FOLDERS)
	$(CC) -o $@ $^ $(LIBS) $(CFLAGS)
# Creates objs for the executable
$(ODIR)/%.o: $(SDIR)/%.$(SRC_EXT) $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

# Asks for a obj folder if needed
# the '|' tells to ignore the folder timestamp
$(OBJ): | $(ODIR) $(FOLDERS)

# Creates obj folder wheen needed
$(ODIR) $(FOLDERS) $(EXECUTABLE_FOLDERS):
	mkdir -p $@

# Compiles all files
all: $(EXECUTABLE)

# Removes all compilation files
clean:
	rm -rf $(ODIR) $(EXECUTABLE)

# Runs the program
run: $(EXECUTABLE)
	./$<

.PHONY: run clean all

# ================================= THE END ================================== #
