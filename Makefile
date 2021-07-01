CXXFLAGS=-std=c++11 -O3
LIBFLAGS=-pthread -fopenmp
CPPDIR=Tools/Tools/cpp
SOURCEDIR=Source
INC=-I Tools/Tools/cpp -I $(SOURCEDIR)

all: clean run

run:
	$(CXX) -o run $(CXXFLAGS) $(INC) run.cpp ${CPPDIR}/linear_classifiers.cpp ${CPPDIR}/algos.cpp ${SOURCEDIR}/zestxml.cpp $(LIBFLAGS)


sparse_prod: clean_sparse_prod gen_sparse_prod
	

clean:
	rm -f run

clean_sparse_prod:
	rm -f sparse_prod

gen_sparse_prod:
	$(CXX) -o sparse_prod $(CXXFLAGS) $(INC) sparse_prod.cpp $(LIBFLAGS)