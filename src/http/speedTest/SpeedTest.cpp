
#include <cmath>
#include <iomanip>
#include "SpeedTest.h"
//#include "MD5Util.h"
#include <netdb.h>

SpeedTest::SpeedTest(float minServerVersion) :
mLatency(0),
mUploadSpeed(0),
mDownloadSpeed(0) {
    //    curl_global_init(CURL_GLOBAL_DEFAULT);
    mIpInfo = IPInfo();
    mServerList = std::vector<ServerInfo>();
    mMinSupportedServer = minServerVersion;
}

SpeedTest::~SpeedTest() {
    //    curl_global_cleanup();
    mServerList.clear();
}

bool SpeedTest::ipInfo(IPInfo& info) {
    //mIpInfo.ip_address ="116.73.242.28";
    if (!mIpInfo.ip_address.empty()) {
        info = mIpInfo;
        return true;
    }
    /*
        std::stringstream oss;
        auto code = httpGet(SPEED_TEST_IP_INFO_API_URL, oss);
        if (code == CURLE_OK) {
            auto values = SpeedTest::parseQueryString(oss.str());
            mIpInfo = IPInfo();
            mIpInfo.ip_address = values["ip_address"];
            mIpInfo.isp = values["isp"];
            mIpInfo.lat = std::stof(values["lat"]);
            mIpInfo.lon = std::stof(values["lon"]);
            values.clear();
            oss.clear();
            info = mIpInfo;
            return true;
        }
     */
    return false;
}

const std::vector<ServerInfo>& SpeedTest::serverList() {
    if (!mServerList.empty())
        return mServerList;


    mServerList = {
        {"http://speedtestkk1.airtel.in:8080/speedtest/upload.php", 12.9833, 77.5833, "Bangalore", "India", "IN", "Bharti Airtel Ltd", 2564, "speedtestkk1.airtel.in:8080", 0.0},
        {"http://speedtestblr.airtelbroadband.in:8080/speedtest/upload.php", 12.9833, 77.5833, "Bangalore", "India", "IN", "Airtel Broadband", 18976, "speedtestblr.airtelbroadband.in:8080", 0.0},
        {"http://bangspeed.hathway.com:8080/speedtest/upload.php", 12.9833, 77.5833, "Bangalore", "India", "IN", "Hathway Cable Datacom Ltd", 4663, "bangspeed.hathway.com:8080", 0.0},
        {"http://ooklatestbenguluru.live.vodafone.in:8080/speedtest/upload.php", 12.9716,77.5946, "Benguluru","India","IN","Vodafone India",24683,"ooklatestbenguluru.live.vodafone.in:8080",0.0}

    };

    // ServerInfo sf("http://bangspeed.hathway.com:8080/speedtest/upload.php", 12.9833, 77.5833, "Bangalore", "India", "IN", "Hathway Cable Datacom Ltd", 4663, "bangspeed.hathway.com:8080", 0.0);


    //mServerList.push_back(sf);
    return mServerList;
}

const ServerInfo SpeedTest::bestServer(const int sample_size, std::function<bool(bool) > cb) {
    auto best = findBestServerWithin(serverList(), mLatency, sample_size, cb);
    SpeedTestClient client = SpeedTestClient(best);
    testLatency(client, SPEED_TEST_LATENCY_SAMPLE_SIZE, mLatency);
    client.close();
    return best;
}

bool SpeedTest::setServer(ServerInfo& server) {
    SpeedTestClient client = SpeedTestClient(server);
    if (client.connect() && client.version() >= mMinSupportedServer) {
        if (!testLatency(client, SPEED_TEST_LATENCY_SAMPLE_SIZE, mLatency)) {
            return false;
        }
    } else {
        client.close();
        return false;
    }
    client.close();
    return true;

}

bool SpeedTest::downloadSpeed(const ServerInfo &server, const TestConfig &config, double& result, std::function<bool(bool, double) > cb) {
    opFn pfunc = &SpeedTestClient::download;
    mDownloadSpeed = execute(server, config, pfunc, cb);
    result = mDownloadSpeed;
    return true;
}

bool SpeedTest::uploadSpeed(const ServerInfo &server, const TestConfig &config, double& result, std::function<bool(bool, double) > cb) {
    opFn pfunc = &SpeedTestClient::upload;
    mUploadSpeed = execute(server, config, pfunc, cb);
    result = mUploadSpeed;
    return true;
}

const long &SpeedTest::latency() {
    return mLatency;
}

bool SpeedTest::jitter(const ServerInfo &server, long& result, const int sample) {
    auto client = SpeedTestClient(server);
    double current_jitter = 0;
    long previous_ms = LONG_MAX;
    if (client.connect()) {
        for (int i = 0; i < sample; i++) {
            long ms = 0;
            if (client.ping(ms)) {
                if (previous_ms == LONG_MAX) {
                    previous_ms = ms;
                } else {
                    current_jitter += std::abs(previous_ms - ms);
                }
            }
        }
        client.close();
    } else {
        return false;
    }

    result = (long) std::floor(current_jitter / sample);
    return true;
}



// private

