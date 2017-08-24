# How to build

mkdir build
cd build
../configure --target-list=x86_64-softmmu --enable-debug --enable-gtk
make -j 4

# How to run

./qemu-system-x86_64 -kernel bzImage -initrd newinitrd.img -append "root=/dev/ram rdinit=/hello" -monitor stdio


# How to create newinitrd.img

Create initrd, last number must match kernel version. e.g.:

mkinitramfs -o initrd.img-4.9.41 

extract image, should create kernel dir
put application inside kernel dir (e.g. hello)

cd kernel
find . | cpio --create --format="newc" > ../newinitrd
cd ..
gzip newinitrd
mv newinitrd.gz newinitrd.img

