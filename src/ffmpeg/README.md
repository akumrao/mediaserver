ffmpeg complile 

git clone git@github.com:FFmpeg/FFmpeg.git  ffmpeg
git checkout release/3.3

./configure --disable-yasm --enable-debug=3 --disable-podpages
