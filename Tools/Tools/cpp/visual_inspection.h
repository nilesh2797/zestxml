#pragma once

#include <iostream>
#include <vector>

#include "mat.h"
#include "utils.h"

using namespace std;

class VisualInspection
{
public:
    SMatF* mat;
    VecS rows;
    VecS cols;

    VisualInspection( SMatF* _mat, VecS& _rows, VecS& _cols )
    {
        mat = _mat;
        rows = _rows;
        cols = _cols;
    }

    VisualInspection( string _matfile, string _rowsfile, string _colsfile )
    {
        
    }

    void visualize_all_colwise( string outfile )
    {

    }

    void visualize_all_rowwise( string outfile )
    {
        
    }


};