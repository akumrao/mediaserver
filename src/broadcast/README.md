rpmbuild -ba --build-in-place --define "_topdir $(pwd)/rpm" runWebrtc.spec --define '_bindir /workspace/acvs-VideoEdge-webrtc/NVR/xStore/rtc/src/webrtc/main'  --define '_sysconfigdir /workspace/acvs-VideoEdge-webrtc/NVR/xStore/rtc/src/webrtc/signalserver/codelab'


mv rpm/SRPMS/*.rpm .
mv rpm/RPMS/*/*.rpm .
rm -rf rpm


rpm -qlp ./runWebrtc.rpm

spec copied from 

https://gist.github.com/shunkica/d6064c1a743029ea75328114b6dc036d








ffmpeg complile

apt-get install libssl-dev -y


apt install libsdl2-dev libsdl2-2.0-0 -y

apt install libfdk-aac-dev -y
apt install -y  nasm
cd /workspace

git clone git@github.com:FFmpeg/FFmpeg.git ffmpeg

cd ffmpeg

git checkout release/4.2

 ./configure --pkg-config-flags="--static" --libdir=/usr/local/lib --disable-shared --enable-static --enable-gpl --enable-pthreads --enable-nonfree  --disable-libfdk-aac    --enable-libx264 --enable-filters --enable-runtime-cpudetect --disable-lzma --disable-vaapi  --disable-libxcb --disable-libmfx



ffmpeg -encoders
ffmpeg -decoders

./configure --disable-yasm --enable-shared  --enable-ffplay--enable-debug=3  --disable-optimizations --disable-mmx --disable-stripping

do it 
 ./configure --enable-yasm --enable-shared  --enable-ffplay --enable-debug=3  --disable-optimizations --disable-mmx --disable-stripping --enable-gpl --enable-nonfree --enable-libfdk-aac


for h264 to enable for decoding - See  openh264 installing section next

./configure --disable-yasm --enable-shared   --enable-debug=2 --disable-optimizations --disable-mmx --disable-stripping --enable-gpl --enable-nonfree --enable-libfdk-aac    --enable-nonfree --enable-libx264 --enable-libopenh264

ffmpeg webrtc - for google chrome based compilation 
gn gen out/m84 --args='is_debug=true symbol_level=2 is_component_build=false is_clang=false rtc_include_tests=false rtc_use_h264=true rtc_enable_protobuf=false use_rtti=true use_custom_libcxx=false treat_warnings_as_errors=false use_ozone=true proprietary_codecs=true ffmpeg_branding="Chrome"'

gn gen out/m75 --args='is_debug=true symbol_level=2 is_component_build=false is_clang=true rtc_include_tests=false rtc_use_h264=false rtc_enable_protobuf=false use_rtti=false use_custom_libcxx=false treat_warnings_as_errors=false use_ozone=true'


apt-get install -y libx264-dev

export PATH=/workspace/webrtc/depot_tools:$PATH
gn gen out/m75 --args='is_debug=false symbol_level=0 is_component_build=false is_clang=true rtc_include_tests=false rtc_use_h264=false rtc_enable_protobuf=false use_rtti=true use_custom_libcxx=false treat_warnings_as_errors=false use_ozone=true'

apt-get install -y  nasm

For compiling and using openh264  
   git clone    https://github.com/cisco/openh264 

   git checkout openh264v1.5
   
   cd openh264/
   
   make -j8
   
   make install

****************************************************************** to compile ffmpeg in static mode**************************************
// do not give prefix path

apt-get install -y  nasm

apt-get remove  libx264-dev

cd /workspace 

git clone    https://github.com/mirror/x264.git 

cd x264

./configure   --disable-opencl --enable-static

make -j8

make install




git clone --branch stable --depth 2 https://bitbucket.org/multicoreware/x265_git
cd ~/ffmpeg_sources/x265_git/build/linux
cmake -G "Unix Makefiles" -DENABLE_SHARED:bool=off ../../source
make -j8
make install


zypper install nasm
zypper install libSDL2-devel

for ffplay sdl2 should be installed

git clone https://github.com/mstorsjo/fdk-aac.git
git checkout tags/v0.1.6 

./configure  --enable-static

 &&
make


git clone https://bitbucket.org/multicoreware/x265_git.git
cd x265/build/linux
./make-Makefiles.bash
make


cd /export/views/video/ffmpeg


  ./configure --pkg-config-flags=--static --libdir=/usr/local/lib --disable-shared --enable-optimizations --enable-static --enable-gpl --enable-pthreads --enable-nonfree --enable-libx264 --enable-filters --enable-runtime-cpudetect --disable-lzma --enable-ffplay --disable-vaapi --disable-vda  --enable-ffplay --enable-swscale 



