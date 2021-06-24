

#include "frame.h"



Frame::Frame() : mstimestamp(0), n_slot(0), stream_index(-1) {
}

  
Frame::~Frame() {
}
  
 
void Frame::print(std::ostream &os) const {
  os << "<Frame: timestamp="<<mstimestamp<<" stream_index="<<stream_index<<" slot="<<n_slot<<">";
}
 

void Frame::copyMetaFrom(Frame *f) {
  this->n_slot          =f->n_slot;
  this->stream_index=f->stream_index;
  this->mstimestamp     =f->mstimestamp;
}


std::string Frame::dumpPayload() {
  return std::string("");
}


void Frame::dumpPayloadToFile(std::ofstream& fout) {
}


void Frame::reset() {
    this->n_slot          =0;
    this->stream_index=-1;
    this->mstimestamp     =0;
}

bool Frame::isSeekable() {
    return true;
}



void Frame::updateAux() {
}



void Frame::update() {
}


  
BasicFrame::BasicFrame() : Frame(), codec_id(AV_CODEC_ID_NONE), media_type(AVMEDIA_TYPE_UNKNOWN), h264_pars(H264Pars()) {
}


BasicFrame::~BasicFrame() {
}



void BasicFrame::print(std::ostream &os) const {
  os << "<BasicFrame: timestamp="<<mstimestamp<<" stream_index="<<stream_index<<" slot="<<n_slot<<" / ";  //<<std::endl;
  os << "payload size="<<payload.size()<<" / ";
  if (codec_id==AV_CODEC_ID_H264) {os << h264_pars;}
  else if (codec_id==AV_CODEC_ID_PCM_MULAW) {os << "PCMU: ";}
  os << ">";
}


std::string BasicFrame::dumpPayload() {
  std::stringstream tmp;
  for(std::vector<uint8_t>::iterator it=payload.begin(); it<min(payload.end(),payload.begin()+20); ++it) {
    tmp << int(*(it)) <<" ";
  }
  return tmp.str();
}


void BasicFrame::dumpPayloadToFile(std::ofstream& fout) {
  std::copy(payload.begin(), payload.end(), std::ostreambuf_iterator<char>(fout));
}


void BasicFrame::reset() {
  Frame::reset();
  codec_id   =AV_CODEC_ID_NONE;
  media_type =AVMEDIA_TYPE_UNKNOWN;
}


bool BasicFrame::isSeekable() {
    fillPars();
    
    /* // nopes
    if (force_seekable) { // this only for filesystem debuggin
        return true;
    }
    */
    
    switch(codec_id) {
        case AV_CODEC_ID_H264:
            if (h264_pars.slice_type == H264SliceType::sps) {
                return true;
            }
            else {
                return false;
            }
            break;
        default:
            return true;
            break;
    }
}



void BasicFrame::reserve(std::size_t n_bytes) {
  this->payload.reserve(n_bytes);
}


void BasicFrame::resize(std::size_t n_bytes) {
  this->payload.resize(n_bytes,0);
}


void BasicFrame::fillPars() {
  if (codec_id==AV_CODEC_ID_H264) {
    fillH264Pars();
  }
}


void BasicFrame::fillH264Pars() {
  if (payload.size()>(nalstamp.size()+1)) { 
    h264_pars.slice_type = ( payload[nalstamp.size()] & 31 );
    h264_pars.frameType =  (( payload[nalstamp.size()] & 96) >> 5);
  }
}


void BasicFrame::fillAVPacket(AVPacket *avpkt) {
  avpkt->data         =payload.data(); // +4; that four is just for debugging..
  avpkt->size         =payload.size(); // -4;
  avpkt->stream_index =stream_index;

  if (codec_id==AV_CODEC_ID_H264 and h264_pars.slice_type==H264SliceType::sps) { // we assume that frames always come in the following sequence: sps, pps, i, etc.
    avpkt->flags=AV_PKT_FLAG_KEY;
  }

  // std::cout << "Frame : useAVPacket : pts =" << pts << std::endl;
//
//  if (mstimestamp>=0) {
//    avpkt->pts=(int64_t)mstimestamp;
//  }
//  else {
//    avpkt->pts=AV_NOPTS_VALUE;
 // }

  // std::cout << "Frame : useAVPacket : final pts =" << pts << std::endl;

  avpkt->dts=AV_NOPTS_VALUE; // let muxer set it automagically 
}


void BasicFrame::copyFromAVPacket(AVPacket *pkt) {
  payload.resize(pkt->size);
  memcpy(payload.data(),pkt->data,pkt->size);
  // TODO: optimally, this would be done only once - in copy-on-write when writing to fifo, at the thread border
//  stream_index=pkt->stream_index;
  // frametype=FrameType::h264; // not here .. avpkt carries no information about the codec
 // mstimestamp=(long int)pkt->pts;
}


