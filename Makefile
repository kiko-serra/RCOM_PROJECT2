# compiler
CCC = gcc

# C++ compiler flags (-g -O3 -Wall)
CCFLAGS = -g -O3 -Wall

# compile flags
LDFLAGS = -g

# source files
SRC = $(wildcard src/*.c)

OBJ = $(patsubst src%, tempfiles%.o, $(SRC))

OUT = bin/download

.SUFFIXES: .c

default: $(OUT)

tempfiles/%.o: src/%
	mkdir -p tempfiles
	$(CCC) $(INCLUDES) $(CCFLAGS) -c $< -o $@

$(OUT): $(OBJ)
	mkdir -p bin
	$(CCC) $(INCLUDES) $(CCFLAGS) $(OBJ) $(LIBS) -o $(OUT)

clean:
	rm -f $(OBJ) $(OUT)


all:
	make