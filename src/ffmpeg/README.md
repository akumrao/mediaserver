ffmpeg complile 


git clone git@github.com:FFmpeg/FFmpeg.git ffmpeg git checkout release/3.3

./configure --disable-yasm --enable-shared --enable-debug=3 --disable-optimizations --disable-mmx --disable-stripping

./configure --disable-yasm --enable-shared --enable-debug=3 --disable-optimizations --disable-mmx --disable-stripping --enable-gpl --enable-nonfree --enable-libfdk-aac

make -j8




play pcm
ffplay -autoexit -f u16be -ar 44100 -ac 1 in.raw


ffmpeg -f s16le -ar 48000 -ac 2 -i  /var/tmp/test.mp3 file.wav



ffmpeg -i /var/tmp/test.mp3 -ar 48000 -ac 2 -f s16le out.pcm





-acodec pcm_s16be: Output pcm format, signed 16 encoding, endian is big end (small end is le);
-ar 16000: The sampling rate is 16000
-ac 1: the number of channels is 1


