"""
import sys
import os
from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize

IRMatch_EXP_DIR = os.getenv( "IRMatch_EXP_DIR" )

extensions = [
    Extension("p_mat", ["p_mat.pyx", os.path.join( IRMatch_EXP_DIR, "Tools", "cpp", "mat.cpp" )],
        include_dirs=[ os.path.join( IRMatch_EXP_DIR, "Tools", "cpp" ) ],
        language="c++", 
        extra_compile_args=['-std=c++11','-O3'])
]
setup(
    name="p_mat",
    ext_modules=cythonize(extensions),
)
"""


import sys
import os
from setuptools import setup, Extension

EXP_DIR = os.getenv( "EXP_DIR" )

setup(
    # Information
    name = "p_mat",
    version = "1.0.0",
    # Build instructions
    ext_modules = [Extension("p_mat", ["p_mat.pyx", os.path.join( EXP_DIR, "Code", "Tools", "cpp", "mat.cpp" ) ], include_dirs=[ os.path.join( EXP_DIR, "Code", "Tools", "cpp" ) ], language="c++", extra_compile_args=['-std=c++11','-O3'])]
)