double SpeedTest::execute(const ServerInfo &server, const TestConfig &config, const opFn &pfunc, std::function<bool(bool, double) > cb) {
    std::vector<std::thread> workers;
    double overall_speed = 0;
    std::mutex mtx;
    for (int i = 0; i < config.concurrency; i++) {
        workers.push_back(std::thread([&server, &overall_speed, &pfunc, &config, &mtx, cb]() {
            long start_size = config.start_size;
            long max_size = config.max_size;
            long incr_size = config.incr_size;
            long curr_size = start_size;

            auto spClient = SpeedTestClient(server);

            if (spClient.connect()) {
                long total_size = 0;
                        long total_time = 0;
                        auto start = std::chrono::steady_clock::now();
                        std::vector<double> partial_results;
                while (curr_size < max_size) {
                    long op_time = 0;
                    if ((spClient.*pfunc)(curr_size, config.buff_size, op_time)) {
                        total_size += curr_size;
                                total_time += op_time;
                                double metric = (curr_size * 8) / (static_cast<double> (op_time) / 1000);
                                partial_results.push_back(metric);
                        if (cb) {
                            bool ret = cb(true, metric);
                            if (ret)
                                break;
                            }
                    } else {
                        if (cb) {
                            bool ret = cb(false, 0);
                            if (ret)
                                break;
                            }
                    }
                    curr_size += incr_size;
                            auto stop = std::chrono::steady_clock::now();
                    if (std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() > config.min_test_time_ms)
                        break;
                    }

                spClient.close();
                        std::sort(partial_results.begin(), partial_results.end());

                        size_t skip = 0;
                        size_t drop = 0;
                if (partial_results.size() >= 10) {
                    skip = partial_results.size() / 4;
                            drop = 2;
                }

                size_t iter = 0;
                        double real_sum = 0;
                for (auto it = partial_results.begin() + skip; it != partial_results.end() - drop; ++it) {
                    iter++;
                            real_sum += (*it);
                }
                mtx.lock();
                        overall_speed += (real_sum / iter);
                        mtx.unlock();
            } else {
                if (cb) {
                    bool ret = cb(false, 0);
                   // if (ret)
                     //   break;
                    }
            }
        }));

    }
    for (auto &t : workers) {
        t.join();
    }

    workers.clear();

    return overall_speed / 1000 / 1000;
}

template<typename T>
T SpeedTest::deg2rad(T n) {
    return (n * M_PI / 180);
}

template<typename T>
T SpeedTest::harversine(std::pair<T, T> n1, std::pair<T, T> n2) {
    T lat1r = deg2rad(n1.first);
    T lon1r = deg2rad(n1.second);
    T lat2r = deg2rad(n2.first);
    T lon2r = deg2rad(n2.second);
    T u = std::sin((lat2r - lat1r) / 2);
    T v = std::sin((lon2r - lon1r) / 2);
    return 2.0 * EARTH_RADIUS_KM * std::asin(std::sqrt(u * u + std::cos(lat1r) * std::cos(lat2r) * v * v));
}

size_t SpeedTest::writeFunc(void *buf, size_t size, size_t nmemb, void *userp) {

    if (userp) {
        std::stringstream &os = *static_cast<std::stringstream *> (userp);
        std::streamsize len = size * nmemb;
        if (os.write(static_cast<char*> (buf), len))
            return static_cast<size_t> (len);
    }
    return 0;
}

std::map<std::string, std::string> SpeedTest::parseQueryString(const std::string &query) {
    auto map = std::map<std::string, std::string>();
    auto pairs = splitString(query, '&');
    for (auto &p : pairs) {
        auto kv = splitString(p, '=');
        if (kv.size() == 2) {
            map[kv[0]] = kv[1];
        }
    }
    return map;
}

std::vector<std::string> SpeedTest::splitString(const std::string &instr, const char separator) {
    if (instr.empty())
        return std::vector<std::string>();

    std::vector<std::string> tokens;
    std::size_t start = 0, end = 0;
    while ((end = instr.find(separator, start)) != std::string::npos) {
        std::string temp = instr.substr(start, end - start);
        if (!temp.empty())
            tokens.push_back(temp);
        start = end + 1;
    }
    std::string temp = instr.substr(start);
    if (!temp.empty())
        tokens.push_back(temp);
    return tokens;

}

const ServerInfo SpeedTest::findBestServerWithin(const std::vector<ServerInfo> &serverList, long &latency,
        const int sample_size, std::function<bool(bool) > cb) {
    int i = sample_size;
    ServerInfo bestServer = serverList[0];

    latency = INT_MAX;

    for (auto &server : serverList) {
        auto client = SpeedTestClient(server);

        if (!client.connect()) {
            if (cb) {
                bool ret = cb(false);
                if (ret)
                    break;
            }
            continue;
        }

        if (client.version() < mMinSupportedServer) {
            client.close();
            continue;
        }

        long current_latency = LONG_MAX;
        if (testLatency(client, 20, current_latency)) {
            if (current_latency < latency) {
                latency = current_latency;
                bestServer = server;
            }
        }
        client.close();
        if (cb) {
            bool ret = cb(true);
            if (ret)
                break;
        }

        if (i-- < 0) {
            break;
        }

    }
    return bestServer;
}

bool SpeedTest::testLatency(SpeedTestClient &client, const int sample_size, long &latency) {
    if (!client.connect()) {
        return false;
    }
    latency = INT_MAX;
    long temp_latency = 0;
    for (int i = 0; i < sample_size; i++) {
        if (client.ping(temp_latency)) {
            if (temp_latency < latency) {
                latency = temp_latency;
            }
        } else {
            return false;
        }
    }
    return true;
}

