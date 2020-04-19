/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */


#ifndef HTTP_Form_H
#define HTTP_Form_H


#include "http/http.h"
#include "base/collection.h"

#include "net/netInterface.h"
#include "http/packetizers.h"
#include "http/client.h"


namespace base {
namespace net {


//class  Request;
class  FormPart;

const int FILE_CHUNK_SIZE = 10*1024; // 32384;65536
//
// HTML Form Writer
//

/// FormWriter is a HTTP client connection adapter for writing HTML forms.
///
/// This class runs in its own thread so as not to block the event loop
/// while uploading big files. Class members are not synchronized hence
/// they should not be accessed while the form is sending, not that there
/// would be any reason to do so.
class  FormWriter :  public NVCollection, public Thread
//    public PacketSource,
//    public basic::Startable
{
public:
    /// Creates the FormWriter that uses the given connection and
    /// encoding type.
    ///
    /// Encoding must be either "application/x-www-form-urlencoded"
    /// (which is the default) or "multipart/form-data".
    static FormWriter* create(ClientConnecton* conn, const std::string& encoding = FormWriter::ENCODING_URL);

    /// Destroys the FormWriter.
    virtual ~FormWriter();
    
    void clearParts();

    /// Adds an part/attachment (file upload) to the form.
    ///
    /// The form takes ownership of the FilePart and deletes it when it
    /// is no longer needed. The part will only be sent if the encoding
    /// set for the form is "multipart/form-data"
    void addPart(const std::string& name, FormPart* part);

    /// Starts the sending thread.
    //void start();

    /// Stops the sending thread.
   // void stop();

    /// Returns true if the request is complete.
    bool complete() const;

    /// Returns true if the request is cancelled.
   // bool cancelled() const;

    /// Prepares the outgoing HTTP request object for submitting the form.
    ///
    /// If the request method is GET, the encoded form is appended to the
    /// request URI as query std::string. Otherwise (the method is
    /// POST), the form's content type is set to the form's encoding.
    /// The form's parameters must be written to the
    /// request body separately, with a call to write.
    /// If the request's HTTP version is HTTP/1.0:
    ///    - persistent connections are disabled
    ///    - the content transfer encoding is set to identity encoding
    /// Otherwise, if the request's HTTP version is HTTP/1.1:
    ///    - the request's persistent connection state is left unchanged
    ///    - the content transfer encoding is set to chunked
    void header();

    /// Processes the entire stream and calculates the content length.
    /// Not used for chunked encoding.
    uint64_t calculateMultipartContentLength();

    /// Writes "application/x-www-form-urlencoded" encoded data to
    /// the client connection.
    void writeUrl(std::ostream& ostr);

#if 0
    /// Writes the complete "multipart/form-data" request to the
    /// client connection. This method is blocking, and should be
    /// called from a thread, especially when sending large files.
    void writeMultipart();
#endif

    /// Writes the next multipart "multipart/form-data" encoded
    /// to the client connection. This method is non-blocking,    // and is
    /// suitable for use with the event loop.
    void writeMultipartChunk();

    /// Called asynchronously by the Runner to write the next message chunk.
    /// If "multipart/form-data" the next multipart chunk will be written.
    /// If "application/x-www-form-urlencoded" the entire message will be
    /// written.
    /// The complete flag will be set when the entire request has been written.
    
    
    void writeChunked();
    void run();
   
    /// Sets the encoding used for posting the form.
    ///
    /// Encoding must be either "application/x-www-form-urlencoded"
    /// (which is the default) or "multipart/form-data".
    void setEncoding(const std::string& encoding);

    /// Returns the encoding used for posting the form.
    const std::string& encoding() const;

    /// Sets the boundary to use for separating form parts.
    /// Must be set before prepareSubmit() is called.
    void setBoundary(const std::string& boundary);

    /// Returns the MIME boundary used for writing multipart form data.
    const std::string& boundary() const;

    /// The associated HTTP client connection.
    ClientConnecton* connection();

    /// The outgoing packet emitter.
//    PacketSignal emitter;

    static const char* ENCODING_URL;               ///< "application/x-www-form-urlencoded"
    static const char* ENCODING_MULTIPART_FORM;    ///< "multipart/form-data"
    static const char* ENCODING_MULTIPART_RELATED; ///< "multipart/related" http://tools.ietf.org/html/rfc2387
    
    static const char* TEXT_PLAIN; 
    static const char* APPLICATION_ZIP; 

   // CondWait condWait;
protected:
    /// Creates the FormWriter that uses the given encoding.
    ///
    /// Encoding must be either "application/x-www-form-urlencoded"
    /// (which is the default) or "multipart/form-data".
    FormWriter(ClientConnecton* conn,
               const std::string& encoding = FormWriter::ENCODING_URL);

    FormWriter(const FormWriter&) = delete;
    FormWriter& operator=(const FormWriter&) = delete;

    /// Writes the message boundary std::string, followed
    /// by the message header to the output stream.
    void writePartHeader(const NVCollection& header, std::ostream& ostr);

    /// Writes the final boundary std::string to the output stream.
    void writeEnd(std::ostream& ostr);

    /// Creates a random boundary std::string.
    ///
    /// The std::string always has the form boundary-XXXXXXXXXXXX,
    /// where XXXXXXXXXXXX is a randomly generate number.
    static std::string createBoundary();

    /// Updates the upload progress via the associated
    /// ConnectionStream object.
    virtual void updateProgress(int nread);

    friend class FormPart;
    friend class FilePart;
    friend class StringPart;

    
    struct Part
    {
        std::string name;
        FormPart* part;
    };

    typedef std::deque<Part> PartQueue;

    ClientConnecton* _stream;
    
    //IPacketizer *outgoing;
    
    void emit( const char * data, size_t len );
     
    void emit( const std::string &str ); 
    
    
            
//    std::shared_ptr<Runner> _runner;
    std::string _encoding;
    std::string _boundary;
    PartQueue _parts;
    uint64_t _filesLength;
    int _writeState;
    bool _initial;
    bool _complete;
};


//
// Form Part
//

/// An implementation of FormPart.
class  FormPart
{
public:
    /// Creates the FormPart with the given MIME type.
    FormPart(const std::string& contentType = "application/octet-stream");

