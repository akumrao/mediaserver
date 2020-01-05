
#include "http/form.h"
//#include "crypto/crypto.h"
#include "base/filesystem.h"

#include "http/packetizers.h"
#include "http/url.h"
#include <stdexcept>
#include "base/platform.h"

namespace base {
    namespace net {
        //
        // Form Writer
        //

        const char* FormWriter::ENCODING_URL = "application/x-www-form-urlencoded";
        const char* FormWriter::ENCODING_MULTIPART_FORM = "multipart/form-data";
        const char* FormWriter::ENCODING_MULTIPART_RELATED = "multipart/related";
        const char* FormWriter::TEXT_PLAIN = "text/plain";
        const char* FormWriter::APPLICATION_ZIP = "application/zip";


        FormWriter* FormWriter::create(ClientConnecton* stream, const std::string& encoding) {
            auto wr = new FormWriter(stream, encoding);
            //  stream.Outgoing.attachSource(wr, true, true);
            if (stream->_request.isChunkedTransferEncoding()) {
                assert(encoding != FormWriter::ENCODING_URL);
                assert(stream->_request.getVersion() != Message::HTTP_1_0);

            }
            return wr;

        }

        FormWriter::FormWriter(ClientConnecton* connection, const std::string& encoding)
        :
        _stream(connection)
        // , _runner(runner)
        , _encoding(encoding)
        , _filesLength(0)
        , _writeState(0)
        , _initial(true)
        , _complete(false) {

        }

        FormWriter::~FormWriter() {
           clearParts();
        }
        void FormWriter::clearParts() {
            for (auto it = _parts.begin(); it != _parts.end(); ++it)
                delete it->part;
            _parts.clear();
        }

        void FormWriter::addPart(const std::string& name, FormPart* part) {
            assert(part);
            assert(_encoding != ENCODING_URL);

            Part p;
            p.part = part;
            p.name = name;
            _parts.push_back(p);
            _filesLength += part->length();
        }

        void FormWriter::header() {
            LTrace("Prepare header")
           // _stream->OutgoingProgress.start();
            Request& request = _stream->_request;
            if (request.getMethod() == Method::Post ||
                    request.getMethod() == Method::Put) {
                if (_encoding == ENCODING_URL) {
                    request.setContentType(_encoding);
                    request.setChunkedTransferEncoding(false);
                    std::ostringstream ostr;
                    writeUrl(ostr);
                    assert(ostr.tellp() > 0);
                    request.setContentLength(ostr.tellp());
                } else if(  _encoding == ENCODING_MULTIPART_FORM || _encoding == ENCODING_MULTIPART_RELATED   )
                {
                    if (_boundary.empty())
                        _boundary = createBoundary();
                    std::string ct(_encoding);
                    ct.append("; boundary=\"");
                    ct.append(_boundary);
                    ct.append("\"");
                    request.setContentType(ct);

                    // Set the total file size for upload progress updates.
                    // This is not the HTTP content length as it does not
                    // factor chunk headers.
                    if (!_parts.empty()) {
                        assert(_filesLength);
                        _stream->OutgoingProgress.total = _filesLength;
                    }

                    // Set Content-Length for non-chunked transfers
                    if (!request.isChunkedTransferEncoding() &&
                            request.getVersion() != Message::HTTP_1_0)
                        request.setContentLength(calculateMultipartContentLength());
                }
                else
                {

                    request.setContentType(_encoding);
                   // assert(_filesLength);
                    _stream->OutgoingProgress.total = _filesLength;

                }
                
                if (request.getVersion() == Message::HTTP_1_0) {
                    request.setKeepAlive(false);
                    request.setChunkedTransferEncoding(false);
                    
                }
                
            } else {
                std::string uri = request.getURI();
                std::ostringstream ostr;
                writeUrl(ostr);
                uri.append("?");
                uri.append(ostr.str());
                request.setURI(uri);
            }
        }

