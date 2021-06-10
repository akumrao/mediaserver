fMP4


fMP4's are structured in boxes as described in the ISOBMFF spec.

For a basic fMP4 to be valid it needs to have the following boxes:

ftyp (File Type Box)
moov (Movie Header Box)
moof (Movie Fragment Box)
mdat (Movie Data Box)
Every fMP4 stream needs to start with an ftyp and moov box which is then followed by many moof and mdat pairs.

It is important to understand that when you append your first segment to Media Source Extensions that this segment will need to start with an ftyp and moov followed by a moof and mdat. A segment containing a ftyp and moov box is often referred to as an Initialization Segment(init) segment, and segments containing moof and mdat boxes, referring to media itself as Media Segments.
