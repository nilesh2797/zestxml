INC=-I..
CXXFLAGS=-std=c++11 -O3 -g
LIBFLAGS=-pthread -fopenmp

all: clean print_diff_visual_inspection

print_diff_visual_inspection:
	$(CXX) -o print_diff_visual_inspection $(CXXFLAGS) $(INC) print_diff_visual_inspection.cpp visual_inspection.cpp ../data.cpp $(LIBFLAGS)

clean:
	rm -f print_diff_visual_inspection
