
# Exit with error 1 if the number of arguments is less than 2
if [ "$#" -lt 2 ]; then
    echo "Usage: $0 <input_file> <function_name> [num_banks] [bank_size]"
    exit 1
fi

INPUT_DIR=$(dirname "$1")
FULLNAME="${1%.*}"

clang -D CGRA_COMPILER -target i386-unknown-linux-gnu -c -emit-llvm \
    -O2 -fno-tree-vectorize -fno-inline -fno-unroll-loops "$FULLNAME.c" -S -o "$FULLNAME.ll"
opt -gvn -mem2reg -memdep -memcpyopt -lcssa -loop-simplify -licm \
    -disable-slp-vectorization -loop-deletion -indvars -simplifycfg -mergereturn -indvars "$FULLNAME.ll" \
    -o "${FULLNAME}_gvn.ll"
#opt -load ~/manycore/cgra_dfg/buildeclipse/skeleton/libSkeletonPass.so -fn $BASENAME -ln $2 -ii $3 -skeleton integer_fft_gvn.ll -S -o integer_fft_gvn_instrument.ll

opt -load ./build/src/libdfggenPass.so -fn "$2" -skeleton "${FULLNAME}_gvn.ll" \
    -S -o "${FULLNAME}_gvn_instrument.ll" -nobanks "${3:-2}"  -banksize "${4:-2048}"

clang -target i386-unknown-linux-gnu -c -emit-llvm -S ./src/instrumentation/instrumentation.cpp -o instrumentation.ll

llvm-link "${FULLNAME}_gvn_instrument.ll" instrumentation.ll -o final.ll

mkdir -p memtraces

llc -filetype=obj final.ll -o final.o
clang++ -m32 final.o -o final
./final 1> final_log.txt 2> final_err_log.txt

rm -rf "$INPUT_DIR/output" "$INPUT_DIR/memtraces"
mkdir -p "$INPUT_DIR/output"

mv "$2"* instrumentation.ll time* cfg* final* "$INPUT_DIR"/*.ll "$INPUT_DIR/output"
mv memtraces "$INPUT_DIR"
