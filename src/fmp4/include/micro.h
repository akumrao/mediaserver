#ifndef macro_HEADER_GUARD
#define macro_HEADER_GUARD



#define ban_copy_ctor(CLASS) \
CLASS(const CLASS &f) {\
  std::cout << "FATAL: copy-construction prohibited for this class" << std::endl;\
  perror("FATAL: copy-construction prohibited for this class");\
  exit(2);\
};\

#define ban_copy_asm(CLASS) \
CLASS &operator= (const CLASS &f) {\
  std::cout << "FATAL: copy assignment prohibited for this class" << std::endl;\
  perror("FATAL: copy assignment prohibited for this class");\
  exit(2);\
};\

#define notice_ban_copy_ctor() \
{\
  std::cout << "FATAL: copy-construction prohibited for this class" << std::endl;\
  perror("FATAL: copy-construction prohibited for this class");\
  exit(2);\
};\



// Macros for making getFrameClass and copyFrom
// use the implicit copy assignment through copyFrom
// prohibit copy-construction: frames are pre-reserved and copied, not copy-constructed
#define frame_essentials(CLASSNAME, CLASS) \
virtual FrameClass getFrameClass() {\
  return CLASSNAME;\
};\
virtual void copyFrom(Frame *f) {\
  CLASS *cf;\
  cf=dynamic_cast<CLASS*>(f);\
  if (!cf) {\
    perror("FATAL : invalid cast at copyFrom");\
    exit(5);\
  }\
  *this =*(cf);\
};\
CLASS(const CLASS &f) {\
  std::cout << "FATAL: copy-construction prohibited for frame classes" << std::endl;\
  perror("FATAL: copy-construction prohibited for frame classes");\
  exit(2);\
};\

#define frame_clone(CLASSNAME, CLASS) \
virtual CLASS *getClone() {\
    CLASS *tmpframe = new CLASS();\
    *tmpframe = *this;\
    return tmpframe;\
};\


// macros for initializing and deallocating SignalFrame's custom_signal_ctx
// nopes.. bad idea
/*
#define init_signal_frames(FIFONAME, CONTEXTCLASS) \
Reservoir &res = FIFONAME.getReservoir(FrameClass::signal);\
for(auto it=res.begin(); it!=res.end(); ++it) {\
    SignalFrame* f = static_cast<SignalFrame*>(*it);\
    f->custom_signal_ctx = (void*)(new CONTEXTCLASS());\
};\

#define clear_signal_frames(FIFONAME, CONTEXTCLASS) \
Reservoir &res = FIFONAME.getReservoir(FrameClass::signal);\
for(auto it=res.begin(); it!=res.end(); ++it) {\
    SignalFrame* f = static_cast<SignalFrame*>(*it);\
    delete (CONTEXTCLASS*)(f->custom_signal_ctx);\
};\
*/

#define get_signal_context(SIGNALFRAME, CONTEXT) \
memcpy((void*)&CONTEXT, (void*)((SIGNALFRAME)->signal_ctx_buf.data()), sizeof(CONTEXT));\

#define put_signal_context(SIGNALFRAME, CONTEXT, CONTEXT_NUM) \
(SIGNALFRAME)->signal_ctx_buf.resize(sizeof(CONTEXT));\
memcpy((void*)((SIGNALFRAME)->signal_ctx_buf.data()), (void*)&CONTEXT, sizeof(CONTEXT));\
(SIGNALFRAME)->signaltype=CONTEXT_NUM;\


// generic error handling
#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)


#endif
