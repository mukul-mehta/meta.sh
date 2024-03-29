SRCS=shell.cc tokenizer.cc utils.cc builtins.cc
EXECUTABLES=shell

# Define the compilers to be used to build the project
CXX = g++
CC = gcc

# Compilation flags for C/C++
CXXFLAGS = -g -Wall -Werror -std=c++11
CFLAGS=-g -Wall -std=gnu99

# Libraries to include during compilation. We use the GNU Readline library
LDFLAGS = -lreadline

OBJS=$(SRCS:.cc=.o)

all: $(EXECUTABLES)

$(EXECUTABLES): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) $(LDFLAGS) -o $@

.cc.o:
	$(CXX) $(CXXFLAGS) -c $< $(LDFLAGS) -o $@

clean:
	rm -rf $(EXECUTABLES) $(OBJS)

format:
	clang-format -i -style=file *.h *.cc
