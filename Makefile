CXXFLAGS=-std=c++11 -O3
LIBFLAGS=-pthread
CPPDIR=Tools/Tools/cpp
SOURCEDIR=Source
INC=-I $(SOURCEDIR)

all: clean run

run:
	$(CXX) -o run $(CXXFLAGS) $(INC) run.cpp ${SOURCEDIR}/linear_classifiers.cpp ${SOURCEDIR}/zestxml.cpp ${SOURCEDIR}/helper.cpp $(LIBFLAGS)

clean:
	rm -f run