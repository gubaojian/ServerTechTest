## init project




## init project source

## uWebSockets

cd uSockets

git submodule init

git submodule update


cd boringssl

git submodule init

git submodule update

cd boringssl

## cmake -B build

cmake -C build -DCMAKE_BUILD_TYPE=Release

make

## boost
下载boost_1_85_0 解压到third_party中。
cd boost_1_85_0
mkdir build
./bootstrap.sh --prefix=./build
./b2


## 暂时不需要
cd lsquic

git submodule init

git submodule update

cd  ls-qpack
git submodule init

git submodule update


cd liblsquic

cmake -DBORINGSSL_DIR=../boringssl/build

make


## 添加submodule

https://blog.csdn.net/Da_zhenzai/article/details/132828059


## 代理下载代码
export https_proxy=http://127.0.0.1:7890 http_proxy=http://127.0.0.1:7890 all_proxy=socks5://127.0.0.1:7890
