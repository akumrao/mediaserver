

#ifndef WebRTC_MultiplexMediaCapturer_H
#define WebRTC_MultiplexMediaCapturer_H


#include "base/base.h"

//#ifdef HAVE_FFMPEG


#if MP4File
#include "ff/ff.h"
#include "ff/mediacapture.h"
#endif


#include "webrtc/audiopacketmodule.h"
#include "webrtc/videopacketsource.h"

#include "api/peer_connection_interface.h"
#include "webrtc/peer.h"


#include "webrtc/localAudioSouce.h"


namespace base
{
namespace web_rtc
{


// class VideoObserver : public rtc::VideoSinkInterface<webrtc::VideoFrame> {
//  public:
//   VideoObserver() {}
//   ~VideoObserver() {}
//   //oid SetVideoCallback(I420FRAMEREADY_CALLBACK callback);
//
//  protected:
//   // VideoSinkInterface implementation
//   void OnFrame(const webrtc::VideoFrame& frame)
//   {
//       int x = 1;
//   }
//
//  private:
//
//   //std::mutex mutex;
// };


class MultiplexMediaCapturer
{
public:
    MultiplexMediaCapturer();
    ~MultiplexMediaCapturer();

    void openFile(const std::string &dir, const std::string &file, const std::string &type, bool loop = true);

    std::string random_string();

    void addMediaTracks(
        webrtc::PeerConnectionFactoryInterface *factory,
        webrtc::PeerConnectionInterface *conn,
        web_rtc::Peer *peer,
        st_track &trackInfo,
        std::string &room);


    void getPeerids(st_track &trackInfo, std::set<std::string> &sPeerIds);

    // void start(std::string & cam );
    void stop(std::string &cam, std::set<std::string> &tmp);

    void remove(web_rtc::Peer *conn, std::string &trackId);

    void remove(web_rtc::Peer *conn);

    void oncommand(web_rtc::Peer *conn, std::string &trackId, std::string &cmd, int arg1, int arg2);

    void mute(web_rtc::Peer *conn, std::string &trackId, std::string &cmd, bool act);

    void onAnswer(web_rtc::Peer *conn, en_EncType &encType, std::string &vtrackId, std::string &atrackId);

    rtc::scoped_refptr<AudioPacketModule> getAudioModule();
    
    void startRecording( std::string &cam );
    void stopRecording( std::string &cam );
    
    // VideoPacketSource* createVideoSource();

protected:
    //    PacketStream _stream;
#if MP4File
    ff::MediaCapture::Ptr _videoCapture;
#endif

    rtc::scoped_refptr<AudioPacketModule> _audioModule;


public:
    std::map<std::string, rtc::scoped_refptr<LocalAudioSource>> mapAudioCapturer;

    std::map<std::string, rtc::scoped_refptr<VideoPacketSource>> mapVideoSource;

protected:
    std::map<std::string, rtc::scoped_refptr<webrtc::AudioTrackInterface>> mapaudio_track;

    std::map<std::string, rtc::scoped_refptr<webrtc::VideoTrackInterface>> mapvideo_track;

    // std::unique_ptr<VideoObserver> local_video_observer_;

    int PlayerID;


private:
    std::string fileName;
    std::mutex mutexCap;
    
    
    struct stLiveThread
    {

        //int refCount{0};

        LiveThread  *liveThread{nullptr}; 
        LiveConnectionContext *ctx{nullptr};
        int slot{1}; 

        stLiveThread( VideoPacketSource *src, std::string& cam, std::string& trackid, bool recording );
        ~stLiveThread();
        
        void addVideoPacketSource(std::string& trackid, VideoPacketSource *src );
        int removeVideoPacketSource(std::string & trackid );
   
        
        void myAddPeerRef(  std::string peerid)
        {

           mtPeerId.lock();
           setPeerId.insert(peerid);
           mtPeerId.unlock();

        }

        rtc::RefCountReleaseStatus myReleasePeer(  std::string peerid )  
        {

            std::set< std::string> ::iterator itr;
            int count =1;

            mtPeerId.lock();
            itr = setPeerId.find(peerid);

            if( itr != setPeerId.end())
            {
                setPeerId.erase(itr);
            }

            count = setPeerId.size();
            mtPeerId.unlock();



            if (count == 0) {

              return rtc::RefCountReleaseStatus::kDroppedLastRef;
            }
            return rtc::RefCountReleaseStatus::kOtherRefsRemained;
        }




         void resetPeer(  std::set< std::string> & peeerids )  {

            std::set< std::string> tmp;
            mtPeerId.lock();

            peeerids =    setPeerId;

            setPeerId.clear();

            mtPeerId.unlock();

        }

        std::set< std::string> setPeerId;
        std::mutex mtPeerId;    

    };
    
 
    std::map< std::string,  stLiveThread* > mapLiveThread ;
    
    
};


}  // namespace web_rtc
}  // namespace base


//#endif // HAVE_FFMPEG
#endif
