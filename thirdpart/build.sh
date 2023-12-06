#/bin/bash
thirdpart_path=$(pwd)
cd librdkafka-2.2.0
./configure --prefix=$thirdpart_path
make -j20
make install