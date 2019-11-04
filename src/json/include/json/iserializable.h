
#ifndef  CONF_JSON_ISerializable_h
#define  CONF_JSON_ISerializable_h


#include "json/json.h"
#include <ostream>


namespace base {
namespace cnfg {


class ISerializable
{
public:
    virtual ~ISerializable() = default;
    virtual void serialize(json::value& root) = 0;
    virtual void deserialize(json::value& root) = 0;
};


inline bool serialize(ISerializable* pObj, std::string& output)
{
    if (pObj == nullptr)
        return false;

    json::value serializeRoot;
    pObj->serialize(serializeRoot);
    output = serializeRoot.dump(4);
    return true;
}


inline bool deserialize(ISerializable* pObj, std::string& input)
{
    if (pObj == nullptr)
        return false;

    try {
        json::value deserializeRoot = json::value::parse(input.begin(), input.end());
        pObj->deserialize(deserializeRoot);
    }
    catch (std::invalid_argument&) {
        // LError("Cannot deserialize object: ", exc.what())
        return false;
    }

    return true;
}


} // namespace json
} // namespace base


#endif // JSON_ISerializable_h