debug

  cd /export/views/video/ffmpeg

  ./configure --pkg-config-flags="--static" --libdir=/usr/local/lib --disable-shared --enable-debug=2 --disable-optimizations --enable-static --enable-gpl --enable-pthreads --enable-nonfree  --enable-libfdk-aac    --enable-libx264 --enable-filters --enable-runtime-cpudetect --disable-lzma --disable-yasm --enable-ffplay 

***********************************************************************************************************************
export PATH=/usr/local/cuda/bin/:$PATH

./configure --pkg-config-flags=--static libdir=/usr/local/lib --incdir=/usr/local/cuda/include --enable-cuda-nvcc  --enable-cuda-nvcc --enable-nvdec --enable-nvenc  --disable-libnpp --extra-cflags=-I/usr/local/cuda/include --extra-ldflags=-L/usr/local/cuda/lib64  --disable-shared --enable-debug=2 --disable-optimizations  --enable-static --enable-gpl --enable-pthreads --enable-nonfree --enable-filters --enable-runtime-cpudetect --disable-lzma --enable-ffplay --disable-vaapi --disable-vdpau   --enable-ffplay --enable-swscale  --enable-libx264 



 ./configure --pkg-config-flags="--static" --libdir=/usr/local/lib --incdir=/usr/local/cuda/include --enable-cuda-nvcc --disable-libnpp --extra-cflags=-I/usr/local/cuda/include --extra-ldflags=-L/usr/local/cuda/lib64  --disable-shared --enable-debug=2 --disable-optimizations --enable-static --enable-gpl --enable-pthreads --enable-nonfree  --enable-libfdk-aac --enable-libx264 --enable-filters --enable-runtime-cpudetect --disable-lzma --disable-stripping --disable-vaapi --enable-libx264 


wget https://downloads.sourceforge.net/lame/lame-3.100.tar.gz && \
tar xvf lame-3.100.tar.gz && cd lame-3.100 && \
PATH="$HOME/bin:$PATH" \
./configure \
       --enable-shared \

on Intel machine 
./configure --pkg-config-flags=--static libdir=/usr/local/lib  --disable-shared --enable-optimizations  --enable-static --enable-gpl --enable-pthreads --enable-nonfree --enable-filters --enable-runtime-cpudetect --disable-lzma --enable-ffplay --enable-vaapi --disable-vdpau   --enable-ffplay --enable-swscale --extra-cflags=-I/usr/local/include --extra-ldflags=-L/usr/local/lib64  --extra-ldflags=-L/usr/local/lib64  --enable-libx264 



./configure --pkg-config-flags=--static libdir=/usr/local/lib  --disable-shared --enable-debug=2 --disable-optimizations  --enable-static --enable-gpl --enable-pthreads --enable-nonfree --enable-filters --enable-runtime-cpudetect --disable-lzma --enable-ffplay --enable-vaapi --disable-vdpau   --enable-ffplay --enable-swscale --extra-cflags=-I/usr/local/include --extra-ldflags=-L/usr/local/lib64  --extra-ldflags=-L/usr/local/lib64  --enable-libx264 


ffmpeg -hwaccel vaapi -hwaccel_output_format vaapi -hwaccel_device /dev/dri/renderD128 -i /var/tmp/test.264 -c:v h264_vaapi /tmp/test.264

export LIBVA_DRIVER_NAME=i965
export LD_LIBRARY_PATH=/usr/local/lib/:/usr/local/lib64/
export LIBVA_DRIVERS_PATH=/opt/intel/mediasdk/lib64

./ffmpeg  -hwaccel vaapi -hwaccel_output_format vaapi -hwaccel_device /dev/dri/renderD128 -i /var/tmp/reverse.264 -c:v h264_vaapi /tmp/test.264

