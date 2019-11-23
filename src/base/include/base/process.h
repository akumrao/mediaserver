

#ifndef base_Process_H
#define base_Process_H


#include "base/base.h"
#include <functional>
#include <vector>
#include <initializer_list>
#include "uv.h"
#include <string>


#define ASSERT(expr)                                      \
 do {                                                     \
  if (!(expr)) {                                          \
    fprintf(stderr,                                       \
            "Assertion failed in %s on line %d: %s\n",    \
            __FILE__,                                     \
            __LINE__,                                     \
            #expr);                                       \
    abort();                                              \
  }                                                       \
 } while (0)


namespace base {




    typedef uv_process_options_t ProcessOptions;

    class Process {
    public:
        /// Default constructor.
        Process(uv_loop_t* loop = uv_default_loop());

        /// Constructor with command line arguments.
        Process(std::initializer_list<std::string> args, uv_loop_t* loop = uv_default_loop());

        /// Destructor.
        ~Process();

        /// Path to the program to execute.
        /// Cenvenience proxy for options.file.
        /// Must be set before `spawn()`
        std::string file;

        /// Set the current working directory.
        /// Cenvenience proxy for options.cwd.
        /// Must be set before `spawn()`
        std::string cwd;

        /// Command line agruments to pass to the process.
        /// Cenvenience proxy for options.args.
        /// Must be set before `spawn()`
        std::vector<std::string> args;

        /// Spawns the process.
        /// Options must be properly set.
        /// Throws and exception on error.
        void spawn();

        /// Kills the process
        bool kill(int signum = SIGKILL);

        /// Returns the process PID
        int pid() const;



        /// Stdout signal.
        /// Signals when a line has been output from the process.
        std::function<void(std::string) > onstdout;

        /// Exit stgnals.
        /// Signals process exit status code.
        std::function<void(std::int64_t) > onexit;

        /// LibUV C options.
        /// Available for advanced use cases.
        ProcessOptions options;
        
        int on_read_cb_called;

    protected:
        void init();

        

        uv_process_t _handle;
        uv_pipe_t in;
        uv_pipe_t out;
        uv_pipe_t err;
        uv_loop_t* m_loop;
        //Pipe _stdin;
        // Pipe _stdout;
        uv_stdio_container_t _stdio[3];
        std::vector<char*> _cargs;
    };


} // namespace base


#endif // base_Process_H


