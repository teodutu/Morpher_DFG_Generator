# Morpher DFG Generator :  LLVM-based Control Data-Flow Graph (CDFG) generator

CGRAs target loop kernels where the application spends significat fraction of the execution time. Morpher requires the user to annotate the specifica loop kernels to be mapped. Morpher DFG Generator extract the loop in an .xml file. Morpher supports control-divergence (i.e., existence of multiple control flow paths) inside the loop kernel through partial predication.

## Build dependencies

This version requires LLVM 10.0.0 and JSON libraries. 

### LLVM, clang, polly

Read https://llvm.org/docs/GettingStarted.html
follow https://github.com/llvm/llvm-project

    git clone https://github.com/llvm/llvm-project.git
    git checkout <correct version> (llvm10.0.0)
    cd llvm-project
    mkdir build
    cd build
    cmake -DLLVM_ENABLE_PROJECTS='polly;clang' -G "Unix Makefiles" ../llvm
    make -j4
    sudo make install

Important points:

    make sure to checkout correct version before building
    better to use gold linker instead of ld if you face memory problem while building: https://stackoverflow.com/questions/25197570/llvm-clang-compile-error-with-memory-exhausted
    don't use release type use default debug version (will take about 70GB disk space)

### JSON

https://blog.csdn.net/jiaken2660/article/details/105155257


    git clone https://github.com/nlohmann/json.git
    mkdir build
    cd build
    cmake ../
    make -j2
    sudo make install

## Build DFG Generator

    cd Morpher_DFG_Generator
    git checkout stable
    mkdir build
    cd build
    cmake ..
    make all

## Examples

   Please refer the parent repository (https://github.com/ecolab-nus/morpher) 

## Running

Use `run_pass.sh <path_to_file> <mapped_funcion_name>` to obtain the DFG and memtraces file from an application:

```console
$ ./run_pass.sh morpher_benchmarks/array_add/array_add.c array_add
```

This will create:

- the DFG file `morpher_benchmarks/array_add/output/array_add_DODADFG.txt`

- the memtraces file with the initial and final memory layouts in `morpher_benchmarks/array_add/memtraces/array_add_trace_0.txt`