both nvidia and mediasdk

 ./configure --pkg-config-flags="--static" --libdir=/usr/local/lib --incdir=/usr/local/cuda/include --enable-cuda-nvcc --disable-libnpp --extra-cflags=-I/usr/local/cuda/include --extra-cflags=-I/opt/intel/mediasdk/include --extra-ldflags=-L/usr/local/cuda/lib64  --extra-ldflags=-L/opt/intel/mediasdk/plugins --extra-ldflags=-L/opt/intel/mediasdk/lib --disable-shared --enable-debug=2 --disable-optimizations --enable-static --enable-gpl --enable-pthreads --enable-nonfree  --enable-libfdk-aac --enable-libx264 --enable-filters --enable-runtime-cpudetect --disable-lzma --disable-stripping --enable-vaapi --enable-libmfx --enable-libdrm

  --enable-libdrm \



  
    ffmpeg -hwaccel qsv -c:v h264_qsv -i /var/tmp/clock.264 -vf scale_qsv=1024:720 -c:v h264_qsv -c:a copy /tmp/output.264


    root:tmp# ffmpeg -hwaccel qsv -c:v h264_qsv -i /var/tmp/test.mp4 -vf 'vpp_qsv=framerate=60,scale_qsv=w=1920:h=1080' -c:v h264_qsv output.mp4


     ffmpeg -hwaccel qsv -c:v h264_qsv -i /var/tmp/test.mp4 -c:v h264_qsv output.mp4







To debug webrtc


chrome://webrtc-internals

firefox
about:webrtc
about:config

enable logging of webrtc

chrome --enablewebrtc log


To compile webrtc
Download and Install the Chromium depot tools.

https://webrtc.github.io/webrtc-org/native-code/development/prerequisite-sw/

*Linux (Ubuntu/Debian)*
A script is provided for Ubuntu, which is unfortunately only available after your first gclient sync:

./build/install-build-deps.sh


https://webrtc.github.io/webrtc-org/native-code/development/

export PATH=/export/webrtc/depot_tools:$PATH

- mkdir webrtc-checkout
- cd webrtc-checkout
- fetch --nohooks webrtc
- gclient sync

 Note: Remove ffmpeg internal build from webrtc with rtc_use_h264=false

- $ cd src
- $ git checkout -b m84 refs/remotes/branch-heads/4147
- $ gclient sync
- In OSX 10.14.16 this works:
- $ gn gen out/m84 --args='is_debug=false is_component_build=false is_clang=true rtc_include_tests=false rtc_use_h264=false rtc_enable_protobuf=false use_rtti=true mac_deployment_target="10.11" use_custom_libcxx=false'
- In Linux Debian Stretch with GCC 6.3 this works:
- $ gn gen out/m75 --args='is_debug=true symbol_level=2 is_component_build=false is_clang=false rtc_include_tests=false rtc_use_h264=false rtc_enable_protobuf=false use_rtti=true use_custom_libcxx=false treat_warnings_as_errors=false use_ozone=true'



- Then build it:
- $ ninja -C out/m75


for release

gn gen out/m84_release --args='is_debug=false symbol_level=0 is_component_build=false is_clang=false rtc_include_tests=false rtc_use_h264=true rtc_enable_protobuf=false use_rtti=true use_custom_libcxx=false treat_warnings_as_errors=false use_ozone=true'

ninja -C out/m84_release


-Then build mediaserver/src/webrtc:



$ cmake . -Bbuild \
  -DLIBWEBRTC_INCLUDE_PATH:PATH=/home/foo/src/webrtc-checkout/src \
  -DLIBWEBRTC_BINARY_PATH:PATH=/home/foo/src/webrtc-checkout/src/out/m84/obj

$ make -C build/
**********************************************************************************************************************************************************
For windows
1. Install git:

if you haven’t installed a copy of git, open https://git-for-windows.github.io/, download and install, suggest to select “Use Git from Windows Command Prompt” during installation.

2. Fetch depot_tools:

follow https://sourcey.com/articles/building-and-installing-webrtc-on-windows use 7z to extract to c:\webrtc\depot_tools

set system path 

open git cmd not git basah

mkdir webrtc-checkout
cd webrtc-checkout
fetch --nohooks webrtc

do not forget to set for using visual studio for builing code
set DEPOT_TOOLS_WIN_TOOLCHAIN=0

cd src
git branch -r
git checkout branch-heads/m75

gn gen out/x64/Debug --args="is_debug=true use_rtti=true target_cpu=\"x64\""
ninja -C out/x64/Debug boringssl field_trial_default protobuf_full p2p

**********************************************************************************************************************************************************
For android

export PATH=/workspace/depot_tools:$PATH

mkdir webrtc_android/

cd webrtc_android

fetch --nohooks webrtc_android.

cd src

git checkout branch-heads/m76

Then type gclient sync

git remote add arvind git@github.com:akumrao/webrtc-android.git

git remote update

gclient sync -D


git checkout 

      1 * (HEAD detached at arvind/multiplex-video)
      
      2   master


cd src

 ./build/install-build-deps.sh
 
 ./build/install-build-deps-android.sh



/*Generate compilation script*/

src/

gn gen out/arm --args='target_os="android" target_cpu="arm"'

