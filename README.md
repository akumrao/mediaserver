# Very high performing mediaserver. It is the only mediaserver which support both webrtc and ortc.

Live media streaming

SFU Multiparti Conference

webrtc streaming  ( states of Sender and Receiver are saved at server)

Ortc Streaming    ( states of producer and consumer are saved at client)

Probator, bandwidth estimator and Sound Energy level observer 


# Other Components of Mediaserver

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

compilers

apt-get install gcc-7

apt-get install g++-7

apt install clang++-10

apt install clang-10

cd /usr/bin

ln -s gcc-7 gcc

ln -s g++-7  g++

ln -s clang-1o  clang

ln -s clang++-10   clang++

gcc --version

clang --version


if you have multipe compilers you can do following 

sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-7 60 \
                         --slave /usr/bin/g++ g++ /usr/bin/g++-7 
                         
sudo update-alternatives --config gcc

gcc --version

g++ --version


sudo apt update

sudo apt -y install curl dirmngr apt-transport-https lsb-release ca-certificates

curl -sL https://deb.nodesource.com/setup_12.x | sudo -E bash -

apt-get install nodejs



To compile 

cd /workspace/mediaserver/src/sfu

make 

cd /workspace/mediaserver/src/sfu/sfuserver
make
./sfu

for broadcasting mp3 songs or mp4 movie

cd /workspace/mediaserver/src/broadcast/main

make

./runStreamer

before compilation, get webrtc and ffmpeg as mentioned at mediaserver/src/broadcast/Readme.md