        uint64_t FormWriter::calculateMultipartContentLength() {
            std::ostringstream ostr;
            for (NVCollection::ConstIterator it = begin(); it != end(); ++it) {
                NVCollection header;
                if (_encoding == ENCODING_MULTIPART_FORM) {
                    std::string disp("form-data; name=\"");
                    disp.append(it->first);
                    disp.append("\"");
                    header.set("Content-Disposition", disp);
                }
                writePartHeader(header, ostr);
                ostr << it->second;
            }
            for (PartQueue::const_iterator pit = _parts.begin(); pit != _parts.end();
                    ++pit) {
                NVCollection header(pit->part->headers());
                if (_encoding == ENCODING_MULTIPART_FORM) {
                    std::string disp("form-data; name=\"");
                    disp.append(pit->name);
                    disp.append("\"");
                    auto filePart = dynamic_cast<FilePart*> (pit->part);
                    if (filePart) {
                        std::string filename = filePart->filename();
                        if (!filename.empty()) {
                            disp.append("; filename=\"");
                            disp.append(filename);
                            disp.append("\"");
                        }
                    }
                    header.set("Content-Disposition", disp);
                }
                header.set("Content-Type", pit->part->contentType());
                writePartHeader(header, ostr);
                pit->part->write(ostr);
                pit->part->reset(); // reset part state
            }
            writeEnd(ostr);
            return ostr.tellp();
        }

        void FormWriter::emit(const char *data, size_t len) {
            LTrace(len);
            _stream->send(data, len);
        }

        void FormWriter::emit(const std::string &data) {
            LTrace(data.length());
             //LTrace(data);
            _stream->send(data);
        }

        
        void FormWriter::run() {

            LTrace("run")
            _stream->OutgoingProgress.start();
            
            while (!complete()) {
            //    condWait.wait();
                if( stopped())
                {
                    LTrace("Set state 2 to stop")
                     clearParts();
                    _writeState =2; // signal server to stop with sending 0/r/n
                }
                
                try {
                    // assert(!complete());
                    if (encoding() == ENCODING_URL) {
                        std::ostringstream ostr;
                        writeUrl(ostr);
                        LTrace("Writing URL: ", ostr.str())
                        emit(ostr.str());
                        _complete = true;
                    } else if(  _encoding == ENCODING_MULTIPART_FORM || _encoding == ENCODING_MULTIPART_RELATED  ) {
                      
                        writeMultipartChunk();
                    }
                    else
                    {
                        writeChunked();
                    }
                            

                    if (complete()) {
                        break;
                        //stop();
                    }
                } catch (std::exception& exc) {
                    LTrace("Error: ", exc.what())
                            // assert(0);
                            //  throw exc;
                            //#ifdef _DEBUG
                            //    throw exc;
                            //#endif
                }
                
               base::sleep(7);
                
            }

            if(_stream->fnFormClose)
            _stream->fnFormClose(_stream);

            LTrace("runover")
            isrunning_ = false;

        }

        void FormWriter::writeMultipartChunk() {
           // LTrace("Writing chunk: ", _writeState)

            switch (_writeState) {

                    // Send form values
                case 0:
                    for (NVCollection::ConstIterator it = begin(); it != end(); ++it) {
                        std::ostringstream ostr;
                        NVCollection header;
                        if (_encoding == ENCODING_MULTIPART_FORM) {
                            std::string disp("form-data; name=\"");
                            disp.append(it->first);
                            disp.append("\"");
                            header.set("Content-Disposition", disp);
                        }
                        writePartHeader(header, ostr);
                        ostr << it->second;
                        emit(ostr.str());
                    }
                    _writeState++;
                    break;

                    // Send file parts
                case 1:
                    if (!_parts.empty()) {
                        auto& p = _parts.front();

                        if (p.part->initialWrite()) {
                            std::ostringstream ostr;
                            NVCollection header(p.part->headers());
                            if (_encoding == ENCODING_MULTIPART_FORM) {
                                std::string disp("form-data; name=\"");
                                disp.append(p.name);
                                disp.append("\"");
                                auto filePart = dynamic_cast<FilePart*> (p.part);
                                if (filePart) {
                                    std::string filename = filePart->filename();
                                    if (!filename.empty()) {
                                        disp.append("; filename=\"");
                                        disp.append(filename);
                                        disp.append("\"");
                                    }
                                }
                                header.set("Content-Disposition", disp);
                            }
                            header.set("Content-Type", p.part->contentType());
                            writePartHeader(header, ostr);
                            emit(ostr.str());
                        }
                        if (p.part->write(*this)) {
                            return; // return after writing a chunk
                        } else {
                            LTrace("Part complete: ", p.name)
                                    delete p.part;
                            _parts.pop_front();
                        }
                    }
                    if (_parts.empty())
                        _writeState++;
                    break;

                    // Send final packet
                case 2:
                {
                    std::ostringstream ostr;
                    writeEnd(ostr);
                    emit(ostr.str());

                    // HACK: Write chunked end code here.
                    // The ChunkedAdapter should really be doing this.

                    if (_stream->_request.isChunkedTransferEncoding()) {
                        emit("0\r\n\r\n", 5);
                    }

                    LTrace("Request complete")
                    _complete = true;
                    _writeState = -1; // raise error if called again
                }
                    break;

                    // Invalid state
                default:
                    LError("Invalid write state: ", _writeState)
                    assert(0 && "invalid write state");
                    break;
            }
        }
        
        
          void FormWriter::writeChunked() {
          //  LTrace("Writing chunk: ", _writeState)

            switch (_writeState) {
                case 0:
                    if (!_parts.empty()) {
                        auto& p = _parts.front();

                   
                        if (p.part->writeChunk(*this)) {
                            return; // return after writing a chunk
                        } else {
                            LTrace("Part complete: ", p.name)
                               delete p.part;
                            _parts.pop_front();
                        }
                    }
                    if (_parts.empty())
                        _writeState = 2;
                    break;

                    // Send final packet
                case 2:
                {
                    emit("0\r\n\r\n", 5);
                    LTrace("Request complete")
                    _complete = true;
                    _writeState = -1; // raise error if called again
                }
                    break;

                    // Invalid state
                default:
                    LError("Invalid write state: ", _writeState)
                    assert(0 && "invalid write state");
                    break;
            }
        }

