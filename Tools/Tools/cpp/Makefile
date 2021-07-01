INC=-I${CPP_TOOLS_DIR}
CXXFLAGS=-std=c++11 -O3
LIBFLAGS=-g
#PARABELCPPINC=${CPP_TOOLS_DIR}/mat.cpp

all: visualize_mat

visualize_mat:
	rm -f visualize_mat
	$(CXX) -o visualize_mat $(CXXFLAGS) $(INC) visualize_mat.cpp $(LIBFLAGS)
