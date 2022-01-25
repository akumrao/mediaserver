/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "http/HTTPResponder.h"
#include "base/application.h"
#include "base/filesystem.h"

#include "GenToken.h"
#include "SecurityToken.h"
#include "base/uuid.h"

namespace base {
    namespace net {
        
        
            struct render_baton
            {

                render_baton() :
                request(),
                result(),
                response_code("200 OK"),
                content_type("text/plain"),
                error(false){
                    request.data = this;
                }
                std::string path;
                uv_work_t request;
                std::string result;
                std::string response_code;
                std::string content_type;
                bool error;
                //uv_loop_t *myloop{nullptr};
                HttpBase *con{nullptr}; 
            };
            
            
            bool endswith(std::string const& value, std::string const& search) {
                if (value.length() >= search.length()) {
                    return (0 == value.compare(value.length() - search.length(), search.length(), search));
                } else {
                    return false;
                }
            }

        
        
            void render(uv_work_t* req)
            {
                render_baton *closure = static_cast<render_baton *> (req->data);
               // client_t* client = (client_t*) closure->client;
                //LOGF("[ %5d ] render\n", client->request_num);

                //closure->result = "hello universe";
                closure->response_code = "200 OK";

                
                //#if 0
                std::string filepath(".");
                //std::string filepath("/workspace/mediaserver/src/rtsp/main");
                filepath += closure->path;
                
                
               // std::string index_path = (filepath + "index.html");

                std::cout << "file Path: " << filepath << std::endl;

                /*bool has_index = base::fs::exists(index_path);               /// (access(index_path.c_str(), R_OK) != -1);
                if (!has_index && filepath[filepath.size() - 1] == '/')
                {
                    uv_fs_t scandir_req;
                    uv_fs_scandir(req->loop, &scandir_req, filepath.c_str(), 0, NULL);
                    uv_dirent_t dent;
                    closure->content_type = "text/html";
                    closure->result = "<html><body><ul>";
                    while (UV_EOF != uv_fs_scandir_next(&scandir_req, &dent))
                    {
                        std::string name = dent.name;
                        if (dent.type == UV_DIRENT_DIR)
                        {
                            name += "/";
                        }
                        closure->result += "<li><a href='";
                        closure->result += name;
                        closure->result += "'>";
                        closure->result += name;
                        closure->result += "</a></li>";
                        closure->result += "\n";
                    }
                    closure->result += "</ul></body></html>";
                    uv_fs_req_cleanup(&scandir_req);
                } else
                */
                {
                    //std::string file_to_open = filepath;
                   /* if (has_index)
                    {
                        file_to_open = index_path;
                    }*/
                    //std::cout << "file Path: " << file_to_open << std::endl;
                    bool exists =  base::fs::exists(filepath); //(access(file_to_open.c_str(), R_OK) != -1);
                    if (!exists)
                    {
                        closure->result = "no access";
                        closure->response_code = "404 Not Found";
                        return;
                    }
                    FILE * f = fopen(filepath.c_str(), "rb");
                    if (f)
                    {
                        std::fseek(f, 0, SEEK_END);
                        unsigned size = std::ftell(f);
                        std::fseek(f, 0, SEEK_SET);
                        closure->result.resize(size);
                        std::fread(&closure->result[0], size, 1, f);

                        //std::cout << &closure->result[0] << std::endl;

                        fclose(f);
                        if (endswith(filepath, "html"))
                        {
                            closure->content_type = "text/html";
                        } else if (endswith(filepath, "css"))
                        {
                            closure->content_type = "text/css";
                        } else if (endswith(filepath, "js"))
                        {
                            closure->content_type = "application/javascript";
                        }
                        else if (endswith(filepath, "ico"))
                        {
                              closure->content_type = "image/x-icon";  
                        } else if (endswith(filepath, "gif"))
                        {
                             closure->content_type = "image/gif"; 
                             
                        } else if (endswith(filepath, "jpeg"))
                        {
                             closure->content_type = "image/jpeg"; 
                        } else if (endswith(filepath, "png"))
                        {
                             closure->content_type = "image/png"; 
                        } else {
                            SError << "file format not supported " << filepath;
                        }
                    
                        
                    } else
                    {
                        closure->result = "failed to open";
                        closure->response_code = "404 Not Found";
                    }
                }
                //#endif
            }