        void FormWriter::writeUrl(std::ostream& ostr) {
            for (NVCollection::ConstIterator it = begin(); it != end(); ++it) {
                if (it != begin())
                    ostr << "&";
                ostr << URL::encode(it->first) << "=" << URL::encode(it->second);
            }
        }

        void FormWriter::writePartHeader(const NVCollection& header, std::ostream& ostr) {
            if (_initial)
                _initial = false;
            else
                ostr << "\r\n";
            ostr << "--" << _boundary << "\r\n";

            NVCollection::ConstIterator it = header.begin();
            while (it != header.end()) {
                ostr << it->first << ": " << it->second << "\r\n";
                ++it;
            }
            ostr << "\r\n";
        }

        void FormWriter::writeEnd(std::ostream& ostr) {
            ostr << "\r\n--" << _boundary << "--\r\n";
        }

        void FormWriter::updateProgress(int nread) {
            _stream->OutgoingProgress.update(nread, _stream);
        }

        std::string FormWriter::createBoundary() {
            return "boundary_" + util::randomString(8);
        }

        void FormWriter::setEncoding(const std::string& encoding) {
            _encoding = encoding;
        }

        void FormWriter::setBoundary(const std::string& boundary) {
            _boundary = boundary;
        }

        const std::string& FormWriter::encoding() const {
            return _encoding;
        }

        const std::string& FormWriter::boundary() const {
            return _boundary;
        }

        ClientConnecton* FormWriter::connection() {
            return _stream;
        }

        bool FormWriter::complete() const {
            return _complete;
        }


        //
        // File Part Source
        //

        FormPart::FormPart(const std::string& contentType)
        : _contentType(contentType)
        , _initialWrite(true) {
        }

        FormPart::~FormPart() {
        }

        NVCollection& FormPart::headers() {
            return _headers;
        }

        const std::string& FormPart::contentType() const {
            return _contentType;
        }

        bool FormPart::initialWrite() const {
            return _initialWrite;
        }

        void FormPart::reset() {
            _initialWrite = true;
        }


        //
        // File Part Source
        //

        FilePart::FilePart(const std::string& path)
        : _path(path)
        , _filename(fs::filename(path))
        , _fileSize(0) {
            open();
        }

        FilePart::FilePart(const std::string& path, const std::string& contentType)
        : FormPart(contentType)
        , _path(path)
        , _filename(fs::filename(path))
        , _fileSize(0) {
            open();
        }

        FilePart::FilePart(const std::string& path, const std::string& filename,
                const std::string& contentType)
        : FormPart(contentType)
        , _path(path)
        , _filename(filename)
        , _fileSize(0) {
            open();
        }

        FilePart::~FilePart() {
        }

        void FilePart::open() {
            LTrace("Open: ", _path)

            _istr.open(_path.c_str(), std::ios::in | std::ios::binary);
            if (!_istr.is_open())
                throw std::runtime_error("Cannot open file: " + _path);

            // Get file size
            _istr.seekg(0, std::ios::end);
            _fileSize = _istr.tellg();
            _istr.seekg(0, std::ios::beg);
        }

