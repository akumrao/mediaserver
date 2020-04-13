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

#include "socketio/packet.h"
#include "base/logger.h"
#include "base/util.h"

#include "json/json.h"

using std::endl;


namespace base {
    namespace sockio {


        packet::packet(string const& nsp, json const& msg, int pack_id, bool isAck) :
        _frame(frame_message),
        _type((isAck ? type_ack : type_event) | type_undetermined),
        _nsp(nsp),
        _pack_id(pack_id),
        _message(msg),
        _pending_buffers(0) {
            assert((!isAck
                    || (isAck && pack_id >= 0)));
        }

        packet::packet(type type, string const& nsp, json const& msg) :
        _frame(frame_message),
        _type(type),
        _nsp(nsp),
        _pack_id(-1),
        _message(msg),
        _pending_buffers(0) {

        }

        packet::packet(packet::frame_type frame) :
        _frame(frame),
        _type(type_undetermined),
        _pack_id(-1),
        _pending_buffers(0) {

        }

        packet::packet() :
        _type(type_undetermined),
        _pack_id(-1),
        _pending_buffers(0) {

        }

        bool packet::is_binary_message(string const& payload_ptr) {
            return payload_ptr.size() > 0 && payload_ptr[0] == frame_message;
        }

        bool packet::is_text_message(string const& payload_ptr) {
            return payload_ptr.size() > 0 && payload_ptr[0] == (frame_message + '0');
        }

        bool packet::is_message(string const& payload_ptr) {
            return is_binary_message(payload_ptr) || is_text_message(payload_ptr);
        }

        bool packet::parse_buffer(const string &buf_payload) {
            if (_pending_buffers > 0) {
                assert(is_binary_message(buf_payload)); //this is ensured by outside.
                _buffers.push_back(std::make_shared<string>(buf_payload.data() + 1, buf_payload.size() - 1));
                _pending_buffers--;
                if (_pending_buffers == 0) {

                    //  Document doc;
                    //  doc.Parse<0>(_buffers.front()->data());
                    //   _buffers.erase(_buffers.begin());
                    //   _message = from_json(doc, _buffers);
                    // _buffers.clear();
                    return false;
                }
                return true;
            }
            return false;
        }

        bool packet::parse(const string& payload_ptr) {
            assert(!is_binary_message(payload_ptr)); //this is ensured by outside
            _frame = (packet::frame_type) (payload_ptr[0] - '0');
            _message.clear();
            _pack_id = -1;
            _buffers.clear();
            _pending_buffers = 0;
            size_t pos = 1;
            if (_frame == frame_message) {
                _type = (packet::type)(payload_ptr[pos] - '0');
                if (_type < type_min || _type > type_max) {
                    return false;
                }
                pos++;
                if (_type == type_binary_event || _type == type_binary_ack) {
                    size_t score_pos = payload_ptr.find('-'); //arvind
                    //     _pending_buffers = boost::lexical_cast<unsigned>(payload_ptr.substr(pos,score_pos - pos));
                    pos = score_pos + 1;
                }
            }

            size_t nsp_json_pos = payload_ptr.find_first_of("{[\"/", pos, 4);
            if (nsp_json_pos == string::npos)//no namespace and no message,the end.
            {
                _nsp = "/";
                return false;
            }
            size_t json_pos = nsp_json_pos;
            if (payload_ptr[nsp_json_pos] == '/')//nsp_json_pos is start of nsp
            {
                size_t comma_pos = payload_ptr.find_first_of(","); //end of nsp
                if (comma_pos == string::npos)//packet end with nsp
                {
                    _nsp = payload_ptr.substr(nsp_json_pos);
                    return false;
                } else//we have a message, maybe the message have an id.
                {
                    _nsp = payload_ptr.substr(nsp_json_pos, comma_pos - nsp_json_pos);
                    pos = comma_pos + 1; //start of the message
                    json_pos = payload_ptr.find_first_of("\"[{", pos, 3); //start of the json part of message
                    if (json_pos == string::npos) {
                        //no message,the end
                        //assume if there's no message, there's no message id.
                        return false;
                    }
                }
            } else {
                _nsp = "/";
            }

            if (pos < json_pos)//we've got pack id.
            {//arvind
                _pack_id = std::stoi(payload_ptr.substr(pos, json_pos - pos));
               // LTrace("packet id ", _pack_id)
            }
            if (_frame == frame_message && (_type == type_binary_event || _type == type_binary_ack)) {
                //parse later when all buffers are arrived.
                _buffers.push_back(make_shared<string>(payload_ptr.data() + json_pos, payload_ptr.length() - json_pos));
                return true;
            } else {
                //arvind
                //  Document doc;
                //  doc.Parse<0>(payload_ptr.data()+json_pos);
                _message = json::parse(payload_ptr.data() + json_pos);
                return false;
            }

        }

