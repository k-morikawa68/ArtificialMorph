CXX = g++
EIGEN_INCLUDE ?= /usr/include/eigen3
LIBIGL_INCLUDE ?= /usr/local/include
CXXFLAGS = -std=c++17 -I$(EIGEN_INCLUDE) -I$(LIBIGL_INCLUDE)
LIBS = -lpthread
OBJS = \
	   main.o\
	   HalfEdgeMesh.o\
	   Element.o\

PROGRAM = a

all: $(PROGRAM)

$(PROGRAM): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) $(LIBS) -o $(PROGRAM).out

clean: 
	rm -f *.o *.out
