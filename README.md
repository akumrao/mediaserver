# Very high performing mediaserver. It is the only mediaserver with support both webrtc and ortc.

Live media streaming

SFU Multiparti Conference

webrtc streaming  ( states of Sender and Receiver are saved at server)

Ortc Streaming    ( states of producer and consumer are saved at client)

Probator, bandwidth estimator and Sound Energy level observer 


# Other Components Mediaserver

OS abstraction with libuv and std c14. 

Http ans https sever

Websocket sever

Mjpeg streaming


******************************************************************************************************************************

libuv is a multi-platform support library with a focus on asynchronous I/O. It was primarily developed for use by Node.js, but it's also used by Luvit, Julia, pyuv, and others.


******************************************************************************************************************************
for building 
apt install build-essential

install clang 8 or above

Install the gcc-7 packages:

sudo apt-get install -y software-properties-common
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt update
sudo apt install g++-7 -y
Set it up so the symbolic links gcc, g++ point to the newer version:

sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-7 60 \
                         --slave /usr/bin/g++ g++ /usr/bin/g++-7 
sudo update-alternatives --config gcc
gcc --version
g++ --version
clang --version
clang++ --version

optional
apt-get install libc++abi-dev
apt-get install libc++-dev
apt-get install libsctp-dev
apt-get install libstdc++5


To compile 
cd /workspace/mediaserver/src/sfu
make 

cd /workspace/mediaserver/src/sfu/sfuserver
make
./sfu