        void FilePart::reset() {
            FormPart::reset();
            _istr.clear();
            _istr.seekg(0, std::ios::beg);
        }

        bool FilePart::writeChunk(FormWriter& writer) {
          //  LTrace("Write chunk")
           // assert(!writer.stopped());
            _initialWrite = false;

            std::ostringstream ost;
            char buffer[FILE_CHUNK_SIZE];
            if (_istr.read(buffer, FILE_CHUNK_SIZE) && !writer.stopped()) {
                
                ost << std::hex << _istr.gcount();
                ost << "\r\n";
                ost.write( buffer, (size_t) _istr.gcount());
                ost << "\r\n";
                writer.emit(ost.str());
                writer.updateProgress((int) _istr.gcount());
                return true;
            }

            if (_istr.eof() && !writer.stopped()) {
                
                if(_istr.gcount() > 0)
                {
                   ost << std::hex << _istr.gcount();
                   ost << "\r\n";
                   ost.write( buffer, (size_t) _istr.gcount());
                   ost << "\r\n";
                   writer.emit(ost.str());
                   writer.updateProgress((int) _istr.gcount());
                }
                 return false;
            } else if (_istr.bad())
                throw std::runtime_error("Cannot read multipart source file: " +
                    _filename);
        }

        bool FilePart::write(FormWriter& writer) {
            LTrace("Write")
            _initialWrite = false;

            char buffer[FILE_CHUNK_SIZE];
            if (_istr.read(buffer, FILE_CHUNK_SIZE) && !writer.stopped()) {
                writer.emit(buffer, (size_t) _istr.gcount());
                writer.updateProgress((int) _istr.gcount());
               return true;
            }

            if (_istr.eof()) {
                if (_istr.gcount() > 0 && !writer.stopped()) {
                    writer.emit(buffer, (size_t) _istr.gcount());
                    writer.updateProgress((int) _istr.gcount());
                }
                return false;
            } else if (_istr.bad())
                throw std::runtime_error("Cannot read multipart source file: " +
                    _filename);
        }

        void FilePart::write(std::ostream& ostr) {
            LTrace("Write")
            _initialWrite = false;

            char buffer[FILE_CHUNK_SIZE];
            while (_istr.read(buffer, FILE_CHUNK_SIZE))
                ostr.write(buffer, (size_t) _istr.gcount());

            if (_istr.eof()) {
                if (_istr.gcount() > 0)
                    ostr.write(buffer, (size_t) _istr.gcount());
            } else if (_istr.bad())
                throw std::runtime_error("Cannot read multipart source file: " +
                    _filename);
        }

        const std::string& FilePart::filename() const {
            return _filename;
        }

        std::ifstream& FilePart::stream() {
            return _istr;
        }

        uint64_t FilePart::length() const {
            return _fileSize;
        }


        //
        // String Part Source
        //

        StringPart::StringPart(const std::string& data)
        : FormPart("application/octet-stream")
        , curlen(0), _data(data) {
        }

        StringPart::StringPart(const std::string& data, const std::string& contentType)
        : FormPart(contentType)
        ,curlen(0), _data(data) {
        }

        StringPart::~StringPart() {
        }

        bool StringPart::writeChunk(FormWriter& writer) {
            LTrace("Write chunk")
            _initialWrite = false;

             std::ostringstream ost;

          //  writer.emit(_data.c_str(), _data.length());
           // writer.updateProgress((int) _data.length());

          //  return false;
             
            ost << std::hex << FILE_CHUNK_SIZE;
            ost << "\r\n";
            ost.write( _data.c_str(), (size_t) FILE_CHUNK_SIZE);
            ost << "\r\n";
            writer.emit(ost.str());
            writer.updateProgress((int) FILE_CHUNK_SIZE);
            ++curlen;
            
            if(curlen * FILE_CHUNK_SIZE  >= _data.length() )
            {
                return false;
            }
            
            return true;

        }

        bool StringPart::write(FormWriter& writer) {
            LTrace("Write")
            _initialWrite = false;

            writer.emit(_data.c_str(), _data.length());
            writer.updateProgress((int) _data.length());
            return false;
        }

        void StringPart::write(std::ostream& ostr) {
            LTrace("Write")
            _initialWrite = false;

            ostr.write(_data.c_str(), _data.length());
        }

        uint64_t StringPart::length() const {
            return _data.length();
        }


    } // namespace http
} // namespace base

