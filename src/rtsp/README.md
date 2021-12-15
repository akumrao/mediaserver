RTSP

--- a/ext/live/config.linux
      4 +++ b/ext/live/config.linux-64bit
      5 @@ -1,4 +1,4 @@
      6 -COMPILE_OPTS =         $(INCLUDES) -I/usr/local/include -I. -O2 -DSOCKLEN_T=socklen_t -D_LARGEFILE_SOURCE=1 -D_FILE_OFFSET_BITS=64
      7 +COMPILE_OPTS =         $(INCLUDES) -I/usr/local/include -I. -g -fpic -DSOCKLEN_T=socklen_t -D_LARGEFILE_SOURCE=1 -D_FILE_OFFSET_BITS=64

same for 


./genMakefiles linux-64bit


leaks

to fix 

ffmpeg -i myVideo.mp4 -y -c:v libx264 -profile:v high -prese6t:v fast -deinterlace -x264opts min-keyint=15:keyint=1000:scenecut=20 -b:v 2000k -c:a aac -b:a 128k -f segment -segment_format mp4 -segment_format_options movflags=empty_moov+frag_keyframe+default_base_moof+skip_trailer+faststart /home/1/output%%05d.mp4


ffmpeg -i myVideo.mp4 -y -c:v libx264 -profile:v high -prese6t:v fast -deinterlace -x264opts min-keyint=15:keyint=1000:scenecut=20 -b:v 2000k -c:a aac -b:a 128k -f segment -segment_format mp4 -segment_format_options movflags=empty_moov+frag_keyframe+default_base_moof+skip_trailer+faststart /home/1/output%%05d.mp4


dashenc.c
View File
@ -1507,12 +1507,12 @@ static int dash_init(AVFormatContext *s)
            if (c->streaming)
                // skip_sidx : Reduce bitrate overhead
                // skip_trailer : Avoids growing memory usage with time
                av_dict_set(&opts, "movflags", "dash+delay_moov+skip_sidx+skip_trailer", 0);
                av_dict_set(&opts, "movflags", "+dash+delay_moov+skip_sidx+skip_trailer", AV_DICT_APPEND);



**fMP4's are structured in boxes as described in the ISOBMFF spec.**

https://www.w3.org/TR/mse-byte-stream-format-isobmff/

https://github.com/gpac/mp4box.js

https://www.programmersought.com/article/37756013328/

For a basic fMP4 to be valid it needs to have the following boxes:

ftyp (File Type Box)
moov (Movie Header Box)
moof (Movie Fragment Box)
mdat (Movie Data Box)
Every fMP4 stream needs to start with an ftyp and moov box which is then followed by many moof and mdat pairs.

It is important to understand that when you append your first segment to Media Source Extensions that this segment will need to start with an ftyp and moov followed by a moof and mdat. A segment containing a ftyp and moov box is often referred to as an Initialization Segment(init) segment, and segments containing moof and mdat boxes, referring to media itself as Media Segments.


----------------------------------------------------------------------------------------------------------------------------------------------------------------
MP4Box: fragmentation, segmentation, splitting and interleaving

Please refer to this page for more up to date information: https://github.com/gpac/gpac/wiki/Fragmentation,-segmentation,-splitting-and-interleaving
 

NOTE: The latest documentation describing MP4Box Support for DASH is given here.

With our work on Dynamic Adaptive Streaming over HTTP (DASH), in the current version of GPAC (revision 2642 on SVN), we now have many options for interleaving, fragmenting and segmenting … which may be confusing. It is time to clarify their usage in MP4Box. Some related aspects using MPEG-2 TS instead of MP4 can be seen in this previous post.

The options are:
-inter time_in_ms (with possibly -tight) or -flat (no interleaving)
-frag time_in_ms
-dash dur_in_ms
-split time_sec (or -split-size)

Interleaving (-inter) is when (groups of ) samples of different tracks are stored alternatively in the file: e.g. N milliseconds of video samples, followed by N milliseconds of audio samples, followed by N milliseconds of video samples … Typically, interleaved samples are grouped within an interleaving window. Interleaving reduces disk accesses, playback buffer requirements and enables progressive download and playback.

Fragmentation (-frag) is an optional process applicable to the MP4 file format. By default, MP4 files generated with MP4Box are not fragmented. This process consists in using Movie Fragments (moof). Movie Fragments is a tool introduced in the ISO spec to improve recording of long-running sequences and that is now used for HTTP streaming. Even if it is possible, according to the ISO spec, to do interleaving on fragments, MP4Box currently does not support it, because we don’t see important use cases for it. For instance, all audio samples within a fragment are contiguously stored and similarly for the video samples. The only way to ‘interleave’ tracks is to have small fragments. There may be some overhead for big files, we welcome comments on this.

