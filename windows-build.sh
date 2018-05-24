export PATH=$1:$2:$PATH

cd ./

find . -name '*.o' -delete

make libpd install prefix='../'$3 LLVM=$4


read -p "Press enter to continue"  