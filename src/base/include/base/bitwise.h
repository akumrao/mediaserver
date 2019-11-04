
#ifndef base_Flaggable_H
#define base_Flaggable_H


#include "base/base.h"


namespace base {


/// Container for smart management of bitwise integer flags.
struct Bitwise
{
    unsigned data; // storage integer

    Bitwise(unsigned flags = 0)
        : data(flags)
    {
    }

    virtual void reset() { data = 0; };
    virtual void set(unsigned flag)
    {
        if (!has(flag))
            data |= flag;
    }

    virtual void add(unsigned flag) { data |= flag; };
    virtual void remove(unsigned flag) { data &= ~flag; };
    virtual void toggle(unsigned flag) { data ^= flag; };
    virtual bool has(unsigned flag) const { return (data & flag) == flag; };
};


#if 0
class Flaggable
{
public:
    Flaggable(unsigned flags = 0) : _flags(flags) {}
    Flaggable(const Flaggable& r) : _flags(r._flags) {}
    Flaggable(const Flags& r) : _flags(r) {}

    virtual void setFlag(unsigned flag) {
        _flags.set(flag);
    };
    virtual void addFlag(unsigned flag) {
        _flags.add(flag);
    };
    virtual void removeFlag(unsigned flag) {
        _flags.remove(flag);
    };
    virtual void toggleFlag(unsigned flag) {
        _flags.toggle(flag);
    };
    virtual bool hasFlag(unsigned flag) const {
        return _flags.has(flag);
    };
    virtual void setFlags(Flags flags) {
        _flags = flags;
    };
    virtual Flags flags() const { return _flags; };

protected:
    Flags _flags;
};
#endif


} // namespace base


#endif // base_Flaggable_H


/// @\}