Segmentation (-dash) is the process of creating segments, parts of an original file meant for individual/separate HTTP download (not necessarily for individual playback). A segment can be a part of a big file or a separate file. It is not specific to the MP4 file format (in particular it may apply to MPEG-2 TS) but a segment may imply specific ISO signaling (styp and sidx boxes, for instance). A segment is what is refered to by the XML file used to drive the HTTP Streaming and segment boundaries can be convenient places for bitstream switching. Segmentation often implies fragmentation but not necessarily.

Last, MP4Box can split (-split) a file and create individual playable files from an original one. It does not use segmentation in the above sense, it removes fragmentation and can use interleaving.

Some examples of MP4Box usages:
– Rewrites a file with an interleaving window of 1 sec.
MP4Box -inter 1000 file.mp4

– Rewrites a file with 10 sec. fragments
MP4Box -frag 10000 file.mp4

– Rewrites a file (and creates the associated XML) with 10 sec. fragments and 30 sec. segments
MP4Box -frag 10000 -dash 30000 file.mp4

– Segmentation of a file into 30 sec. segment files with 10 sec. fragments (and creation of the associated XML file.mpd)
MP4Box -frag 10000 -dash 30000 -segment-name segment_file file.mp4


----------------------------------------------------------------------------------------------------------------------------------------------------------------




Check key frame alignment with MP4Box
jeanlf edited this page on 4 May 2020 · 2 revisions
HOME » HOWTOs » DASH & HLS » Key-frame alignment

When packaging encoded content for DASH, a lot of issues come from misalignment of key-frames across the different encoded qualities. This page describes how to check key-frame alignment.

A packager like MP4Box doesn't re-encode your content

MP4Box does two things for you:

Import: It understands your media to import it to the MP4 container.
For example: MP4Box -add video.h264 -add audio.aac av.mp4
When MP4Box doesn't understand the format, you may want to specify manually this step with NHML.

Manipulates: MP4Box manipulates the MP4 container (e.g. edit, fragment, cut, dash, encrypt, etc.).
One key feature of the MP4 container is the ability to manipulate content without any knowledge about the content format. Theoretically it means that MP4Box can package some MPEG-DASH content even for a codec it would not know.

But in any case MP4Box does not re-encode the content. For that, please use an encoder (such as FFmpeg - see references at the end of the article). It is your responsibility, as the content editor, to feed MP4Box with some appropriate content at the encoder level. If your content is not prepared correctly, MP4Box works on a best-effort basis and may (or may not) do its job. MP4Box may or may not print warnings. But some players (like dash.js for MPEG-DASH) may silently fail with the packaged content.

###Command-line to get quick summary of key-frames intervals

$ MP4Box -info TRACK_ID source1.mp4 2>&1 | grep GOP
You will get the average key-frame interval computed for the track with ID TRACK_ID:

Average GOP length: 25 samples
Having different numbers for the average GOP length on different files mean that your GOP size differ and key-frames won't be aligned across various qualities: you will have to re-encode. However there may be cases where the average GOP length is the same, but slight variations may occur resulting in misalignment when DASHing.

###Command-line to get complete key-frames list and indexes to check alignment

$ MP4Box -std -diso source1.mp4 2>&1 | grep SyncSampleEntry > 1.txt
You'll get:

<SyncSampleEntry sampleNumber="1"/>
<SyncSampleEntry sampleNumber="121"/>
<SyncSampleEntry sampleNumber="241"/>
<SyncSampleEntry sampleNumber="361"/>
<SyncSampleEntry sampleNumber="481"/>
<SyncSampleEntry sampleNumber="601"/>
<SyncSampleEntry sampleNumber="721"/>
...
Then do it with another source file, and compare:

$ MP4Box -std -diso source1.mp4 2>&1 | grep SyncSampleEntry > sync1
$ MP4Box -std -diso source2.mp4 2>&1 | grep SyncSampleEntry > sync2
$ diff sync1 sync2

-----------------------------------------------------------------------------------------------------------------------------------------------------------



DASH Streaming Support
1 February 2012General, GPACDASH, HLS, HTTP Streaming, MP4Box, MPEG, segmentationJean
NOTE: This post is not the most up-to-date documentation of DASH support in GPAC. For the latest information on DASH, check out these pages:

DASH Support in MP4Box
DASH Support in GPAC Players
We have been updating the support for DASH Streaming (ISO/IEC 23009-1) in GPAC as of revision 3849. The update covers both content generation tools and player, and both DASH and HLS support.

