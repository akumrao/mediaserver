#ifndef MS_RTC_RTP_DICTIONARIES_HPP
#define MS_RTC_RTP_DICTIONARIES_HPP

#include "common.h"
#include "RTC/Parameters.h"
#include <json.hpp>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

using json = nlohmann::json;

namespace RTC
{
	class Media
	{
	public:
		enum class Kind : uint8_t
		{
			ALL = 0,
			AUDIO,
			VIDEO
		};

	public:
		static Kind GetKind(std::string& str);
		static Kind GetKind(std::string&& str);
		static const std::string& GetString(Kind kind);

	private:
		static std::unordered_map<std::string, Kind> string2Kind;
		static std::map<Kind, std::string> kind2String;
	};

	class RtpCodecMimeType
	{
	public:
		enum class Type : uint8_t
		{
			UNSET = 0,
			AUDIO,
			VIDEO
		};

	public:
		enum class Subtype : uint16_t
		{
			UNSET = 0,
			// Audio codecs:
			OPUS = 100,
			PCMA,
			PCMU,
			ISAC,
			G722,
			ILBC,
			SILK,
			// Video codecs:
			VP8 = 200,
			VP9,
			H264,
			X_H264UC,
			H265,
                        multiplex,
			// Complementary codecs:
			CN = 300,
			TELEPHONE_EVENT,
			// Feature codecs:
			RTX = 400,
			ULPFEC,
			X_ULPFECUC,
			FLEXFEC,
			RED
		};

	public:
		static std::unordered_map<std::string, Type> string2Type;
		static std::map<Type, std::string> type2String;
		static std::unordered_map<std::string, Subtype> string2Subtype;
		static std::map<Subtype, std::string> subtype2String;

	public:
		RtpCodecMimeType() = default;

		bool operator==(const RtpCodecMimeType& other) const
		{
			return this->type == other.type && this->subtype == other.subtype;
		}

		bool operator!=(const RtpCodecMimeType& other) const
		{
			return !(*this == other);
		}

		void SetMimeType(const std::string& mimeType);

		void UpdateMimeType();

		const std::string& ToString() const
		{
			return this->mimeType;
		}

		bool IsMediaCodec() const
		{
			return this->subtype >= Subtype(100) && this->subtype < (Subtype)300;
		}

		bool IsComplementaryCodec() const
		{
			return this->subtype >= Subtype(300) && this->subtype < (Subtype)400;
		}

		bool IsFeatureCodec() const
		{
			return this->subtype >= Subtype(400);
		}

	public:
		Type type{ Type::UNSET };
		Subtype subtype{ Subtype::UNSET };

	private:
		std::string mimeType;
	};

	class RtpHeaderExtensionUri
	{
	public:
		enum class Type : uint8_t
		{
			UNKNOWN                = 0,
			MID                    = 1,
			RTP_STREAM_ID          = 2,
			REPAIRED_RTP_STREAM_ID = 3,
			ABS_SEND_TIME          = 4,
			TRANSPORT_WIDE_CC_01   = 5,
			FRAME_MARKING_07       = 6, // NOTE: Remove once RFC.
			FRAME_MARKING          = 7,
			SSRC_AUDIO_LEVEL       = 10,
			VIDEO_ORIENTATION      = 11,
			TOFFSET                = 12
		};

	private:
		static std::unordered_map<std::string, Type> string2Type;

	public:
		static Type GetType(std::string& uri);
	};

	class RtcpFeedback
	{
	public:
		RtcpFeedback() = default;
		explicit RtcpFeedback(json& data);

		void FillJson(json& jsonObject) const;

	public:
		std::string type;
		std::string parameter;
	};

	class RtpCodecParameters
	{
	public:
		RtpCodecParameters() = default;
		explicit RtpCodecParameters(json& data);

		void FillJson(json& jsonObject) const;

	private:
		void CheckCodec();

	public:
		RtpCodecMimeType mimeType;
		uint8_t payloadType{ 0 };
		uint32_t clockRate{ 0 };
		uint8_t channels{ 1 };
		RTC::Parameters parameters;
		std::vector<RtcpFeedback> rtcpFeedback;
	};

	class RtpRtxParameters
	{
	public:
		RtpRtxParameters() = default;
		explicit RtpRtxParameters(json& data);

		void FillJson(json& jsonObject) const;

	public:
		uint32_t ssrc{ 0 };
	};

	class RtpEncodingParameters
	{
	public:
		RtpEncodingParameters() = default;
		explicit RtpEncodingParameters(json& data);

		void FillJson(json& jsonObject) const;

	public:
		uint32_t ssrc{ 0 };
		std::string rid;
		uint8_t codecPayloadType{ 0 };
		bool hasCodecPayloadType{ false };
		RtpRtxParameters rtx;
		bool hasRtx{ false };
		uint32_t maxBitrate{ 0 };
		double maxFramerate{ 0 };
		bool dtx{ false };
		std::string scalabilityMode;
		uint8_t spatialLayers{ 1 };
		uint8_t temporalLayers{ 1 };
		bool ksvc{ false };
	};

	class RtpHeaderExtensionParameters
	{
	public:
		RtpHeaderExtensionParameters() = default;
		explicit RtpHeaderExtensionParameters(json& data);

		void FillJson(json& jsonObject) const;

	public:
		std::string uri;
		RtpHeaderExtensionUri::Type type;
		uint8_t id{ 0 };
		bool encrypt{ false };
		RTC::Parameters parameters;
	};

	class RtcpParameters
	{
	public:
		RtcpParameters() = default;
		explicit RtcpParameters(json& data);

		void FillJson(json& jsonObject) const;

	public:
		std::string cname;
		uint32_t ssrc{ 0 };
		bool reducedSize{ true };
	};

	class RtpParameters
	{
	public:
		enum class Type : uint8_t
		{
			NONE = 0,
			SIMPLE,
			SIMULCAST,
			SVC,
			PIPE
		};

	public:
		static Type GetType(const RtpParameters& rtpParameters);
		static Type GetType(std::string& str);
		static Type GetType(std::string&& str);
		static std::string& GetTypeString(Type type);

	private:
		static std::unordered_map<std::string, Type> string2Type;
		static std::map<Type, std::string> type2String;

	public:
		RtpParameters() = default;
		explicit RtpParameters(json& data);

		void FillJson(json& jsonObject) const;
		const RTC::RtpCodecParameters* GetCodecForEncoding(RtpEncodingParameters& encoding) const;
		const RTC::RtpCodecParameters* GetRtxCodecForEncoding(RtpEncodingParameters& encoding) const;

	private:
		void ValidateCodecs();
		void ValidateEncodings();
		void SetType();

	public:
		std::string mid;
		std::vector<RtpCodecParameters> codecs;
		std::vector<RtpEncodingParameters> encodings;
		std::vector<RtpHeaderExtensionParameters> headerExtensions;
		RtcpParameters rtcp;
		bool hasRtcp{ false };
	};
} // namespace RTC

#endif
