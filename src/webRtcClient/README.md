ffmpeg complile 
apt-get install libssl-dev -y


apt install libsdl2-dev libsdl2-2.0-0 -y
apt install libfdk-aac-dev -y
git clone git@github.com:FFmpeg/FFmpeg.git  ffmpeg
git checkout release/3.3

./configure --disable-yasm --enable-shared  --enable-ffplay--enable-debug=3  --disable-optimizations --disable-mmx --disable-stripping

do it 
 ./configure --disable-yasm --enable-shared  --enable-ffplay --enable-debug=3  --disable-optimizations --disable-mmx --disable-stripping --enable-gpl --enable-nonfree --enable-libfdk-aac



./configure --disable-yasm --enable-shared  --enable-ffplay --enable-debug=3  --disable-optimizations --disable-mmx --disable-stripping --enable-gpl --enable-nonfree --enable-libfdk-aac  --enable-libmp3lame   --enable-nonfree --enable-libx264


apt-get install -y libx264-dev

wget https://downloads.sourceforge.net/lame/lame-3.100.tar.gz && \
tar xvf lame-3.100.tar.gz && cd lame-3.100 && \
PATH="$HOME/bin:$PATH" \
./configure \
       --enable-shared \

make install



make -j8

make install



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
- git checkout m76
 Note: Remove ffmpeg internal build from webrtc with rtc_use_h264=false


 gn gen out/m85 --args='is_debug=true symbol_level=2 is_component_build=false is_clang=false rtc_include_tests=false rtc_use_h264=true rtc_enable_protobuf=false use_rtti=true use_custom_libcxx=false treat_warnings_as_errors=false use_ozone=true proprietary_codecs=true ffmpeg_branding="Chrome"'

