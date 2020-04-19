/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */


/*
  The following events are reserved and should not be used as event names by your application:
 error
connect
disconnect
disconnecting
newListener
removeListener
ping
pong
 
 
 */


#ifndef SocketIO_Packet_H
#define SocketIO_Packet_H

#include "json/json.h"

using namespace std;

namespace base {
namespace sockio {

class packet
    {
    public:
        enum frame_type
        {
            frame_open = 0,
            frame_close = 1,
            frame_ping = 2,
            frame_pong = 3,
            frame_message = 4,
            frame_upgrade = 5,
            frame_noop = 6
        };
        
        enum type
        {
            type_min = 0,
            type_connect = 0,
            type_disconnect = 1,
            type_event = 2,
            type_ack = 3,
            type_error = 4,
            type_binary_event = 5,
            type_binary_ack = 6,
            type_max = 6,
            type_undetermined = 0x10 //undetermined mask bit
        };
    private:
        frame_type _frame;
        int _type;
        string _nsp;
        int _pack_id;
        json _message;
        unsigned _pending_buffers;
        vector<shared_ptr<const string> > _buffers;
    public:
        packet(string const& nsp,json const& msg,int pack_id = -1,bool isAck = false);//message type constructor.
        
        packet(frame_type frame);
        
        packet(type type,string const& nsp= string(),json const& msg = json());//other message types constructor.
        //empty constructor for parse.
        packet();
        
        frame_type get_frame() const;
        
        type get_type() const;
        
        bool parse(string const& payload_ptr);//return true if need to parse buffer.
        
        bool parse_buffer(string const& buf_payload);
        
        bool accept(string& payload_ptr, vector<shared_ptr<const string> >&buffers); //return true if has binary buffers.
        
        string const& get_nsp() const;
        
        json const& get_message() const;
        
        unsigned get_pack_id() const;
        
        static bool is_message(string const& payload_ptr);
        static bool is_text_message(string const& payload_ptr);
        static bool is_binary_message(string const& payload_ptr);
    };
    
 class packet_manager
    {
    public:
        typedef function<void (bool,shared_ptr<const string> const&)> encode_callback_function;
        typedef  function<void (packet const&)> decode_callback_function;
        
        void set_decode_callback(decode_callback_function const& decode_callback);

        void set_encode_callback(encode_callback_function const& encode_callback);
        
        void encode(packet& pack,encode_callback_function const& override_encode_callback = encode_callback_function()) const;
        
        void put_payload(string const& payload);
        
        void reset();
        
    private:
        decode_callback_function m_decode_callback;
        
        encode_callback_function m_encode_callback;
        
        std::unique_ptr<packet> m_partial_packet;
    };


} } // namespace base::sockio


#endif //  SocketIO_Packet_H