void BasicFrame::filterFromAVPacket(AVPacket *pkt, AVCodecContext *codec_ctx, AVBitStreamFilterContext *filter) {
  int out_size;
  uint8_t *out;
  
  av_bitstream_filter_filter(
    filter,
    codec_ctx,
    NULL,
    &out,
    &out_size,
    pkt->data,
    pkt->size,
    pkt->flags & AV_PKT_FLAG_KEY
  );
  
  payload.resize(out_size);
  // std::cout << "BasicFrame: filterFromAVPacket: " << out_size << " " << (long unsigned)(out) << std::endl; 
  memcpy(payload.data(),out,out_size);
  stream_index=pkt->stream_index;
  mstimestamp=(long int)pkt->pts;
}


std::size_t BasicFrame::calcSize() {
    // device_id (std::size_t) stream_index (int) mstimestamp (long int) media_type (AVMediaType) codec_id (AVCodecId) size (std::size_t) payload (char)
    // TODO: should use typedefs more
    return sizeof(IdNumber) + sizeof(stream_index) + sizeof(mstimestamp) + sizeof(media_type) + sizeof(codec_id) + sizeof(std::size_t) + payload.size();
}


#define dump_bytes(var) raw_writer.dump( (const char*)&var, sizeof(var));
#define read_bytes(var) raw_reader.get((char*)&var, sizeof(var));


//// bool BasicFrame::dump(IdNumber device_id, std::fstream &os) {
//bool BasicFrame::dump(IdNumber device_id, RaWriter& raw_writer) {
//    std::size_t len;
//    len=payload.size();
//    
//    dump_bytes(device_id);
//    dump_bytes(stream_index);
//    dump_bytes(mstimestamp);
//    dump_bytes(media_type);
//    dump_bytes(codec_id);
//    dump_bytes(len); // write the number of bytes
//    // std::cout << "BasicFrame: dump: len = " << len << std::endl;
//    raw_writer.dump((const char*)payload.data(), payload.size());
//    // os.write((const char*)payload.data(), payload.size()); // write the bytes themselves
//    // os.flush();
//    return true;
//}


//// IdNumber BasicFrame::read(std::fstream &is) {
//IdNumber BasicFrame::read(RawReader& raw_reader) {
//    std::size_t len;
//    IdNumber device_id;
//    
//    len = 0;
//    device_id = 0;
//    
//    read_bytes(device_id);
//    // is.read((char*)&device_id, sizeof(IdNumber));
//    // std::cout << "BasicFrame : read : device_id =" << device_id << std::endl;
//    
//    if (device_id==0) { // no frame
//        return device_id;
//    }
//    
//    read_bytes(stream_index);
//    if (stream_index < 0) { // corrupt frame
//        valkkafslogger.log(LogLevel::fatal) << "BasicFrame : read : corrupt frame (stream_index)" << std::endl;
//        return 0;
//    }
//    
//    read_bytes(mstimestamp);
//    if (mstimestamp < 0) { // corrupt frame
//        valkkafslogger.log(LogLevel::fatal) << "BasicFrame : read : corrupt frame (mstimestamp)" << std::endl;
//        return 0;
//    }
//    
//    read_bytes(media_type);
//    read_bytes(codec_id);
//    read_bytes(len);  // read the number of bytes
//    
//    try {
//        payload.resize(len);
//    }
//    catch (std::bad_alloc& ba) {
//        valkkafslogger.log(LogLevel::fatal) << "BasicFrame : read : corrupt frame : bad_alloc caught " << ba.what() << std::endl;
//        return 0;
//    }
//        
//    raw_reader.get((char*)payload.data(), len); // read the bytes themselves
//    
//    return device_id;
//}



MuxFrame::MuxFrame() : Frame(), 
    codec_id(AV_CODEC_ID_NONE), 
    media_type(AVMEDIA_TYPE_UNKNOWN),  
    meta_type(MuxMetaType::none)
    {}


MuxFrame::~MuxFrame() {
}


void MuxFrame::print(std::ostream &os) const {
    os << "<MuxFrame: timestamp="<<mstimestamp<<" stream_index="<<stream_index<<" slot="<<n_slot<<" / ";
    os << "fragment size="<<payload.size();
    os << ">";
}


std::string MuxFrame::dumpPayload() {
    std::stringstream tmp;
    for(std::vector<uint8_t>::iterator it=payload.begin(); it<min(payload.end(),payload.begin()+20); ++it) {
        tmp << int(*(it)) <<" ";
    }
    return tmp.str();
}


