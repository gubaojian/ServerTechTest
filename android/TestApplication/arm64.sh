rm -rf /Users/efurture/Downloads/openssl-openssl-3.6.0/android
mkdir 
export ANDROID_NDK_ROOT=/Users/efurture/Library/Android/sdk/ndk/27.0.12077973
PATH=$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/darwin-x86_64/bin:$PATH
./Configure android-arm64 -D__ANDROID_API__=29 --prefix=/Users/efurture/Downloads/openssl-openssl-3.6.0/android
make -j12