For more details on what is DASH and HTTP streaming, please refer to this post.

DASH Basics: MPD and Segments
Let’s quickly summarize how a DASH content is made of:

MPD: an XML document describing where the various media resources present in the content are located. The media resources can be single-media (for example, a video-only MP4 file) or a multiplexed set of streams (for example an AV MPEG-2 Transort Stream). Streams can be scalable (such as SVC) but we won’t go into such details as GPAC doesn’t support advanced description of scalable streams in DASH. Some media resources may exist in different versions, for example different bitrate or language or resolutions. In DASH, such a “version” of the stream is called a representation, and all representations are grouped together in an AdaptationSet.
segment: a continuous part of a media resource. The segment is the smallest part of the media that can be located in the MPD. What a segment exactly contains depends on the underlying media format of the content.
subsegment: a continuous part of a segment, or of a subsegment.
sidx: short name for SegmentIndexBox, this is an ISOBMF (MP4/3GP container) structure describing a segment by giving its earliest presentation time, how the segment is further divided into subsegments, random access points locations (byte offset) and timing in the segment payload. The goal of the SIDX is to build an index of the segment at a given granularity to simplify trick modes (seeking, fast-forward, fast-rewind, …).
There are several ways to refer to a segment in an MPD. If the file is made of a single segment (-single-segment option for ISOBMF), one will likely use SegmentBase element. If a file is made of several segments, each segment will be identified by the SegmentList syntax in the MPD, using byte ranges. For other cases, we need to instruct MP4Box how to refer to segments (and how to store them as well). The following switches are defined:

-segment-ext EXT: tells MP4Box to generate segments with EXT extension (by default m4s for ISOBMF and or ts for MPEG-2)
-segment-name NAME: tells MP4Box to generate each segment in a dedicated file, called NAME%d.EXT. NAME can also have %s in it, in which case %s will be replaced by the name of the file being dashed without folder indication and extension. By default, such segments will be stored using the SegmentList syntax in the MPD.
-url-template: if set when generating segements in different files, the segments will be refered to using the SegmentTemplate syntax in the MPD.
ISO Base Media File Format
For content based on ISOBMF (ISO/IEC 14496-12), MP4Box can be used to cut files into DASH segments. Before going any further, some definitions will be needed:

segment: for ISOBMF, a segment is a consecutive set of movie fragments. Each movie fragment is composed of a moof box followed by mdat box(es), and all data adressing in the mdat(s) are done using relative offsets in the moof.
subsegment: a part of a segment, made of a consecutive set of movie fragments. A subsegment can be further divided in subsegments, until only a single movie fragment per subsegment is present.
With that in mind, we can generate DASH content by playing with the following MP4Box parameters:

-dash X: produce segments of roughly X milliseconds.
-frag Y: use movie fragments of roughly Y milliseconds. By default, fragments duration is 500 milliseconds.
-rap:  attempts to cut segments so that they all start with an access point (IDR, I-frame or beginning of a gradual decoding refresh for example).
-subsegs-per-sidx N: specifies how many subsegments per sidx we would like. This only covers the first level of segment spliting (MP4Box doesn’t handle subsegments subdivision into subsegments). Noticable values are:
<0: disable: sidx will not be produced
0: a single sidx box is used for the entire segment, and each subsegment is made of a single movie fragment (i.e., there will be X/Y subsegments in sidx). This is the default value.
>0: produces X/Y/N subsegments referenced in the first sidx.
-daisy-chain: this is only used when producing multiple subsegments per segment (-subsegs-per-sidx). If specified, subsegments will be described in SIDX in which the last entry (subsegment) points to the next SIDX. Otherwise, multiple SIDXs will be stored in a hierarchical way, with the first SIDX pointing to each SIDX of the subsegments.
 -single-segment: special mode indicating that the file should be segmented as one single segment. In that case, the dash duration X becomes the subsegment duration, and a single sidx is produced before any movie fragment.
Now let’s see an example.

Dashing a file with 10 seconds, rap-aligned segments with a fragment duration (i.e. subsegment duration since we don’t subdivide the SIDX)  of 1 sec:

MP4Box -dash 10000 -frag 1000 -rap test.mp4
The same with a separated segment using template addressing, and 5 subsegments per segments:

MP4Box -dash 10000 -frag 1000 -rap -segment-name myDash
          -subsegs-per-sidx 5 -url-template test.mp4
Generating an onDemand profile DASH file (single segment) is just as simple:

MP4Box -dash 10000 -frag 1000 -rap -single-segment test.mp4


