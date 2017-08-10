# How to build

mkdir build
cd build
../configure --target-list=x86_64-softmmu --enable-debug --enable-gtk
make -j 4
