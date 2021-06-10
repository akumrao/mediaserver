fMP4


**fMP4's are structured in boxes as described in the ISOBMFF spec.**

https://www.w3.org/TR/mse-byte-stream-format-isobmff/

https://github.com/gpac/mp4box.js


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