            void after_render(uv_work_t* req){
                render_baton *closure = static_cast<render_baton *> (req->data);
               // client_t* client = (client_t*) closure->client;

               // LOGF("[ %5d ] after render\n", client->request_num);

                std::ostringstream rep;
                rep << "HTTP/1.1 " << closure->response_code << "\r\n"
                        << "Content-Type: " << closure->content_type << "\r\n"
                        //<< "Connection: keep-alive\r\n"
                        << "Connection: close\r\n"
                        << "Content-Length: " << closure->result.size() << "\r\n"
                        << "Access-Control-Allow-Origin: *" << "\r\n"
                        << "\r\n";
                rep << closure->result;
                std::string res = rep.str();

               // SInfo << res;

                //uv_buf_t resbuf;
              //  resbuf.base = (char *) res.c_str();
               // resbuf.len = res.size();
             
                auto cb =  onSendCallback([&closure ](bool sent)
                {
                    if (sent)
                    {
                        closure->con->Close();
                       // delete closure; //Sanjay modified for preventing excption double delete in queue
                      //  closure = NULL;
                    }
                }
                
                );
                  
                closure->con->tcpsend( res.c_str(), res.size(), cb );

             //   client->write_req.data = closure;

                // https://github.com/joyent/libuv/issues/344
    //            int r = uv_write(&client->write_req,
    //                    (uv_stream_t*) client->handle,
    //                    &resbuf, 1, after_write);    //arvind
                //CHECK(r, "write buff");
                
                
        }
        
      
        
        
        
        void BasicResponder::sendResponse(std::string &result, bool success)
        {
            std::string response_code("200 OK");
            std::string content_type("text/plain");
            
            if(!success)
              response_code =  "404 Not Found";
                      
             std::ostringstream rep;
                rep << "HTTP/1.1 " << response_code << "\r\n"
                        << "Content-Type: " << content_type << "\r\n"
                        //<< "Connection: keep-alive\r\n"
                        << "Connection: close\r\n"
                        << "Content-Length: " << result.size() << "\r\n"
                        << "Access-Control-Allow-Origin: *" << "\r\n"
                        << "\r\n";
                rep << result;
                std::string res = rep.str();
            
            HttpBase *con = connection();     
                
            auto cb =  onSendCallback([&con ](bool sent)
            {
                if (sent)
                {
                    con->Close();

                }
            }

            );

            connection()->tcpsend( res.c_str(), res.size(), cb );
        }
        
        bool BasicResponder::authcheck(net::Request& request, std::string &ret, bool tokenOnly) 
        {
            
            if(request.has("key"))
            {
                std::string key =  request.get("key");
               
                if(request.has("token"))
                {
                    
                    std::string token =  request.get("token");
                    
                    std::string perm;
                    uint32_t statusCode;


                     MS_SecurityToken obj(token);
                     obj.validate(key, ret, perm, statusCode, false);
                     
                     if(statusCode == 200)
                     return true;      
                     else
                     return false;
                }
                else if(request.has("exp") && request.has("perm") )
                {
                    std::string cam;
                    if(request.has("cam"))
                    {
                       cam =  request.get("cam");
                    }
                    else
                    {
                        cam = uuid4::uuid();
                    }
                    
                    std::string exp =  request.get("exp");
                    
                    int iexp = std::stoi(exp);
                    
                    std::string perm =  request.get("perm");
                    
                    
                   ret= SecToken::createSecurityToken(cam, perm, key, iexp);
                    
                     return true;   
                    
                }
                else
                {
                   if( tokenOnly)
                    ret = "token missing";
                   else
                   ret = "(expiring(secs) or permission missing. Please set exp as 360 and perm as w ";
                   return false;
                    
                }
               
           }
           else
           {
                ret = "key missing. Please set key as admin@passsword";
                return false;
           }
          
            
            
        }
         

