
./ffmpeg -loglevel trace -hwaccel qsv -c:v h264_qsv -i /var/tmp/reverse.264 -vf 'vpp_qsv=w=640:h=360' -c:v h264_qsv /tmp/output.264

lspci
lspci | grep VGA


ffmpeg complile 


ffmpeg -loglevel trace -vsync 0 -hwaccel cuda -hwaccel_output_format cuda -i /var/tmp/test.264 -vf scale_cuda=640:480 -c:a copy -c:v h264_nvenc -b:v 5M /tmp/test.264


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

  ./configure --pkg-config-flags="--static" --libdir=/usr/local/lib --incdir=/usr/local/cuda/include --enable-cuda-nvcc --enable-libnpp --extra-cflags=-I/usr/local/cuda/include --extra-ldflags=-L/usr/local/cuda/lib64  --disable-shared --enable-debug=2 --disable-optimizations --enable-static --enable-gpl --enable-pthreads --enable-nonfree  --enable-libfdk-aac --enable-libx264 --enable-filters --enable-runtime-cpudetect --disable-lzma --disable-stripping --disable-vaapi







oth nvidia and mediasdk

 ./configure --pkg-config-flags="--static" --libdir=/usr/local/lib --incdir=/usr/local/cuda/include --enable-cuda-nvcc --disable-libnpp --extra-cflags=-I/usr/local/cuda/include --extra-cflags=-I/opt/intel/mediasdk/include --extra-ldflags=-L/usr/local/cuda/lib64  --extra-ldflags=-L/opt/intel/mediasdk/plugins --extra-ldflags=-L/opt/intel/mediasdk/lib --disable-shared --enable-debug=2 --disable-optimizations --enable-static --enable-gpl --enable-pthreads --enable-nonfree  --enable-libfdk-aac --enable-libx264 --enable-filters --enable-runtime-cpudetect --disable-lzma --disable-stripping --enable-vaapi --enable-libmfx

  --enable-libdrm \







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



i5-6300U CPU @ 2.40GHz
6th generation graphics

modinfo nvidia
