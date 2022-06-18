ffmpeg complile 


git clone git@github.com:FFmpeg/FFmpeg.git ffmpeg git checkout release/3.3

./configure --disable-yasm --enable-shared --enable-debug=3 --disable-optimizations --disable-mmx --disable-stripping

./configure --disable-yasm --enable-shared --enable-debug=3 --disable-optimizations --disable-mmx --disable-stripping --enable-gpl --enable-nonfree --enable-libfdk-aac

make -j8


apt install libfdk-aac-dev -y


play pcm
ffplay -autoexit -f s16le -ar 48000 -ac 2 /var/tmp/out.pcm


ffmpeg -f s16le -ar 48000 -ac 2 -i  /var/tmp/test.mp3 file.wav



ffmpeg -i /var/tmp/test.mp3 -ar 48000 -ac 2 -f s16le out.pcm





-acodec pcm_s16be: Output pcm format, signed 16 encoding, endian is big end (small end is le);
-ar 16000: The sampling rate is 16000
-ac 1: the number of channels is 1



avcodec_find_decoder_by_name , do not find decoder with id  avcodec_find_decoder(AV_CODEC_ID_H264);


And there is no h264_videotoolbox decoder, only encoder. To list decoders/encoders available:

ffmpeg -encoders
ffmpeg -decoders

*FFMPEG MINIMAL Steps from Sanjay*



ffmpeg complile

git clone git@github.com:FFmpeg/FFmpeg.git ffmpeg git checkout release/3.3

./configure --disable-yasm --enable-shared --enable-debug=3 --disable-optimizations --disable-mmx --disable-stripping

./configure --disable-yasm --enable-shared --enable-debug=3 --disable-optimizations --disable-mmx --disable-stripping --enable-gpl --enable-nonfree --enable-libfdk-aac

make -j8

apt install libfdk-aac-dev -y

play pcm ffplay -autoexit -f s16le -ar 48000 -ac 2 /var/tmp/out.pcm

ffmpeg -f s16le -ar 48000 -ac 2 -i /var/tmp/test.mp3 file.wav

ffmpeg -i /var/tmp/test.mp3 -ar 48000 -ac 2 -f s16le out.pcm

-acodec pcm_s16be: Output pcm format, signed 16 encoding, endian is big end (small end is le); -ar 16000: The sampling rate is 16000 -ac 1: the number of channels is 1

avcodec_find_decoder_by_name , do not find decoder with id avcodec_find_decoder(AV_CODEC_ID_H264);

And there is no h264_videotoolbox decoder, only encoder. To list decoders/encoders available:

ffmpeg -encoders ffmpeg -decoders

Created ffmpeg_min folder from release3.3 and used following flags to configure ./configure --pkg-config-flags=--static --libdir=/usr/local/lib --disable-shared --enable-debug=2 --disable-optimizations --enable-static --enable-gpl --enable-pthreads --enable-nonfree --enable-libfdk-aac --enable-libx264 --enable-runtime-cpudetect --disable-lzma --disable-encoders --enable-encoder=aac --enable-encoder=h264_nvenc --enable-encoder=libx264 --disable-decoders --enable-decoder=h264 --enable-decoder=h264_cuvid --disable-hwaccels --enable-hwaccel=h264_cuvid --disable-muxers --enable-muxer=h264 --disable-demuxers --enable-demuxer=aac --enable-demuxer=h264 --enable-demuxer=pcm_s32le --enable-demuxer=pcm_s8 --enable-demuxer=pcm_u16le --enable-demuxer=pcm_u32le --disable-parsers --enable-parser=h264 --disable-bsfs --enable-bsf=h264_mp4toannexb --disable-protocols --enable-protocol=file --disable-indevs --disable-outdevs --enable-outdev=sdl2 --disable-filters --enable-decoder=aac --enable-filter=scale --enable-filter=aresample --disable-doc --disable-htmlpages --disable-manpages --disable-podpages --disable-txtpages --disable-avdevice --disable-postproc --disable-lsp --disable-lzo --disable-faan --disable-pixelutils --disable-ffprobe --disable-ffmpeg --disable-network --disable-ffserver --enable-demuxer=mov