MP4Box ( gpac) is c source code very compklex to understand. Please use Bento4 C++ easy to understand 
-------------------------------------------------------------------------------------------------------------------------------
for mp3 original codec copy
fmpeg -i quintin.mp3 -codec copy -f mp4 -movflags empty_moov+omit_tfhd_offset+frag_keyframe+default_base_moof  /tmp/output3.mp4


ffmpeg -i test.264  -i test.aac -f mp4 -movflags empty_moov+omit_tfhd_offset+frag_keyframe+default_base_moof /tmp/output1.mp4

ffmpeg -i test.264  -i test.aac -f mp4 -movflags empty_moov+omit_tfhd_offset+separate_moof+frag_custom /tmp/output2.mp4


ffmpeg -i kunal720.264  -i kunal720_track2.aac -f mp4 -movflags empty_moov+omit_tfhd_offset+frag_keyframe+default_base_moof /tmp/output1.mp4

ffmpeg -i kunal720.264  -i kunal720_track2.aac -f mp4 -movflags empty_moov+omit_tfhd_offset+separate_moof+frag_custom /tmp/output2.mp4

source code of ffmpgeg
ffmpege encode pcm to aac
https://ffmpeg.org/doxygen/trunk/encode__audio_8c_source.html

https://ffmpeg.org/doxygen/trunk/muxing_8c-example.html

online mp4 parser

http://download.tsi.telecom-paristech.fr/gpac/mp4box.js/filereader.html
https://www.onlinemp4parser.com/




  H.264 comes in a variety of stream formats. One variation is called "Annex B".

(AUD)(SPS)(PPS)(I-Slice)(PPS)(P-Slice)(PPS)(P-Slice) ... (AUD)(SPS)(PPS)(I-Slice).
  
 ALL the NAL Unit start with 001

 
