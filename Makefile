CXXFLAGS=-std=c++11 -O3
LIBFLAGS=-pthread -fopenmp
CPPDIR=Tools/Tools/cpp
SOURCEDIR=Source
INC=-I Tools/Tools/cpp -I $(SOURCEDIR)

all: clean run

run:
	$(CXX) -o run $(CXXFLAGS) $(INC) run.cpp ${CPPDIR}/linear_classifiers.cpp ${CPPDIR}/algos.cpp ${SOURCEDIR}/zestxml.cpp ${SOURCEDIR}/helper.cpp $(LIBFLAGS)

clean:
	rm -f run