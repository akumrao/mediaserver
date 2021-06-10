fMP4


fMP4's are structured in boxes as described in the ISOBMFF spec.

For a basic fMP4 to be valid it needs to have the following boxes:

ftyp (File Type Box)
moov (Movie Header Box)
moof (Movie Fragment Box)
mdat (Movie Data Box)
Every fMP4 stream needs to start with an ftyp and moov box which is then followed by many moof and mdat pairs.

It is important to understand that when you append your first segment to Media Source Extensions that this segment will need to start with an ftyp and moov followed by a moof and mdat. A segment containing a ftyp and moov box is often referred to as an Initialization Segment(init) segment, and segments containing moof and mdat boxes, referring to media itself as Media Segments.







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