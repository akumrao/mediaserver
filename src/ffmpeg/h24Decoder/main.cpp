/*
 
  BASIC GLFW + GLXW WINDOW AND OPENGL SETUP 
  ------------------------------------------
  See https://gist.github.com/roxlu/6698180 for the latest version of the example.
 
*/
#include <stdlib.h>
#include <stdio.h>

#if 0
#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

#define ROXLU_USE_MATH
#define ROXLU_USE_PNG
#define ROXLU_USE_OPENGL
#define ROXLU_IMPLEMENTATION
#include <tinylib.h>
#endif

#include "H264_Decoder.h"
#include "YUV420P_Player.h"
H264_Decoder* decoder_ptr = NULL;
//YUV420P_Player* player_ptr = NULL;
bool playback_initialized = false;

#if 0
void button_callback(GLFWwindow* win, int bt, int action, int mods);
void cursor_callback(GLFWwindow* win, double x, double y);
void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods);
void char_callback(GLFWwindow* win, unsigned int key);
#endif
void error_callback(int err, const char* desc);

#if 0
void resize_callback(GLFWwindow* window, int width, int height);
#endif

void frame_callback(AVFrame* frame, AVPacket* pkt, void* user);
void initialize_playback(AVFrame* frame, AVPacket* pkt);

int main() {
 #if 0
  glfwSetErrorCallback(error_callback);
 
  if(!glfwInit()) {
    printf("Error: cannot setup glfw.\n");
    return false;
  }
 

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  
  GLFWwindow* win = NULL;
  int w = 1516;
  int h = 853;

 

  win = glfwCreateWindow(w, h, "GLFW", NULL, NULL);
  if(!win) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwSetFramebufferSizeCallback(win, resize_callback);
  glfwSetKeyCallback(win, key_callback);
  glfwSetCharCallback(win, char_callback);
  glfwSetCursorPosCallback(win, cursor_callback);
  glfwSetMouseButtonCallback(win, button_callback);
  glfwMakeContextCurrent(win);
  glfwSwapInterval(1);
 
#if defined(__linux) || defined(_WIN32)
  if(glxwInit() != 0) {
    printf("Error: cannot initialize glxw.\n");
    ::exit(EXIT_FAILURE);
  }
#endif

 #endif
  // ----------------------------------------------------------------
  // THIS IS WHERE YOU START CALLING OPENGL FUNCTIONS, NOT EARLIER!!
  // ----------------------------------------------------------------
  //H264_Decoder decoder(frame_callback, NULL);

  H264_Decoder decoder(NULL, NULL);

 // YUV420P_Player player;

  //player_ptr = &player;
  decoder_ptr = &decoder;
  
  if(!decoder.load( "/var/tmp/test.264", 30.0f)) {
    ::exit(EXIT_FAILURE);
  }

  
   

    decoder.readFrame();
    
    decoder.readFrame();
    
    decoder.readFrame();
    
    
 
  
 #if 0
  while(!glfwWindowShouldClose(win)) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    decoder.readFrame();
    player.draw(0, 0, w, h);

    glfwSwapBuffers(win);
    glfwPollEvents();
  }
 
  glfwTerminate();
 #endif

  return EXIT_SUCCESS;
}
 

 #if 0

void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods) {
  
  if(action != GLFW_PRESS) {
    return;
  }

  switch(key) {
    case GLFW_KEY_ESCAPE: {
      glfwSetWindowShouldClose(win, GL_TRUE);
      break;
    }
  };
}



void frame_callback(AVFrame* frame, AVPacket* pkt, void* user) {

  if(!playback_initialized) {
    initialize_playback(frame, pkt);
    playback_initialized = true;
  }

  if(player_ptr) {
    player_ptr->setYPixels(frame->data[0], frame->linesize[0]);
    player_ptr->setUPixels(frame->data[1], frame->linesize[1]);
    player_ptr->setVPixels(frame->data[2], frame->linesize[2]);
  }
}

void initialize_playback(AVFrame* frame, AVPacket* pkt) {

  if(frame->format != AV_PIX_FMT_YUV420P) {
    printf("This code only support YUV420P data.\n");
    ::exit(EXIT_FAILURE);
  }

  if(!player_ptr) {
    printf("player_ptr not found.\n");
    ::exit(EXIT_FAILURE);
  }

  if(!player_ptr->setup(frame->width, frame->height)) {
    printf("Cannot setup the yuv420 player.\n");
    ::exit(EXIT_FAILURE);
  }
}

void error_callback(int err, const char* desc) {
  printf("GLFW error: %s (%d)\n", desc, err);
}




void resize_callback(GLFWwindow* window, int width, int height) { 

  if(player_ptr) {
    player_ptr->resize(width, height);
  }
}

void button_callback(GLFWwindow* win, int bt, int action, int mods) {
  double x,y;
  if(action == GLFW_PRESS || action == GLFW_REPEAT) { 
    glfwGetCursorPos(win, &x, &y);
  }
}

void cursor_callback(GLFWwindow* win, double x, double y) { }

void char_callback(GLFWwindow* win, unsigned int key) { }
 #endif