ninja -C out/arm/

cp  ./out/arm/clang_x64/protoc ./out

delete our rm -rf out/arm


open android studio

open project /workspace/android_arwebrtc/MixedRealityWebRTCUnityDemo

open terminal in android studio

./gradlew clean

./gradlew genWebrtcSrc

./gradlew build

replace binaries generated by unity 

run install app

give permission to camera and mic at android setttings

open debug configuration 

select dual mode debug


Pplay pcm
ffplay -autoexit -f s16le -ar 48000 -ac 2 /var/tmp/out.pcm


ffmpeg -f s16le -ar 48000 -ac 2 -i  /var/tmp/test.mp3 file.wav


for aac for opus rencoding use 48000
ffmpeg -i /var/tmp/test.mp3 -ar 48000 -ac 2 -f s16le out.pcm


for aac and mp3 re encoding use 44100
ffmpeg -i /var/tmp/test.mp3 -ar 44100 -ac 2 -f s16le out.pcm



-acodec pcm_s16be: Output pcm format, signed 16 encoding, endian is big end (small end is le);
-ar 16000: The sampling rate is 16000
-ac 1: the number of channels is 1


**************************************************************************************


ffmpeg -re -i test.mp4 -g 52 -c:a aac -b:a 64k -c:v libx264 -b:v 448k -f mp4 -movflags frag_keyframe+empty_moov output.mp4



ffmpeg -i test.264  -i test.aac -f mp4 -movflags empty_moov+omit_tfhd_offset+frag_keyframe+default_base_moof /tmp/output1.mp4

ffmpeg -i test.264  -i test.aac -f mp4 -movflags empty_moov+omit_tfhd_offset+separate_moof+frag_custom /tmp/output2.mp4


ffmpeg -i kunal720.264  -i kunal720_track2.aac -f mp4 -movflags empty_moov+omit_tfhd_offset+frag_keyframe+default_base_moof /tmp/output1.mp4



for coredump  disable appport for file  otherwise do the following



 I looked at /var/log/apport.log:

cat /var/log/apport.log 
and I saw the file name:

ERROR: apport (pid 3426) Mon Nov  8 14:34:07 2021: writing core dump to core._home_guest_a_out.1000.4 ... 
and then I search throughout all the system

sudo find . -name "core._home_guest_a_out.1000.4..."
I found the core dump in /var/lib/apport/coredump/


chrome log

export CHROME_LOG_FILE=/tmp/chrome_debug.log
--enable-logging --v=1



***************************************************************************************************************************

https://web.postman.co/workspace/My-Workspace~758e8645-de0b-4fea-851a-6c7bb9149548/request/create?requestId=8c69d39b-a282-4e59-8bde-9a807c75309c

select options

https://streaming.pro-vigil.info:8080

select Headers

enter
key   admin@passsword
exp   360
perm   w

it will generate token
45674E7A-7936-4946-8E69-4A6C78537A2B^w^0^0^0^1652121492^25aece8f5854b8b4f1951a206332f9bbb676d9e6


Select Get 
add
token 


then do post to append camera

do put to add fresh camera list
















 ./configure --pkg-config-flags="--static" --libdir=/usr/local/lib --incdir=/usr/local/cuda/include --enable-cuda-nvcc --enable-libnpp --extra-cflags=-I/usr/local/cuda/include --extra-ldflags=-L/usr/local/cuda/lib64  --disable-shared --enable-debug=2 --disable-optimizations --enable-static --enable-gpl --enable-pthreads --enable-nonfree  --enable-libfdk-aac --enable-libx264 --enable-filters --enable-runtime-cpudetect --disable-lzma --disable-stripping --disable-vaapi









cd ~/ffmpeg_sources
git clone https://github.com/FFmpeg/FFmpeg -b master
cd FFmpeg
PATH="$HOME/bin:$PATH" PKG_CONFIG_PATH="$HOME/ffmpeg_build/lib/pkgconfig:/opt/intel/mediasdk/lib/pkgconfig" 


./configure \
  --pkg-config-flags="--static" \
  --extra-cflags="-I/opt/intel/mediasdk/include" \
  --extra-ldflags="-L/opt/intel/mediasdk/lib" \
  --extra-ldflags="-L/opt/intel/mediasdk/plugins" \
  --enable-libmfx \
  --enable-vaapi \
  --disable-debug \
  --enable-libdrm \
  --enable-gpl \
  --cpu=native \
  --enable-libfdk-aac \
  --enable-libx264 \
  --enable-openssl \
  --enable-pic \
  --extra-libs="-lpthread -lm -lz -ldl" \
  --enable-nonfree 