        void BasicResponder::onRequest(net::Request& request, net::Response& response) {
            STrace << "On complete" << std::endl;

            //response.setContentLength(14); // headers will be auto flushed

           // connection()->send((const char *) "hello universe", 14);
           // connection()->Close();  // wrong we should close close after write is successful. Check the callback onSendCallback function
            std::string reponse= "hello universe";
            sendResponse(reponse, true);
            
        }
        
        
        
       HttpResponder::HttpResponder(net::HttpBase* conn) : ServerResponder(conn) 
       {
            closure = new render_baton();
           
            SInfo << "HttpResponder()" <<  this;
                
       }
          
        HttpResponder::~HttpResponder() 
       {
            SInfo << "~HttpResponder()" <<  this;
            if(closure)
            delete closure;
                    
           closure = nullptr;
                    
       }
        
        
        void  HttpResponder::onClose() {
            
            SInfo << "onClose close  render_baton " << closure ;
           
         
           // SInfo << ": onClose:\n" ;
        }

        void HttpResponder::onRequest(net::Request& request, net::Response& response) {
            STrace << "On complete" << std::endl;

            
           
 
            auto& request1 = connection()->_request;

            // Log incoming requests
            SInfo << ": response:\n" << response ;


            //  response.setContentLength(14); // headers will be auto flushed

            //connection()->send((const char *) "hello universe", 14);
            // connection()->Close();
            std::string file_to_open;

#if 1
            if (request1.getURI() == "/") {
                file_to_open = "/index.html";
            } else {
                file_to_open =  request1.getURI();
            }
#else           
           // arvind for directory list 
           file_to_open = request1.getURI();
#endif

            SInfo << "Response file Path: " << file_to_open << std::endl;

             SInfo << "open render_baton " << closure ;
            
            
            closure->path = file_to_open;
            closure->con = connection();
            closure->request.data = closure;
            //int status = uv_queue_work(Application::uvGetLoop(),
            //        &closure->request,
            //        render,
            //        (uv_after_work_cb) after_render);
            //assert(status == 0);

            render(&closure->request);  //Removing call to uv_queue_work to avoid thread issues. Our client is already multithreaded

            after_render(&closure->request);
            
            return;
            
      
//            std::string result;
//            FILE * f = fopen(file_to_open.c_str(), "rb");
//            if (f) {
//                std::fseek(f, 0, SEEK_END);
//                unsigned size = std::ftell(f);
//                std::fseek(f, 0, SEEK_SET);
//                result.resize(size);
//                std::fread(&result[0], size, 1, f);
//
//                //std::cout << &closure->result[0] << std::endl;
//
//                fclose(f);
//                //           
//                if (endswith(file_to_open, "html")) {
//                    response.setContentType("text/html");
//                } else if (endswith(file_to_open, "css")) {
//                    response.setContentType("text/css");
//                } else if (endswith(file_to_open, "js")) {
//                    response.setContentType("application/javascript");
//                } else if (endswith(file_to_open, "ico")) {
//                    response.setContentType("image/x-icon");
//                } else if (endswith(file_to_open, "gif")) {
//                    response.setContentType("image/gif");
//                } else if (endswith(file_to_open, "jpeg")) {
//                    response.setContentType("image/jpeg");
//                } else if (endswith(file_to_open, "png")) {
//                    response.setContentType("image/png");
//                } else {
//                    SError << "file format not supported " << file_to_open;
//                }
//
//
//
//                response.setContentLength(size); // headers will be auto flushed
//
//
//                connection()->send(result.c_str(), size);
//                connection()->Close();
//
//            } else {
//                response.setStatusAndReason(StatusCode::BadRequest, "File not found");
//
//                response.setContentLength(file_to_open.length()); // headers will be auto flushed
//
//                connection()->send((const char *) file_to_open.c_str(), file_to_open.length());
//                connection()->Close();
//
//            }
        }

    }
}
