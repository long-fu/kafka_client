## 编译librdkafka

```sh
cd librdkafka
./configure
make
sudo make install
```


## 编译kafka_client

当中可能会出现链接库错误 用下面的解决方案

1. Error while loading shared libraries: librdkafka.so.1: cannot open shared object file: No such file or directory

在编译lib库后

Either that or add /usr/local/lib to /etc/ld.so.conf (and run ldconfig afterwards to update)

```sh
vim /etc/ld.so.conf 
# add  /usr/local/lib
/usr/local/lib
# 保存
sudo ldconfig
```

```sh
cd kafka_client
mkdir build
cd build
cmake ..
cmake --build .
cd ../hack
sudo bash install.sh
```