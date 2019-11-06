

#ifndef base_JSON_Configuration_H
#define base_JSON_Configuration_H


#include "json/json.h"
#include "base/util.h"
#include <mutex>


namespace base {
namespace cnfg {


/// JSON configuration file
///
/// See base Configuration for all accessors
class Configuration 
{
public:
    Configuration();
    virtual ~Configuration();

    virtual void load(const std::string path, bool create = false);
   
    virtual void save();

    virtual bool remove(const std::string& key);
    virtual void removeAll(const std::string& baseKey);
    virtual void replace(const std::string& from, const std::string& to);
    virtual void keys(std::vector<std::string>& keys, const std::string& baseKey = "");
    virtual void print(std::ostream& ost);

    virtual std::string path();
    virtual bool loaded();

    json root;


    virtual bool getRaw(const std::string& key, std::string& value) const ;
    virtual void setRaw(const std::string& key, const std::string& value) ;

private:
     virtual void loadIt(bool create);

    bool _loaded;
    std::string _path;
    mutable std::mutex _mutex;
};


} // namespace json
} // namespace base


#endif // base_JSON_Configuration_H


/// @\}
