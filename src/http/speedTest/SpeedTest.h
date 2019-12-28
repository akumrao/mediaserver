#ifndef SPEEDTEST_SPEEDTEST_H
#define SPEEDTEST_SPEEDTEST_H

#include "SpeedTestConfig.h"
#include "SpeedTestClient.h"
#include <functional>
#include <cmath>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <thread>
#include <mutex>
#include "DataTypes.h"

class SpeedTestClient;
typedef bool (SpeedTestClient::*opFn)(const long size, const long chunk_size, long &millisec);
typedef void (*progressFn)(bool success);


class SpeedTest {
public:
    explicit SpeedTest(float minServerVersion);
    ~SpeedTest();
    static std::map<std::string, std::string> parseQueryString(const std::string& query);
    static std::vector<std::string> splitString(const std::string& instr, char separator);
    bool ipInfo(IPInfo& info);
    const std::vector<ServerInfo>& serverList();
    const ServerInfo bestServer(int sample_size = 5, std::function<bool(bool)> cb = nullptr);
    bool setServer(ServerInfo& server);
    const long &latency();
    bool downloadSpeed(const ServerInfo& server, const TestConfig& config, double& result, std::function<bool(bool,double)> cb = nullptr);
    bool uploadSpeed(const ServerInfo& server, const TestConfig& config, double& result, std::function<bool(bool,double)> cb = nullptr);
    bool jitter(const ServerInfo& server, long& result, int sample = 40);

private:
    bool fetchServers(const std::string& url,  std::vector<ServerInfo>& target, int &http_code);
    bool testLatency(SpeedTestClient& client, int sample_size, long& latency);
    const ServerInfo findBestServerWithin(const std::vector<ServerInfo>& serverList, long& latency, int sample_size = 5, std::function<bool(bool)> cb = nullptr);
    static size_t writeFunc(void* buf, size_t size, size_t nmemb, void* userp);
    double execute(const ServerInfo &server, const TestConfig &config, const opFn &fnc, std::function<bool(bool,double)> cb = nullptr);
    template <typename T>
        static T deg2rad(T n);
    template <typename T>
        static T harversine(std::pair<T, T> n1, std::pair<T, T> n2);

    IPInfo mIpInfo;
    std::vector<ServerInfo> mServerList;
    long mLatency;
    double mUploadSpeed;
    double mDownloadSpeed;
    float mMinSupportedServer;

};


#endif //SPEEDTEST_SPEEDTEST_H