        bool packet::accept(string& payload_ptr, vector<shared_ptr<const string> >&buffers) {
            char frame_char = _frame + '0';
            payload_ptr.append(&frame_char, 1);
            if (_frame != frame_message) {
                return false;
            }
            bool hasMessage = false; //arvind
            ///* Document doc;
            if (_message.size()) {
                // accept_message(*_message, doc, doc, buffers);
                hasMessage = true;
            }
            bool hasBinary = buffers.size() > 0;
            _type = _type & (~type_undetermined);
            if (_type == type_event) {
                _type = hasBinary ? type_binary_event : type_event;
            } else if (_type == type_ack) {
                _type = hasBinary ? type_binary_ack : type_ack;
            }
            ostringstream ss;
            ss.precision(8);
            ss << _type;
            if (hasBinary) {
                ss << buffers.size() << "-";
            }
            if (_nsp.size() > 0 && _nsp != "/") {
                ss << _nsp;
                if (hasMessage || _pack_id >= 0) {
                    ss << ",";
                }
            }

            if (_pack_id >= 0) {
                ss << _pack_id;
            }

            payload_ptr.append(ss.str());
            if (hasMessage) {//arvind
                //StringBuffer buffer;
                // Writer<StringBuffer> writer(buffer);
                //  doc.Accept(writer);
                // payload_ptr.append(buffer.GetString(),buffer.GetSize());
                payload_ptr.append(cnfg::stringify(_message));
            }
            //LTrace(payload_ptr);
            return hasBinary;
        }

        packet::frame_type packet::get_frame() const {
            return _frame;
        }

        packet::type packet::get_type() const {
            assert((_type & type_undetermined) == 0);
            return (type) _type;
        }

        string const& packet::get_nsp() const {
            return _nsp;
        }

        json const& packet::get_message() const {
            return _message;
        }

        unsigned packet::get_pack_id() const {
            return _pack_id;
        }

        void packet_manager::set_decode_callback(function<void (packet const&) > const& decode_callback) {
            m_decode_callback = decode_callback;
        }

        void packet_manager::set_encode_callback(function<void (bool, shared_ptr<const string> const&) > const& encode_callback) {
            m_encode_callback = encode_callback;
        }

        void packet_manager::reset() {
            m_partial_packet.reset();
        }

        void packet_manager::encode(packet& pack, encode_callback_function const& override_encode_callback) const {
            shared_ptr<string> ptr = make_shared<string>();
            vector<shared_ptr<const string> > buffers;
            const encode_callback_function *cb_ptr = &m_encode_callback;
            if (override_encode_callback) {
                cb_ptr = &override_encode_callback;
            }
            if (pack.accept(*ptr, buffers)) {
                if ((*cb_ptr)) {
                    (*cb_ptr)(false, ptr);
                }
                for (auto it = buffers.begin(); it != buffers.end(); ++it) {
                    if ((*cb_ptr)) {
                        (*cb_ptr)(true, *it);
                    }
                }
            } else {
                if ((*cb_ptr)) {
                    (*cb_ptr)(false, ptr);
                }
            }
        }

        void packet_manager::put_payload(string const& payload) {
            //STrace << "payload " << payload ;

            unique_ptr<packet> p;
            do {
                if (packet::is_text_message(payload)) {
                    p.reset(new packet());
                    if (p->parse(payload)) {
                        m_partial_packet = std::move(p);
                    } else {
                        break;
                    }
                } else if (packet::is_binary_message(payload)) {
                    if (m_partial_packet) {
                        if (!m_partial_packet->parse_buffer(payload)) {
                            p = std::move(m_partial_packet);
                            break;
                        }
                    }
                } else {
                    p.reset(new packet());
                    p->parse(payload);
                    break;
                }
                return;
            } while (0);

            if (m_decode_callback) {
                m_decode_callback(*p);
            }
        }


    }
} // namespace scy::sockio
