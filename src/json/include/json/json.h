//

#ifndef base_JSON_H
#define base_JSON_H


#include "base/base.h"
#include "base/error.h"

#include "json.hpp" // include nlohmann json

#include <cstdint>
#include <fstream>


// Shared library exports
#if defined(base_WIN) && defined(base_SHARED_LIBRARY)
    #if defined(JSON_EXPORTS)
        #define JSON_API __declspec(dllexport)
    #else
        #define JSON_API __declspec(dllimport)
    #endif
#else
    #define JSON_API // nothing
#endif

using namespace nlohmann;
using value = nlohmann::json;

namespace base {

namespace cnfg {


// Import the `json` type and friends to our namespace



inline void loadFile(const std::string& path, json & root)
{
    std::ifstream ifs;
    ifs.open(path.c_str(), std::ifstream::in);
    if (!ifs.is_open())
        throw std::runtime_error("Cannot open input file: " + path);

    root = json::parse(ifs); // thorws std::invalid_argument
}


inline void saveFile(const std::string& path, const json& root, int indent = 4)
{
    std::ofstream ofs(path, std::ios::binary | std::ios::out);
    if (!ofs.is_open())
        throw std::runtime_error("Cannot open output file: " + path);

    if (indent > 0)
        ofs << root.dump(indent);
    else
        ofs << root.dump();
    ofs.close();
}


inline void stringify(const json& root, std::string& output, bool pretty = false)
{
    if (pretty) {
        output = root.dump(4);
    } else {
        output = root.dump();
    }
}


inline std::string stringify(const json& root, bool pretty = false)
{
    std::string output;
    stringify(root, output, pretty);
    return output;
}


inline void assertMember(const json& root, const std::string& name)
{
    if (root.find(name) == root.end())
        throw std::runtime_error("A '" + name + "' member is required.");
}


inline void countNestedKeys(json& root, const std::string& key,
                            int& count, int depth = 0)
{
    depth++;
    for (auto it = root.begin(); it != root.end(); ++it) {
        if ((*it).is_object() && (*it).find(key) != (*it).end())
            count++;
        countNestedKeys(*it, key, count, depth);
    }
}


inline bool hasNestedKey(json& root, const std::string& key, int depth = 0)
{
    depth++;
    for (auto it = root.begin(); it != root.end(); it++) {
        if ((*it).is_object() && (*it).find(key) != (*it).end())
            return true;
        if (hasNestedKey(*it, key, depth))
            return true;
    }
    return false;
}


/// Only works for objects with string values.
/// Key or value may be empty for selecting wildcard values.
/// If partial is false substring matches will be accepted.
/// Result must be a reference to a pointer or the root value's
/// internal reference will also be changed when the result is
/// assigned. Further investigation is required.
inline bool findNestedObjectWithProperty(
    json& root, json*& result, const std::string& key,
    const std::string& value, bool partial = true, int index = 0,
    int depth = 0) 
{
    depth++;
    if (root.is_object()) {
        for (auto it = root.begin(); it != root.end(); it++) {
            json& test = it.value();
            if (test.is_null())
                continue;
            else if (test.is_string() &&
                     (key.empty() || it.value() == key) &&
                     (value.empty() ||
                      (!partial ? test.get<std::string>() == value
                                : test.get<std::string>().find(value) != std::string::npos))) {
                if (index == 0) {
                    result = &root;
                    return true;
                } else
                    index--;
            } else if ((test.is_object() || test.is_array()) &&
                       findNestedObjectWithProperty(it.value(),
                                                    result, key, value, partial,
                                                    index, depth))
                return true;
        }
    } else if (root.is_array()) {
        for (size_t i = 0; i < root.size(); i++) {
            json& test = root[(int)i];
            if (!test.is_null() && (test.is_object() || test.is_array()) &&
                findNestedObjectWithProperty(root[(int)i], result, key, value,
                                             partial, index, depth))
                return true;
        }
    }
    return false;
}


} // namespace cnfg
} // namespace base


#endif // base_JSON_H


/// @\}
