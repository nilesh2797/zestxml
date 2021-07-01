import sys
import os
from setuptools import setup, Extension

EXP_DIR = os.getenv( "EXP_DIR" )

setup(
    # Information
    name = "p_mat",
    version = "1.0.0",
    # Build instructions
    ext_modules = [Extension("p_mat", ["p_mat.pyx", os.path.join( EXP_DIR, "Code", "Tools", "cpp", "mat.cpp" ) ], include_dirs=[ os.path.join( EXP_DIR, "Code", "Tools", "cpp" ) ], language="c++", extra_compile_args=['-std=c++11','-O3']), Extension("p_metrics", ["p_metrics.pyx", os.path.join( EXP_DIR, "Code", "Tools", "cpp", "mat.cpp" ), os.path.join( EXP_DIR, "Code", "Tools", "cpp", "metrics.cpp" ), os.path.join( EXP_DIR, "Code", "Tools", "python", "misc", "p_mat.pyx" ) ], include_dirs=[ os.path.join( EXP_DIR, "Code", "Tools", "cpp" ) ], language="c++", extra_compile_args=['-std=c++11','-O3'])]
)
