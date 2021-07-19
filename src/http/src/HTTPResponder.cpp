/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "http/HTTPResponder.h"


namespace base {
    namespace net {

        bool endswith(std::string const& value, std::string const& search) {
            if (value.length() >= search.length()) {
                return (0 == value.compare(value.length() - search.length(), search.length(), search));
            } else {
                return false;
            }
        }

        void BasicResponder::onRequest(net::Request& request, net::Response& response) {
            STrace << "On complete" << std::endl;

            response.setContentLength(14); // headers will be auto flushed

            connection()->send((const char *) "hello universe", 14);
            connection()->Close();
        }

        void HttpResponder::onRequest(net::Request& request, net::Response& response) {
            STrace << "On complete" << std::endl;


            auto& request1 = connection()->_request;

            // Log incoming requests
            SInfo << ": response:\n" << response << std::endl;


            //  response.setContentLength(14); // headers will be auto flushed

            //connection()->send((const char *) "hello universe", 14);
            // connection()->Close();
            std::string file_to_open;

            if (request1.getURI() == "/") {
                file_to_open = "./index.html";
            } else {
                file_to_open = "." + request1.getURI();
            }



            SInfo << "Response file Path: " << file_to_open << std::endl;

            std::string result;
            FILE * f = fopen(file_to_open.c_str(), "rb");
            if (f) {
                std::fseek(f, 0, SEEK_END);
                unsigned size = std::ftell(f);
                std::fseek(f, 0, SEEK_SET);
                result.resize(size);
                std::fread(&result[0], size, 1, f);

                //std::cout << &closure->result[0] << std::endl;

                fclose(f);
                //           
                if (endswith(file_to_open, "html")) {
                    response.setContentType("text/html");
                } else if (endswith(file_to_open, "css")) {
                    response.setContentType("text/css");
                } else if (endswith(file_to_open, "js")) {
                    response.setContentType("application/javascript");
                } else if (endswith(file_to_open, "ico")) {
                    response.setContentType("image/x-icon");
                } else if (endswith(file_to_open, "gif")) {
                    response.setContentType("image/gif");
                } else if (endswith(file_to_open, "jpeg")) {
                    response.setContentType("image/jpeg");
                } else if (endswith(file_to_open, "png")) {
                    response.setContentType("image/png");
                } else {
                    SError << "file format not supported " << file_to_open;
                }



                response.setContentLength(size); // headers will be auto flushed


                connection()->send(result.c_str(), size);
                connection()->Close();

            } else {
                response.setStatusAndReason(StatusCode::BadRequest, "File not found");

                response.setContentLength(file_to_open.length()); // headers will be auto flushed

                connection()->send((const char *) file_to_open.c_str(), file_to_open.length());
                connection()->Close();

            }
        }

    }
}