    /// Destroys the FormPart.
    virtual ~FormPart();

    /// Reset the internal state and write position to the start.
    virtual void reset();

    /// Writes a form data chunk to the given HTTP client connection.
    /// Returns true if there is more data to be written.
    virtual bool writeChunk(FormWriter& writer) = 0;

    /// Writes the form data to the given HTTP client connection.
    virtual bool write(FormWriter& writer) = 0;

    /// Writes the form data to the given output stream.
    virtual void write(std::ostream& ostr) = 0;

    /// Returns a NVCollection containing additional header
    /// fields for the part.
    NVCollection& headers();

    /// Returns true if this is the initial write.
    virtual bool initialWrite() const;

    /// Returns the MIME type for this part or attachment.
    const std::string& contentType() const;

    /// Returns the length of the current part.
    virtual uint64_t length() const = 0;

protected:
    std::string _contentType;
    uint64_t _length;
    NVCollection _headers;
    bool _initialWrite;
};


//
// File Part
//

/// An implementation of FilePart for plain files.
class  FilePart : public FormPart
{
public:
    /// Creates the FilePart for the given path.
    ///
    /// The MIME type is set to application/octet-stream.
    ///
    /// Throws an FileException if the file cannot be opened.
    FilePart(const std::string& path);

    /// Creates the FilePart for the given
    /// path and MIME type.
    ///
    /// Throws an FileException if the file cannot be opened.
    FilePart(const std::string& path, const std::string& contentType);

    /// Creates the FilePart for the given
    /// path and MIME type. The given filename is
    /// used as part filename (see filename()) only.
    ///
    /// Throws an FileException if the file cannot be opened.
    FilePart(const std::string& path, const std::string& filename,
             const std::string& contentType);

    /// Destroys the FilePart.
    virtual ~FilePart();

    /// Opens the file.
    ///
    /// Throws an FileException if the file cannot be opened.
    virtual void open();

    /// Reset the internal state and write position to the start.
    virtual void reset();

    /// Writes a form data chunk to the given HTTP client connection.
    /// Returns true if there is more data to be written.
    virtual bool writeChunk(FormWriter& writer);

    /// Writes the form data to the given HTTP client connection.
    virtual bool write(FormWriter& writer);

    /// Writes the form data to the given output stream.
    virtual void write(std::ostream& ostr);

    /// Returns the filename portion of the path.
    const std::string& filename() const;

    /// Returns the file input stream.
    std::ifstream& stream();

    /// Returns the length of the current part.
    virtual uint64_t length() const;

    // /// Returns a NVCollection containing additional header
    // /// fields for the part.
    // NVCollection& headers();
    //
    // /// Returns the MIME type for this part or attachment.
    // const std::string& contentType() const;
    //
    // /// Returns the file size.
    // uint64_t fileSize() const;

protected:
    // std::string _contentType;
    std::string _path;
    std::string _filename;
    std::ifstream _istr;
    uint64_t _fileSize;
    // uint64_t _nWritten;
    // NVCollection _headers;
};


//
// String Part
//

/// An implementation of StringPart for plain files.
class  StringPart : public FormPart
{
public:
    /// Creates the StringPart for the given string.
    StringPart(const std::string& path);

    /// Creates the StringPart for the given string and MIME type.
    StringPart(const std::string& data, const std::string& contentType);

    /// Destroys the StringPart.
    virtual ~StringPart();

    /// Writes a form data chunk to the given HTTP client connection.
    /// Returns true if there is more data to be written.
    virtual bool writeChunk(FormWriter& writer);

    /// Writes the form data to the given HTTP client connection.
    virtual bool write(FormWriter& writer);

    /// Writes the form data to the given output stream.
    virtual void write(std::ostream& ostr);

    /// Returns the length of the current part.
    virtual uint64_t length() const;

protected:
    std::string _data;
    int curlen;
};


} // namespace http
} // namespace base


#endif // HTTP_Form_H

