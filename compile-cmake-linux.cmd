#export PATH=/usr/local/clang_9.0.0/bin:$PATH
#export LD_LIBRARY_PATH=/usr/local/clang_9.0.0/lib:$LD_LIBRARY_PATH

cd ./pfm/linux/proj/
make -j 8 VERBOSE=1
cd /home/indigoabstract/Desktop/appplex