PATH="$HOME/bin:$PATH" make -j$(nproc) 
make -j$(nproc) install 
make -j$(nproc) distclean 
hash -r












workspace/intel_qsv/ffmpeg_codecs

Readme.txt

Encoder and decoder for intel based GPU or Inyegrated GPUs 

https://trac.ffmpeg.org/wiki/HWAccelIntro
https://www.hardening-consulting.com/en/posts/20170625-using-vaapi-with-ffmpeg.html
https://wiki.libav.org/Hardware/vaapi
https://habr.com/en/company/intel/blog/575632/
- 

Checking driver details:-
  inxi -G
    Graphics:  Card: Intel HD Graphics 620
               Display Server: X.Org 1.20.8 driver: i915 Resolution: 1366x768@60.06hz, 1366x768@59.79hz
               OpenGL: renderer: Mesa DRI Intel HD Graphics 620 (KBL GT2) version: 4.6 Mesa 20.0.8

  glxinfo -B
  vainfo


Encoder/Decoder for i965 driver based Integrated GPUs (Machine: Intel(R) HD Graphics 620 (KBL GT2))
  A. Building Encoder

    apt install vainfo

    Minimum build tools
    sudo apt update 
    sudo apt install build-essential
    apt-get install -y  nasm
    apt install cmake
    sudo apt-get install libtool

    X264 compilation
    https://code.videolan.org/videolan/x264

    git clone http://git.videolan.org/git/x264.git -b stable
    ./configure  --enable-static --disable-opencl
    make
    make install


    X265_git
        git clone https://bitbucket.org/multicoreware/x265_git/ x265
    cd x265/build/linux
    cmake -G "Unix Makefiles"  -DENABLE_SHARED:bool=off ../../source
    make install

    apt install libfdk-aac-dev -y

    #to enable ffplay 
    sudo apt-get install libsdl2-dev

    git clone -b release/4.2  https://github.com/FFmpeg/FFmpeg.git 
    cd FFmpeg
    ./configure --pkg-config-flags="--static"  --extra-cflags="-I/usr/local/include"  --extra-ldflags="-L/usr/local/lib" --enable-debug=3 --enable-vaapi --enable-gpl  --cpu=native   --enable-opengl  --enable-libfdk-aac   --enable-libx264   --enable-libx265   --extra-libs=-lpthread --enable-nonfree
    make -j8
    make install
    ffmpeg -hwaccel vaapi -hwaccel_device /dev/dri/renderD128 -loglevel verbose -i /home/sanjay/Downloads/test.mp4 testOut.mp4


    zypper install libdrm-devel-2.4.104-1.12.x86_64

    zypper   libX11-devel   libXext-devel  libXfixes-devel  libxcb-devel 


    sudo apt-get install libdrm-dev





    git clone https://github.com/intel/libva
    cd libva

     git checkout v2.5-branch


    ./autogen.sh
    ./configure
    time make -j$(nproc) VERBOSE=1
    sudo make -j$(nproc) install
    sudo ldconfig -vvvv


    LibDRM:
    git clone https://gitlab.freedesktop.org/mesa/drm.git drm
    cd drm

    For ubuntu 18.04
    ./autogen.sh --prefix=/usr --libdir=/usr/lib/x86_64-linux-gnu

    pip3 install meson
    sudo apt install libpciaccess-dev

    meson builddir/
    ninja -C builddir/ install


    https://01.org/linuxmedia/quickstart/linux-vaapi-videostack-environment 

    git clone https://github.com/intel/libva-utils.git 

    git checkout v2.5-branch

    ./autogen.sh
    make
    make install

    ln -s /usr/lib/x86_64-linux-gnu/dri/i965_drv_video.so /usr/local/lib/dri/

    //for installing opengl
    sudo apt install freeglut3-dev

    vainfo

    https://trac.ffmpeg.org/wiki/Hardware/VAAPI

    ffmpeg -vaapi_device /dev/dri/renderD128 -i /home/sanjay/Downloads/test.mp4 -vf 'format=nv12,hwupload' -c:v h264_vaapi output.mp4



export LIBVA_DRIVER_NAME=i965 

