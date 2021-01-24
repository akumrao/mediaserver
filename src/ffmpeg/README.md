ffmpeg complile 


git clone git@github.com:FFmpeg/FFmpeg.git ffmpeg git checkout release/3.3

./configure --disable-yasm --enable-shared --enable-debug=3 --disable-optimizations --disable-mmx --disable-stripping

./configure --disable-yasm --enable-shared --enable-debug=3 --disable-optimizations --disable-mmx --disable-stripping --enable-gpl --enable-nonfree --enable-libfdk-aac

make -j8