void MuxFrame::dumpPayloadToFile(std::ofstream& fout) {
    std::copy(payload.begin(), payload.end(), std::ostreambuf_iterator<char>(fout));
}


void MuxFrame::reset() {
    Frame::reset();
    codec_id   =AV_CODEC_ID_NONE;
    media_type =AVMEDIA_TYPE_UNKNOWN;
    meta_type  =MuxMetaType::none;
}


void MuxFrame::reserve(std::size_t n_bytes) {
    this->payload.reserve(n_bytes);
    this->meta_blob.reserve(METADATA_MAX_SIZE);
}


void MuxFrame::resize(std::size_t n_bytes) {
    this->payload.resize(n_bytes, 0);
}


SetupFrame::SetupFrame() : Frame(), sub_type(SetupFrameType::stream_init), media_type(AVMEDIA_TYPE_UNKNOWN), codec_id(AV_CODEC_ID_NONE), stream_state(AbstractFileState::none) {
  // reset(); // done at Frame ctor
}
  
  
SetupFrame::~SetupFrame() {
}

//frame_essentials(FrameClass::setup, SetupFrame);

void SetupFrame::print(std::ostream &os) const {
    os << "<SetupFrame: timestamp="<<mstimestamp<<" stream_index="<<stream_index<<" slot="<<n_slot<<" / ";  //<<std::endl;
    if (sub_type == SetupFrameType::stream_init) {
        os << "media_type=" << int(media_type) << " codec_id=" << int(codec_id);
    }
    else if (sub_type == SetupFrameType::stream_state) {
        os << "stream_state=" << int(stream_state);
    }
    os << ">";
}


void SetupFrame::reset() {
  Frame::reset();
  media_type=AVMEDIA_TYPE_UNKNOWN;
  codec_id  =AV_CODEC_ID_NONE;
}



AVMediaFrame::AVMediaFrame() : media_type(AVMEDIA_TYPE_UNKNOWN), codec_id(AV_CODEC_ID_NONE) { // , mediatype(MediaType::none) {
  av_frame =av_frame_alloc();
}


AVMediaFrame::~AVMediaFrame() {
  av_frame_free(&av_frame);
  av_free(av_frame); // needs this as well?
}


//frame_essentials(FrameClass::avmedia, AVMediaFrame);


void AVMediaFrame::print(std::ostream& os) const {
  os << "<AVMediaFrame: timestamp="<<mstimestamp<<" stream_index="<<stream_index<<" slot="<<n_slot<<" / ";
  os << "media_type=" << media_type << std::endl;
  os << ">";
}


std::string AVMediaFrame::dumpPayload() {
  std::stringstream tmp;
  return tmp.str();
}


void AVMediaFrame::reset() {
  Frame::reset();
  media_type   =AVMEDIA_TYPE_UNKNOWN; 
  codec_id     =AV_CODEC_ID_NONE;
  // TODO: reset AVFrame ?
}








/*
AVAudioFrame::AVAudioFrame() : AVMediaFrame(), av_sample_fmt(AV_SAMPLE_FMT_NONE) {
  mediatype=MediaType::audio;
}


AVAudioFrame::~AVAudioFrame() {
}


//frame_essentials(FrameClass::avaudio, AVAudioFrame);


std::string AVAudioFrame::dumpPayload() {
  return std::string("");
}


void AVAudioFrame::print(std::ostream& os) const {
  os << "<AVAudioFrame: timestamp="<<mstimestamp<<" stream_index="<<stream_index<<" slot="<<n_slot<<" / ";
  os << ">";
}


bool AVAudioFrame::isOK() {
  return true;
}


void AVAudioFrame::getParametersDecoder(const AVCodecContext *ctx) {
  av_media_type   = ctx->codec_type;
  av_sample_fmt   = ctx->sample_fmt;
  // TODO: from FFMpeg codecs to valkka codecs
  updateParameters();
}
*/





MarkerFrame::MarkerFrame() : Frame(), fs_start(false), fs_end(false), tm_start(false), tm_end(false) {
}

MarkerFrame::~MarkerFrame() {
}


void MarkerFrame::print(std::ostream& os) const {
  os << "<MarkerFrame: timestamp="<<mstimestamp<<" stream_index="<<stream_index<<" slot="<<n_slot<<" / ";
  if (fs_start) {
    os << "FS_START ";
  }
  if (fs_end) {
      os << "FS_END ";
  }
  if (tm_start) {
      os << "TM_START ";
  }
  if (tm_end) {
      os << "TM_END ";
  }
  os << ">";
}


void MarkerFrame::reset() {
    fs_start=false;
    fs_end=false;
    tm_start=false;
    tm_end=false;
}
    
    
    
    
    
    
    
    