export LD_LIBRARY_PATH=/usr/local/lib/:/usr/local/lib64/

   export LIBVA_DRIVER_NAME=iHD
   

   export LIBVA_DRIVERS_PATH=/opt/intel/mediasdk/lib64
 

    arvindnvr:/workspace/intel/libva-utils # /usr/local/bin/vainfo 



    git clone https://github.com/Intel-Media-SDK/MediaSDK msdk
    cd msdk
    mkdir build && cd build
    cmake ..
    make
    make install


    To find out where the library is - sudo find / -name the_name_of_the_file.so

    Add library path in standard lib search -
    LD_LIBRARY_PATH=/usr/local/lib
    echo $LD_LIBRARY_PATH
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/intel/mediasdk/lib/
    echo $LD_LIBRARY_PATH
    sudo ldconfig

    Ffmpeg doc/examples for encode_video test program
    Add -lva-drm in LD_LIBS for inking error related to drm

    Ffmpeg command to run vaapi
    ffmpeg -y -loglevel trace -vaapi_device /dev/dri/renderD128 -i /home/sanjay/Downloads/test.mp4 -vf 'format=nv12,hwupload' -c:v h264_vaapi file_out.mp4



    ffmpeg -i /home/sanjay/Downloads/test.mp4 -c:v rawvideo -pix_fmt nv12 in.yuv
    ffplay out.yuv
    Get dimensions and give it in next command

    ./vaapi_encoder <Width> <Height> in.yuv output.h264
    ffplay output.h264




**********************************************************************************************************************************************************************************************************

For Nvidia

ffmpeg complile 

https://trac.ffmpeg.org/ticket/7782

https://docs.nvidia.com/video-technologies/video-codec-sdk/ffmpeg-with-nvidia-gpu/


http://trac.ffmpeg.org/wiki/Hardware/QuickSync


git clone https://git.videolan.org/git/ffmpeg/nv-codec-headers.git

git checkout sdk/11.1 //version should match with driver and nvidia-smi


Install ffnvcodec

cd nv-codec-headers && sudo make install && cd –
Clone FFmpeg's public GIT repository.
git clone https://git.ffmpeg.org/ffmpeg.git ffmpeg/
Install necessary packages.
sudo apt-get install build-essential yasm cmake libtool libc6 libc6-dev unzip wget libnuma1 libnuma-dev
Configure


./configure --enable-nonfree --enable-cuda-nvcc --enable-libnpp --extra-cflags=-I/usr/local/cuda/include --extra-ldflags=-L/usr/local/cuda/lib64 --disable-static --enable-shared
Compile

make -j 8
Install the libraries.
sudo make install


to install cuda

first check the version of cuda with nvidia-smi

go to archived older versions

wget https://developer.download.nvidia.com/compute/cuda/11.7.0/local_installers/cuda_11.7.0_515.43.04_linux.run
sudo sh cuda_11.7.0_515.43.04_linux.run


add /usr/local/cuda/lib64 at  /etc/ld.so.conf

ldconfig to reload the config


cd /export/views/video/ffmpeg

    ./configure --pkg-config-flags="--static" --libdir=/usr/local/lib --disable-shared --enable-static --enable-gpl --enable-pthreads --enable-nonfree  --enable-libfdk-aac --enable-filters --enable-runtime-cpudetect --disable-lzma  --enable-pic --extra-cflags=-fPIC

debug

  cd /export/views/video/ffmpeg

  ./configure --pkg-config-flags="--static" --libdir=/usr/local/lib --incdir=/usr/local/cuda/include --enable-cuda-nvcc --enable-libnpp --extra-cflags=-I/usr/local/cuda/include --extra-ldflags=-L/usr/local/cuda/lib64  --disable-shared --enable-debug=2 --disable-optimizations --enable-static --enable-gpl --enable-pthreads --enable-nonfree  --enable-libx264 --enable-filters --enable-runtime-cpudetect --disable-lzma --disable-stripping --disable-vaapi


./configure --pkg-config-flags="--static" --libdir=/usr/local/lib --incdir=/usr/local/cuda/include --enable-cuda-nvcc --disable-libnpp --extra-cflags=-I/usr/local/cuda/include --extra-ldflags=-L/usr/local/cuda/lib64  --disable-shared --enable-debug=2 --disable-optimizations --enable-static --enable-gpl --enable-pthreads --enable-nonfree --enable-libx264 --enable-filters --enable-runtime-cpudetect --disable-lzma --disable-stripping --disable-vaapi












ERROR: failed checking for nvcc.

ompute_30 to compute_35 and sm_30 to sm_35 inside the ffmpeg/configure worked. – 
user388160
 Nov 20, 2020 at 20:03


we can get the value from compile cuda samples




