mkdir -p memtraces
rm *_looptrace.log
rm *_loopinfo.log
rm *_munittrace.log
rm memtraces/*
clang -D CGRA_COMPILER  -target i386-unknown-linux-gnu -c -emit-llvm -O2 -fno-vectorize -fno-slp-vectorize -fno-tree-vectorize -fno-inline -fno-unroll-loops eeg_accuracy_flatloops_inttype_cgra_binary_gen.c -S -o eeg.ll
opt -gvn -mem2reg -memdep -memcpyopt -lcssa -loop-simplify -licm -disable-slp-vectorization -loop-deletion -indvars -simplifycfg -mergereturn -indvars eeg.ll -S -o eeg_opt.ll

opt -load ../../../build/src/libdfggenPass.so -fn $1 -nobanks $2 -banksize $3 -skeleton eeg_opt.ll -S -o eeg_opt_instrument.ll

clang -target i386-unknown-linux-gnu -c -emit-llvm -S ../../../src/instrumentation/instrumentation.cpp -o instrumentation.ll

llvm-link eeg_opt_instrument.ll instrumentation.ll -o final.ll

llc -filetype=obj final.ll -o final.o
clang++ -m32 final.o -o final
./final 1> final_log.txt 2> final_err_log.txt
