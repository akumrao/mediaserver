#ifndef framefilter_HEADER_GUARD
#define framefilter_HEADER_GUARD

#include <chrono>
#include <cstdint>

inline uint64_t CurrentTime_milliseconds()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::high_resolution_clock::now().time_since_epoch())
        .count();
}

inline uint64_t CurrentTime_microseconds()
{
    return std::chrono::duration_cast<std::chrono::microseconds>(
               std::chrono::high_resolution_clock::now().time_since_epoch())
        .count();
}

inline uint64_t CurrentTime_nanoseconds()
{
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
               std::chrono::high_resolution_clock::now().time_since_epoch())
        .count();
}

enum en_EncType
{
    EN_NONE = -1,
    EN_X264 = 0,
    EN_NVIDIA,
    EN_QUICKSYNC,
    EN_VAAPI,
    EN_VP8,
    EN_NATIVE,
};

struct st_Cam
{
    std::string camid;
    std::string start;
    std::string end;
    int width{0};
    int height{0};
    int scale{1};
    int speed{1};

    std::string encoder;

    en_EncType encType;  // 0 hw , 1 s/w , 2 native

    std::string getTrackId()
    {
        if (!trackid.size())
            trackid = camid + "_" + start + "_" + end + "_" + std::to_string(width) + std::to_string(height)
                    + std::to_string(scale) + std::to_string(speed);
        return trackid;
    }

    st_Cam() : encType(EN_NVIDIA) {}

private:
    std::string trackid;
};

#endif