git diff configure 
      1 diff --git a/configure b/configure
      2 index 6409b94b65..b85802f480 100755
      3 --- a/configure
      4 +++ b/configure
      5 @@ -4352,10 +4352,10 @@ fi
      6  
      7  if enabled cuda_nvcc; then
      8      nvcc_default="nvcc"
      9 -    nvccflags_default="-gencode arch=compute_30,code=sm_30 -O2"
     10 +    nvccflags_default="-gencode arch=compute_35,code=sm_35 -O2"
     11  else
     12      nvcc_default="clang"
     13 -    nvccflags_default="--cuda-gpu-arch=sm_30 -O2"
     14 +    nvccflags_default="--cuda-gpu-arch=sm_35 -O2"
     15      NVCC_C=""
     16  fi



./ffmpeg -hwaccel cuda -hwaccel_output_format cuda -i /var/tmp/test.mp4 -c:v h264_nvenc -preset slow /tmp/output/out.mp4

If ffmpeg was compiled with support for libnpp, it can be used to insert a GPU based scaler into the chain:

ffmpeg -hwaccel_device 0 -hwaccel cuda -i input -vf scale_npp=-1:720 -c:v h264_nvenc -preset slow output.mkv


if you get following error 

Error while decoding stream #0:0: Invalid data found when processing input
[h264 @ 0x55e0f4718580] No decoder surfaces left

add -extra_hw_frames 8

./ffmpeg -y -vsync 0 -hwaccel cuda -hwaccel_output_format cuda -extra_hw_frames 8 -i /var/tmp/test6.mp4 -c:a copy -c:v h264_nvenc -b:v 5M output.mp4






-profile:v – one of high, main, or baseline (and others, but this is irrelevant here)



-acodec pcm_s16be: Output pcm format, signed 16 encoding, endian is big end (small end is le);
-ar 16000: The sampling rate is 16000
-ac 1: the number of channels is 1



avcodec_find_decoder_by_name , do not find decoder with id  avcodec_find_decoder(AV_CODEC_ID_H264);


And there is no h264_videotoolbox decoder, only encoder. To list decoders/encoders available:

ffmpeg -encoders
ffmpeg -decoders




lscpu | grep "Model name"
Model name:          Intel(R) Core(TM) i7-10700F CPU @ 2.90GHz
                                 
https://www.intel.com/content/www/us/en/processors/processor-numbers.html

0F no integerated graphics
 

 lscpu | grep "Model name"
Model name:                      Intel(R) Xeon(R) W-2225 CPU @ 4.10GHz


Integrated graphics card  

or 

cat /proc/cpuinfo

Intel® UHD Graphics for 11th Gen Intel® Processors


Max Resolution (HDMI)‡
4096x2304@60Hz

Max Resolution (DP)‡
7680x4320@60Hz

Max Resolution (eDP - Integrated Flat Panel)‡
4096x2304@60Hz


Intel® Quick Sync Video
Yes


/del suse laptop
i5-6300U CPU @ 2.40GHz
6th generation graphics   // it soupport only vapi drivers http://trac.ffmpeg.org/wiki/Hardware/QuickSync


modinfo nvidia



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

vaapi information 



/workspace/intel_qsv/ffmpeg_codecs

Readme.txt

Encoder and decoder for intel based GPU or Inyegrated GPUs 

https://trac.ffmpeg.org/wiki/HWAccelIntro
https://www.hardening-consulting.com/en/posts/20170625-using-vaapi-with-ffmpeg.html
https://wiki.libav.org/Hardware/vaapi
https://habr.com/en/company/intel/blog/575632/
- 

Checking driver details:-
  inxi -G
    Graphics:  Card: Intel HD Graphics 620
               Display Server: X.Org 1.20.8 driver: i915 Resolution: 1366x768@60.06hz, 1366x768@59.79hz
               OpenGL: renderer: Mesa DRI Intel HD Graphics 620 (KBL GT2) version: 4.6 Mesa 20.0.8

  glxinfo -B
  vainfo


