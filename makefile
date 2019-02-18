


# Example makefile for using the mat libraries
BIN=nn

# what you want to name your tar/zip file:
TARNAME=goes1944
CXX=g++

##CXXFLAGS=-O3 -Wall   # optimize
CXXFLAGS= -std=c++11 -ggdb -Wall # debug
##LIBS = -lm

EXAMPLES=

EXTRAS=\


SRCS=\
$(BIN).cpp\
dataset.cpp\
utility.cpp

HDRS=\
dataset.h\
utility.h

OBJS=\
$(BIN).o\
dataset.o\
utility.o

$(BIN): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) $(LIBS) -o $(BIN)

clean:
	/bin/rm -f *.o $(TARNAME).tar *~ core gmon.out a.out

tar:
	tar -cvf $(TARNAME).tar makefile $(EXAMPLES) $(SRCS) $(HDRS) $(EXTRAS) Eigen
	ls -l $(TARNAME).tar

zip:
	zip $(TARNAME).zip makefile $(EXAMPLES) $(SRCS) $(HDRS) $(EXTRAS)
	ls -l $(TARNAME).zip