0x67=  11 00111 =  type7   Sequence parameter set ( I-frame)  0x67 = (103)
0x68=  11 01000 =  type8   Piture parameter set ( I-frame) (104)
  same case for p frame( SPS and PPS are same for all Mp4 store SPS and PPS separately for streaming we need sps ad pps  very frequently                     
0x65 = 11 00101 = type 5   Coded slice of an IDR picture (I-frame)
0x41 = 10 00001 = type 1   Coded slice of a non-IDR picture (P-frame)

0x27 = 01 00111 = type 7    Sequence parameter set (B-frame)
0x28 = 01 01000 = type 8    Picture parameter set (B-frame)   
0x25 = 01 00101 = type 5    Coded slice of an IDR picture (B-frame) //The first picture in a coded video sequence is always an IDR picture. An IDR frame is a special type of I-frame in H. 264. An IDR frame specifies that no frame after the IDR frame can reference any frame before it.
0x21 = 01 00001 = type 1    Coded slice of a non-IDR picture (B-frame)
 
* |0|1|2|3|4|5|6|7|
+-+-+-+-+-+-+-+-+
  |F|NRI|  Type   |   F 0 forbidden_zero_bit. This bit must be 0 in the H.264 specification.
  NRI(nal_ref_idc) for  00((none) I(11) P(10) B01
  * 
  * Access Unit Delimiter (AUD). An AUD is an optional NALU that can be use to delimit frames in an elementary stream. It is not required (unless otherwise stated by the container/protocol, like TS), and is often not included in order to save space, but it can be useful to finds the start of a frame without having to fully parse each NALU.
  
  IDR Picture is first MB, reset MB could be grouped into slices. Also check Partioning of NALU with A B C

  I-slice is a portion of a picture composed of macroblocks, all of which are based upon macroblocks within the same picture.
  Thus, H.264 introduces a new concept called slices — segments of a picture bigger than macroblocks but smaller than a frame.
  Just as there are I-slices, there are P- and B-slices. P- and B-slices are portions of a picture composed of macroblocks that are not dependent on macroblocks in the same picture. 
  
  H264 encoder sends an IDR (Instantaneous Decoder Refresh) coded picture (a group of I slices ) to clear the contents of the reference picture buffer. When we send an IDR coded picture, the decoder marks all pictures in the reference buffer as ‘unused for reference’. All subsequently transmitted slices are decoded without reference to any frame decoded prior to the IDR picture. However, the reference buffer is not cleared with an I frame i.e, any frame after an I frame can use the reference buffer before the I frame. The first picture in a coded video sequence is always an IDR picture.

An IDR frame( Kye frame having sps and pps) is a special type of I-frame in H.264. An IDR frame specifies that no frame after the IDR frame can reference any frame before it. This makes seeking the H.264 file easier and more responsive to the player.

The IDR frames are introduced to avoid any distortions in the video when you want to skip/forward to some place in the video or start watching in the middle of the video.



 00 00 00 01 is the NALU header, the beginning of the sequence identifier,
 0x27 to binary is 100111, 00111 to decimal is 7, then 7 corresponds to NALU type=sps,
 0x28 is 101000 in binary, 8 in decimal, 8 corresponds to NALU type=pps,
 0x25 converted to binary is 100101, 00101 converted to decimal is 5, corresponding to 5 NALU type=IDR frame
 (Using FFMPEG, sps and pps are saved in the extradata.data of AVCodecContext. When decoding and extracting sps and pps, you can use extradata.data[ 4 ]&0x1f to judge the NALU type (the result is 7 is sps, 8 is pps, calculation method Is converted to binary first, 0x27&0x1f=11111&00111=00111=7, pps calculation is similar)),
 NALU type=1 is splice. There are three coding modes for splice. I_slice, P_slice, B_slice, and I-frames are divided and stored in splice when coding. 


The full name of GOP is the Group of picture image group, that is, the distance between two I frames. The larger the GOP value, the P between I frame rates The greater the number of frames and B frames, the finer the image quality. If the GOP is 120, if the resolution is 720P, and the frame rate is 60, then the time for two I frames is 120/60=2s.


nal_unit_type
![Alt text](/images/30-Table2.1-1.png?raw=true "nal_unit_type")

![Alt text](/images/h264.png?raw=true "H264")

![Alt text](/images/pic_management.png?raw=true "H264")

![Alt text](/images/data_partitioning.png?raw=true "H264")




Media Source Extensions for Audio

https://developers.google.com/web/fundamentals/media/mse/seamless-playback




What is video timescale, timebase, or timestamp in ffmpeg


Modern containers govern the time component of presentation of video (and audio) frames using timestamps, rather than framerate. So, instead of recording a video as 25 fps, and thus implying that each frame should be drawn 0.04 seconds apart, they store a timestamp for each frame e.g.

 Frame      pts_time
   0          0.00
   1          0.04
   2          0.08
   3          0.12
   ...
For the sake of precise resolution of these time values, a timebase is used i.e. a unit of time which represents one tick of a clock, as it were. So, a timebase of 1/75 represents 1/75th of a second. The Presentation TimeStamps are then denominated in terms of this timebase. Timescale is simply the reciprocal of the timebase. FFmpeg shows the timescale as the tbn value in the readout of a stream.

Timebase = 1/75; Timescale = 75
 Frame        pts           pts_time
   0          0          0 x 1/75 = 0.00
   1          3          3 x 1/75 = 0.04 
   2          6          6 x 1/75 = 0.08
   3          9          9 x 1/75 = 0.12
   ...
This method of regulating time allows variable frame-rate video.

Share
Improve this answer
Follow
answered Apr 11 '17 at 5:12

Gyan
63.6k77 gold badges102102 silver badges142142 bronze badges
2
The timebase can be any number that at least respects the FPS i.e. for 25 fps, it should be at least 1/25. If it is 1/15, then depending on the muxer, ffmpeg will either drop frames or alter the output framerate to 15. Framerates can be fractional hence a rational number. Timebases are rational because they represent fractions of a second. – Gyan Nov 12 '17 at 5:23
20
The reason for the typical use of 90,000 as a common base of calculation is that it is a number which is divisible by 24, by 25, and by 30 (in each case the result is an integer - there is no remainder, decimal or fraction), thus the maths is equally suitable for handling 24 frames per second, 25 fps, and 30 fps. – Ed999 Dec 3 '17 at 3:43
5
@Ed999 is correct that 90000 is an integral multiple of 24, 25 and 30 but that is not the reason. 600 suffices for that purpose, and Quicktime writers typically use that value for timescale. – Gyan Jun 26 '19 at 14:13
3
"For notational convenience, equations in which PCR, PTS, or DTS appear, lead to values of time which are accurate to some integral multiple of (300 × 2^33/system_clock_frequency) seconds. This is due to the encoding of PCR timing information as 33 bits of 1/300 of the system clock frequency plus 9 bits for the remainder, and encoding as 33 bits of the system clock frequency divided by 300 for PTS and DTS." – Gyan Jun 26 '19 at 14:13
3
27 Mhz / 300 = 90000 Hz. – Gyan Jun 26 '19 at 14:13
Show 7 more comments















/////////////////////////////////////////////////////////////////////////////////////////////


Ihttps://github.com/haywire/haywire

apt-get install postgresql postgresql-client postgresql-contrib libpq-dev

https://github.com/libuv/libuv/issues/570
https://github.com/delaemon/libuv-tutorial/blob/master/pipe-echo-server.c
https://github.com/willemt/uv_multiplex/tree/master/src

-DLWS_WITH_LWSWS=1

cmake -DLWS_WITH_LWSWS=1 -DCMAKE_BUILD_TYPE=Debug .. 



runfmp4: ../../libuv/src/unix/stream.cpp:942: void uv__write_callbacks(uv_stream_t*): Assertion `uv__has_active_reqs(stream->loop)' failed.



http://5.9.10.113/65744845/decoding-aac-audio-with-ffmpeg

https://18.118.78.176:8080/speedtest.html
http://18.118.78.176:3000/

 Malformed AAC bitstream detected: use the audio bitstream filter 'aac_adtstoasc' to fix it ('-bsf:a aac_adtstoasc' option with ffmpeg)

fmpeg -i clip-1.mp4 -bsf:a aac_adtstoasc -acodec copy -vcodec copy -ss 0 -t 00:43:00 OUTFILE.mp4


ffmpeg -i test.aac  -bsf:a aac_adtstoasc -acodec copy - -ss 0  arvind.aac

malformated aac bitstream, use -absf aac_adtstoasc doesn't mean the input stream is invalid. It's rather the re-encoding that's invalid. The idea is that you're using an MPEG-2 transport stream as input and the AAC stream in it uses the ADTS format.

If you want to re-encode the file and change the container to FLV or MP4 for example you MUST use the bitstream filter aac_adtstoasc to remove the ADTS header:

-bsf:a aac_adtstoasc
To view the input streams you can use ffprobe. Eg:

ffprobe -v quiet -print_format json -show_streams [input]
eg -i <infile> -vcodec copy -acodec copy -absf aac_adtstoasc
> > out.flv
>
> > If I re-encode the audio stream to aac then it works ok. So is this a
> > problem with ffmpeg removing the adts headers? Or would you know of
> > any reasons why crtmpserver can not read the audio?


imp

https://www.programmersought.com/article/99832446840/#movie-fragments


root:build# apt-get install g++-4.8 

apt-get install gcc-4.8 





https://webrtchacks.com/web-audio-conference/
audo merger


mp4box.onReady
https://webrtc.github.io/samples/src/content/datachannel/datatransfer/



very important 

https://hacks.mozilla.org/2015/07/streaming-media-on-demand-with-media-source-extensions/

https://www.onlinemp4parser.com/
http://download.tsi.telecom-paristech.fr/gpac/mp4box.js/filereader.html
MP4Box.js ()
mp4info input.mp4
mp4dump input.mp4

usemp4fragment, The command format is (specify fragment time is 2s):

 input.mp4 fragmented.mp4




Structure of original .mp4 file:

___________
|moov|mdat|
-----------

moov - metadata (timestamp, video duration, codecs etc..)
mdat - video


Structure of fragmented .mp4 file:


  initial     fragment1    fragment2    fragment3 
___________  ___________  ___________  ___________
|   moov  |  |moof|mdat|  |moof|mdat|  |moof|mdat|
-----------  -----------  -----------  -----------
moov - general data (codecs, video chunks size etc..)
moof - metadata of current video fragment
mdat - video fragment


https://qiita.com/binzume/items/3ee9bdd590f92fa45fcf




https://stackoverflow.com/questions/54119705/fragmented-mp4-problem-playing-in-browser
http://underpop.online.fr/f/ffmpeg/help/mov_002c-mp4_002c-ismv.htm.gz


javascript framgmented mp4 streaming


https://github.com/lieff/minimp4







ffmpeg -i infile.ext -g 52 \
-strict experimental -acodec aac -ab 64k -vcodec libx264 -vb 448k \
-f mp4 -movflags frag_keyframe+empty_moov \
output.mp4
(Hint: Do not user -r 52  if not necessary)  g key frame repeat after 52

If you don't have the bitrates from the input file, use ffprobe to get it for all streams:







https://github.com/axiomatic-systems/Bento4

 python3 -m http.server 8000


ffmpeg -i test4.mp3  -codec:a libmp3lame converted_file1.mp4
ffmpeg -i test4.mp3 -acodec aac -b:a 256000 /tmp/output.m4a
https://icecast.org/download/
https://tldp.org/HOWTO/MP3-HOWTO-11.html#ss11.7
https://tldp.org/HOWTO/MP3-HOWTO-11.html#ss11.7
https://github.com/xiph
https://github.com/kevinGodell/mp4frag/issues/1
https://stackoverflow.com/questions/47268020/fragmented-mp4-doesnt-show-up-in-chrome





usrsctp_register_address

git clone  https://github.com/sctplab/usrsctp
 cd usrsctp/
./bootstrap
./configure
 make install
git clone https://github.com/gabime/spdlog
mkdir build && cd build
root:build# cmake .. && make -j

wrong way do not do it 
apt install python3-pip
pip3 install meson

sudo pip3 install meson
meson build --prefix=/usr
ninja -C build
sudo ninja -C build install


correct way

download libnice-0.1.11.tar.gz    from  https://www.mirrorservice.org/sites/distfiles.macports.org/libnice/

./configure   --disable-static  --without-gstreamer-0.10

make 

make install


cmake -DCMAKE_BUILD_TYPE=Debug ..


https://stackoverflow.com/questions/14273978/integrating-websockets-with-a-standard-http-server/14278738









in the case of WebRTC, SCTP is tunneled over a secure DTLS tunnel, which itself runs on top of UDP.

sctp

https://hpbn.co/webrtc/


https://mediasoup.discourse.group/t/sctp-over-udp/623/6
https://mediasoup.discourse.group/t/sctp-over-udp/623/8




https://github.com/TopPano/librtcdc/blob/master/src/sctp.c
https://github.com/sctplab/usrsctp/issues/147



g_socket_bind

binary file transfer
https://webrtc.github.io/samples/src/content/datachannel/datatransfer/

























        if (MediaSource.isTypeSupported(mimeCodec)) {
          const sourceBuffer = mediaSource.addSourceBuffer(mimeCodec);

          for (let chunk of media) {
            await new Promise(resolve => {
              sourceBuffer.appendBuffer(chunk.mediaBuffer);
              sourceBuffer.onupdateend = e => {
                sourceBuffer.onupdateend = null;
                sourceBuffer.timestampOffset += chunk.mediaDuration;
                console.log(mediaSource.duration);
                resolve()
              }
            })













https://gitmemory.com/issue/gpac/mp4box.js/178/496050884

faac
https://www.programmersought.com/article/26434403620/







wget https://downloads.sourceforge.net/lame/lame-3.100.tar.gz && \
tar xvf lame-3.100.tar.gz && cd lame-3.100 && \
PATH="$HOME/bin:$PATH" \
./configure \
       --prefix="$HOME/lame_build" \
       --bindir=$HOME/bin \
       --enable-static \
       --disable-shared \
       --enable-nasm  && \
PATH="$HOME/bin:$PATH" make && \
make install

MP4Box -add video.h264 video.mp4

faac --mpeg-vers 4 -b 128 -c 44100 myfile.wav -o myfile.aac
Mux into MP4: H.264 video and and AAC audio
Code:
MP4Box -add myfile.264:fps=29.97 -add myfile.aac myfile.mp4


MP4Box -frag 10000 file.mp4

mp4box -add track1.h264:fps=24 -add track2.aac#audio -new res.mp4

MP4Box  -raw 1 -raw 2 ./kunal720.mp4

MP4Box -add kunal720_track1.264 -add kunal720_track2.aac -new res.mp


Generate fragmented mp4 video with random access points just like MP4Box does
Run the following command to put the header at the front of the file and to ensure that the fragments start with Random Access Points:
MP4Box -dash 1000 -rap -frag-rap videocodec.mp4

https://medium.com/google-developers/use-video-loops-with-interactive-canvas-dc7503e95c6a


MP4Box -dash 1000 -rap -frag-rap -add kunal720_track1.264 -add kunal720_track2.aac -new res1.mp4

MP4Box -dash 1000 -rap -frag-rap -add /experiment/fmp4/kunal720_track1.264 -add /experiment/fmp4/kunal720_track2.aac -new res1.mp4

Loop in dash-live


Loop in dash-live with growing file #835
mp4box -dash-live 2000 -fps 25 -frag 1000 -profile "dashavc264:live" -mpd-title "DASH-LIVE stream" -rap -frag-rap -insert-utc -mpd-refresh 30 -time-shift 1200 -min-buffer 2000 -bs-switching no -out /streaming/live/dash.mpd /tmp/longTC_2.mp4:role=main

Also tested with:

mp4box -dash-ctx /streaming/live/dash-live.txt -dash-live 2000 -profile "dashavc264:live" -mpd-title "DASH-LIVE stream" -ast-offset 12 -no-frags-default -rap -frag-rap -insert-utc -mpd-refresh 30 -time-shift 1200 -min-buffer 2000 -bs-switching no -out /streaming/live/dash.mpd -subsegs-per-sidx -1 /tmp/longTC_2.mp4:role=main



MP4Box -dash-live 2000 -subdur 4000 -closest -no-cache -profile dashavc264:live -subsegs-per-sidx -1 -mpd-refresh 8 -time-shift 12 -min-buffer 2000 -url-template -insert-utc -bs-switching no -run-for $DUR -dash-ctx ../../live/log.txt -out ../../live/manifest.mpd ../current/1080.mp4#video ../current/720.mp4#video ../current/360.mp4#video ../current/180.mp4#video ../current/audio.mp4:#audio





ffmpeg -i rtsp://10.50.1.29/media/video1 -vcodec copy -f mp4 -reset_timestamps 1 -movflags frag_keyframe+empty_moov -loglevel quiet out.mp4

and then: mp4box -dash 1000 -frag 1000 -frag-rap out.mp4

I can take the MP4Box output (out_dashinit.mp4) and send it through Web Sockets, chunk by chunk, to a JavaScript client that feeds it to the media source API.

However, this is not a good method for live content.




To generate the correct fragmented MP4, you need to pass this to ffmpeg: -movflags empty_moov+omit_tfhd_offset+frag_keyframe+default_base_moof.


empty_moov+omit_tfhd_offset+separate_moof+frag_custom


ffmpeg -re -i test.mp4 -g 52 -c:a aac -b:a 64k -c:v libx264 -b:v 448k -f mp4 -movflags frag_keyframe+empty_moov output.mp4



ffmpeg -i test.264  -i test.aac -f mp4 -movflags empty_moov+omit_tfhd_offset+frag_keyframe+default_base_moof /tmp/output1.mp4

ffmpeg -i test.264  -i test.aac -f mp4 -movflags empty_moov+omit_tfhd_offset+separate_moof+frag_custom /tmp/output2.mp4


ffmpeg -i kunal720.264  -i kunal720_track2.aac -f mp4 -movflags empty_moov+omit_tfhd_offset+frag_keyframe+default_base_moof /tmp/output1.mp4

ffmpeg -i test.264 -f mp4 -movflags empty_moov+omit_tfhd_offset+separate_moof+frag_custom /tmp/output2.mp4

source code of ffmpgeg
ffmpege encode pcm to aac
https://ffmpeg.org/doxygen/trunk/encode__audio_8c_source.html

https://ffmpeg.org/doxygen/trunk/muxing_8c-example.html





know this is an old topic but wanted to answer in case someone else is having the same question.

In general, you can use both the Live and On-Demand DASH profiles for VoD content, for live content you need to use the Live profile. The difference would be that if you have a 1:30 hrs VoD file if you are distributing it using DASH Live Profile you will have a long manifest file with a big segment timeline list and a lot of mp4 (90 * 3600/{segment duration}) segments while if you are using the On-Demand profile you end up having a much shorter manifest file and a single mp4 file.

In the case of Live profile, the player would need to download the segments, while with On-Demand profile it would simply send HTTP byte-range requests to the CDN to target a particular time in the content.

Of course, this would result also in a lower CDN load, because in DASH Live profile, the player would need to download all the segments to play the video, meaning that it would need to download the same mp4 header for each segment.

If you are willing to optimize the content delivery, I strongly recommend adopting On-Demand DASH profile.



export VALKKA_TEST_RTSP_1=rtsp://192.168.0.19:8554/testStream


rtsp://192.168.0.19:8554/mpeg1or2AudioVideoTest



EST: live_muxshmem_test: test 1: ** @@Write FragMP4 to shmem ringbuffer.  10 byte cells to test overflow. **
@TEST: live_muxshmem_test: test 1: ** test rtsp stream 1: rtsp://192.168.0.19:8554/testStream
@TEST: live_muxshmem_test: test 1: starting threads
@TEST: live_muxshmem_test: test 1: registering stream
@TEST: live_muxshmem_test: test 1: playing stream !
Created new TCP socket 3 for connection
InfoFrameFilter: info start dump>> 
InfoFrameFilter: FRAME   : <SetupFrame: timestamp=1623835234697 subsession_index=0 slot=2 / media_type=0 codec_id=28>
In









NT Service or Windows Service
Reference Link:- https://www.wintips.org/how-to-run-an-application-as-service/

Steps:-
1. Download nssm from "http://nssm.cc/download" and unzip in some project specific folder.
2. Start cmd prompt in admin and go to nssm application folder As per your machine architecture
3. type "nssm install".
4. Now fill all basic details like application to run as service, its arguments, startup dir, startup type  and service name.
5. Select run as Local system account.
6. Ensure restart on stop/exit option is selected in case of exit action tab. 
7. click "Install Servcie"
8. Open services in admin and start your service to test for first time.
9. if required, change user account for running service.