Encoder/Decoder for i965 driver based Integrated GPUs (Machine: Intel(R) HD Graphics 620 (KBL GT2))
  A. Building Encoder

    apt install vainfo

    Minimum build tools
    sudo apt update 
    sudo apt install build-essential
    apt-get install -y  nasm
    apt install cmake
    sudo apt-get install libtool

    X264 compilation
    https://code.videolan.org/videolan/x264

    git clone http://git.videolan.org/git/x264.git -b stable
    ./configure  --enable-static --disable-opencl
    make
    make install


    X265_git
        git clone https://bitbucket.org/multicoreware/x265_git/ x265
    cd x265/build/linux
    cmake -G "Unix Makefiles"  -DENABLE_SHARED:bool=off ../../source
    make install

    apt install libfdk-aac-dev -y

    #to enable ffplay 
    sudo apt-get install libsdl2-dev






    git clone -b release/4.2  https://github.com/FFmpeg/FFmpeg.git 
    cd FFmpeg
    ./configure --pkg-config-flags="--static"  --extra-cflags="-I/usr/local/include"  --extra-ldflags="-L/usr/local/lib" --enable-debug=3 --enable-vaapi --enable-gpl  --cpu=native   --enable-opengl  --enable-libfdk-aac   --enable-libx264   --enable-libx265   --extra-libs=-lpthread --enable-nonfree
    make -j8
    make install
    ffmpeg -hwaccel vaapi -hwaccel_device /dev/dri/renderD128 -loglevel verbose -i /home/sanjay/Downloads/test.mp4 testOut.mp4


    sudo apt-get install libdrm-dev
    git clone https://github.com/intel/libva
    cd libva


    ./autogen.sh
    ./configure
    time make -j$(nproc) VERBOSE=1
    sudo make -j$(nproc) install
    sudo ldconfig -vvvv


    LibDRM:
    git clone https://gitlab.freedesktop.org/mesa/drm.git drm
    cd drm

    For ubuntu 18.04
    ./autogen.sh --prefix=/usr --libdir=/usr/lib/x86_64-linux-gnu

    pip3 install meson
    sudo apt install libpciaccess-dev

    meson builddir/
    ninja -C builddir/ install


    https://01.org/linuxmedia/quickstart/linux-vaapi-videostack-environment 

    git clone libva-utils
    ./autogen.sh
    make
    make install

    ln -s /usr/lib/x86_64-linux-gnu/dri/i965_drv_video.so /usr/local/lib/dri/

    //for installing opengl
    sudo apt install freeglut3-dev

    vainfo

    https://trac.ffmpeg.org/wiki/Hardware/VAAPI

    ffmpeg -vaapi_device /dev/dri/renderD128 -i /home/sanjay/Downloads/test.mp4 -vf 'format=nv12,hwupload' -c:v h264_vaapi output.mp4


    git clone https://github.com/Intel-Media-SDK/MediaSDK msdk
    cd msdk
    mkdir build && cd build
    cmake ..
    make
    make install


    To find out where the library is - sudo find / -name the_name_of_the_file.so

    Add library path in standard lib search -
    LD_LIBRARY_PATH=/usr/local/lib
    echo $LD_LIBRARY_PATH
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/intel/mediasdk/lib/
    echo $LD_LIBRARY_PATH
    sudo ldconfig

    Ffmpeg doc/examples for encode_video test program
    Add -lva-drm in LD_LIBS for inking error related to drm

    Ffmpeg command to run vaapi
    ffmpeg -y -loglevel trace -vaapi_device /dev/dri/renderD128 -i /home/sanjay/Downloads/test.mp4 -vf 'format=nv12,hwupload' -c:v h264_vaapi file_out.mp4



    ffmpeg -i /home/sanjay/Downloads/test.mp4 -c:v rawvideo -pix_fmt nv12 in.yuv
    ffplay out.yuv
    Get dimensions and give it in next command

    ./vaapi_encoder <Width> <Height> in.yuv output.h264
    ffplay output.h264









//////////////////////// office nvr

./configure --pkg-config-flags="--static" --libdir=/workspace/x264/ --incdir=/workspace/x264  --incdir=/usr/local/cuda/include --enable-cuda-nvcc --disable-libnpp --extra-cflags=-I/workspace/x264  --extra-cflags=-I/usr/local/cuda/include --extra-ldflags=-L/usr/local/cuda/lib64 --extra-ldflags=-L/workspace/x264  --disable-shared --enable-debug=2 --disable-optimizations --enable-static --enable-gpl --enable-pthreads --enable-nonfree  --disable-libfdk-aac --enable-libx264 --enable-filters --enable-runtime-cpudetect --disable-lzma --disable-stripping --disable-vaapi --disable-vdpau



officeNVR:/workspace/ffmpeg # ./ffmpeg_g -hwaccel cuda -hwaccel_output_format cuda -extra_hw_frames 8 -i /var/tmp/test.264 -c:v h264_nvenc -preset slow /tmp/test.264





Environment="LIBVA_DRIVER_NAME=i965"
Environment="GST_PLUGIN_PATH=/opt/americandynamics/venvr/lib/gstreamer-1.0:/usr/lib64/gstreamer-1.0:/opt/americandynamics/venvr/device_handlers/intellex/gst-plugins:/opt/americandynamics/3rdParty/lib/gstreamer-1.0"


