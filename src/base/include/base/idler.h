

#ifndef base_IDLER_H
#define base_IDLER_H



#include "base/base.h"
#include <functional>
#include "uv.h"

namespace base {

///
class Idler 
{
public:
    /// Create the idler with the given event loop.
    Idler();

    /// Create and start the idler with the given callback.
    Idler(std::function<void()> cbfun);

    std::function<void()> cbfun;
   
    /// Start the idler with the given callback function.
    virtual void start();

    virtual ~Idler();

    virtual void stop();
    virtual void run();

protected:

    uv_idle_t idler;
};


} // namespace scy


#endif // SCY_Idler_H

