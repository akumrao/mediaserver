#define MS_CLASS "RTC::WebRtcTransport"
// #define MS_LOG_DEV_LEVEL 3

#include "RTC/WebRtcTransport.h"
#include "LoggerTag.h"
#include "base/error.h"
#include "Utils.h"
#include "Channel/Notifier.h"
#include <cmath> // std::pow()

namespace RTC
{
	/* Static. */

	static constexpr uint16_t IceCandidateDefaultLocalPriority{ 10000 };
	// We just provide "host" candidates so type preference is fixed.
	static constexpr uint16_t IceTypePreference{ 64 };
	// We do not support non rtcp-mux so component is always 1.
	static constexpr uint16_t IceComponent{ 1 };

	static inline uint32_t generateIceCandidatePriority(uint16_t localPreference)
	{
		MS_TRACE();

		return std::pow(2, 24) * IceTypePreference + std::pow(2, 8) * localPreference +
		       std::pow(2, 0) * (256 - IceComponent);
	}

	/* Instance methods. */

	WebRtcTransport::WebRtcTransport(const std::string& id, RTC::Transport::Listener* listener, json& data)
	  : RTC::Transport::Transport(id, listener, data)
	{
		MS_TRACE();

		bool enableUdp{ true };
		auto jsonEnableUdpIt = data.find("enableUdp");

		if (jsonEnableUdpIt != data.end())
		{
			if (!jsonEnableUdpIt->is_boolean())
				base::uv::throwError("wrong enableUdp (not a boolean)");

			enableUdp = jsonEnableUdpIt->get<bool>();
		}

		bool enableTcp{ false };
		auto jsonEnableTcpIt = data.find("enableTcp");

		if (jsonEnableTcpIt != data.end())
		{
			if (!jsonEnableTcpIt->is_boolean())
				base::uv::throwError("wrong enableTcp (not a boolean)");

			enableTcp = jsonEnableTcpIt->get<bool>();
		}

		bool preferUdp{ false };
		auto jsonPreferUdpIt = data.find("preferUdp");

		if (jsonPreferUdpIt != data.end())
		{
			if (!jsonPreferUdpIt->is_boolean())
				base::uv::throwError("wrong preferUdp (not a boolean)");

			preferUdp = jsonPreferUdpIt->get<bool>();
		}

		bool preferTcp{ false };
		auto jsonPreferTcpIt = data.find("preferTcp");

		if (jsonPreferTcpIt != data.end())
		{
			if (!jsonPreferTcpIt->is_boolean())
				base::uv::throwError("wrong preferTcp (not a boolean)");

			preferTcp = jsonPreferTcpIt->get<bool>();
		}

		auto jsonListenIpsIt = data.find("listenIps");

		if (jsonListenIpsIt == data.end())
			base::uv::throwError("missing listenIps");
		else if (!jsonListenIpsIt->is_array())
			base::uv::throwError("wrong listenIps (not an array)");
		else if (jsonListenIpsIt->empty())
			base::uv::throwError("wrong listenIps (empty array)");
		else if (jsonListenIpsIt->size() > 8)
			base::uv::throwError("wrong listenIps (too many IPs)");

		std::vector<ListenIp> listenIps(jsonListenIpsIt->size());

		for (size_t i{ 0 }; i < jsonListenIpsIt->size(); ++i)
		{
			auto& jsonListenIp = (*jsonListenIpsIt)[i];
			auto& listenIp     = listenIps[i];

			if (!jsonListenIp.is_object())
				base::uv::throwError("wrong listenIp (not an object)");

			auto jsonIpIt = jsonListenIp.find("ip");

			if (jsonIpIt == jsonListenIp.end())
				base::uv::throwError("missing listenIp.ip");
			else if (!jsonIpIt->is_string())
				base::uv::throwError("wrong listenIp.ip (not an string");

			listenIp.ip.assign(jsonIpIt->get<std::string>());

			// This may throw.
			base::net::IP::NormalizeIp(listenIp.ip);

			auto jsonAnnouncedIpIt = jsonListenIp.find("announcedIp");

			if (jsonAnnouncedIpIt != jsonListenIp.end())
			{
				if (!jsonAnnouncedIpIt->is_string())
					base::uv::throwError("wrong listenIp.announcedIp (not an string)");

				listenIp.announcedIp.assign(jsonAnnouncedIpIt->get<std::string>());
			}
		}

		try
		{
			uint16_t iceLocalPreferenceDecrement{ 0 };

			if (enableUdp && enableTcp)
				this->iceCandidates.reserve(2 * jsonListenIpsIt->size());
			else
				this->iceCandidates.reserve(jsonListenIpsIt->size());

			for (auto& listenIp : listenIps)
			{
				if (enableUdp)
				{
					uint16_t iceLocalPreference =
					  IceCandidateDefaultLocalPriority - iceLocalPreferenceDecrement;

					if (preferUdp)
						iceLocalPreference += 1000;

					uint32_t icePriority = generateIceCandidatePriority(iceLocalPreference);

					// This may throw.
					auto* udpSocket = new RTC::UdpSocket(this, listenIp.ip);

					this->udpSockets[udpSocket] = listenIp.announcedIp;

					if (listenIp.announcedIp.empty())
						this->iceCandidates.emplace_back(udpSocket, icePriority);
					else
						this->iceCandidates.emplace_back(udpSocket, icePriority, listenIp.announcedIp);
				}

				if (enableTcp)
				{
					uint16_t iceLocalPreference =
					  IceCandidateDefaultLocalPriority - iceLocalPreferenceDecrement;

					if (preferTcp)
						iceLocalPreference += 1000;

					uint32_t icePriority = generateIceCandidatePriority(iceLocalPreference);

					// This may throw.
					auto* tcpServer = new RTC::TcpServer(this, listenIp.ip);

					this->tcpServers[tcpServer] = listenIp.announcedIp;

					if (listenIp.announcedIp.empty())
						this->iceCandidates.emplace_back(tcpServer, icePriority);
					else
						this->iceCandidates.emplace_back(tcpServer, icePriority, listenIp.announcedIp);
				}

				// Decrement initial ICE local preference for next IP.
				iceLocalPreferenceDecrement += 100;
			}

			// Create a ICE server.
            STrace << " Create a ICE server.";
			this->iceServer = new RTC::IceServer(
			this, Utils::Crypto::GetRandomString(16), Utils::Crypto::GetRandomString(32));

			// Create a DTLS transport.
            STrace << " Create a DTLS transport.";
			this->dtlsTransport = new RTC::DtlsTransport(this);
		}
		catch (const std::exception& error)
		{
			// Must delete everything since the destructor won't be called.

			delete this->dtlsTransport;
			this->dtlsTransport = nullptr;

			delete this->iceServer;
			this->iceServer = nullptr;

			for (auto& kv : this->udpSockets)
			{
				auto* udpSocket = kv.first;

				delete udpSocket;
			}
			this->udpSockets.clear();

			for (auto& kv : this->tcpServers)
			{
				auto* tcpServer = kv.first;

				delete tcpServer;
			}
			this->tcpServers.clear();

			this->iceCandidates.clear();

			throw;
		}
	}

	WebRtcTransport::~WebRtcTransport()
	{
		MS_TRACE();

		// Must delete the DTLS transport first since it will generate a DTLS alert
		// to be sent.
		delete this->dtlsTransport;
		this->dtlsTransport = nullptr;

		delete this->iceServer;
		this->iceServer = nullptr;

		for (auto& kv : this->udpSockets)
		{
			auto* udpSocket = kv.first;

			delete udpSocket;
		}
		this->udpSockets.clear();

		for (auto& kv : this->tcpServers)
		{
			auto* tcpServer = kv.first;

			delete tcpServer;
		}
		this->tcpServers.clear();

		this->iceCandidates.clear();

		delete this->srtpRecvSession;
		this->srtpRecvSession = nullptr;

		delete this->srtpSendSession;
		this->srtpSendSession = nullptr;
	}

	void WebRtcTransport::FillJson(json& jsonObject) const
	{
		MS_TRACE();

		assertm(this->iceServer, "no iceServer");
		assertm(this->dtlsTransport, "no dtlsTransport");

		// Call the parent method.
		RTC::Transport::FillJson(jsonObject);

		// Add iceRole (we are always "controlled").
		jsonObject["iceRole"] = "controlled";

		// Add iceParameters.
		jsonObject["iceParameters"] = json::object();
		auto jsonIceParametersIt    = jsonObject.find("iceParameters");

		(*jsonIceParametersIt)["usernameFragment"] = this->iceServer->GetUsernameFragment();
		(*jsonIceParametersIt)["password"]         = this->iceServer->GetPassword();
		(*jsonIceParametersIt)["iceLite"]          = true;

		// Add iceCandidates.
		jsonObject["iceCandidates"] = json::array();
		auto jsonIceCandidatesIt    = jsonObject.find("iceCandidates");

		for (size_t i{ 0 }; i < this->iceCandidates.size(); ++i)
		{
			jsonIceCandidatesIt->emplace_back(json::value_t::object);

			auto& jsonEntry    = (*jsonIceCandidatesIt)[i];
			auto& iceCandidate = this->iceCandidates[i];

			iceCandidate.FillJson(jsonEntry);
		}

		// Add iceState.
		switch (this->iceServer->GetState())
		{
			case RTC::IceServer::IceState::NEW:
				jsonObject["iceState"] = "new";
				break;
			case RTC::IceServer::IceState::CONNECTED:
				jsonObject["iceState"] = "connected";
				break;
			case RTC::IceServer::IceState::COMPLETED:
				jsonObject["iceState"] = "completed";
				break;
			case RTC::IceServer::IceState::DISCONNECTED:
				jsonObject["iceState"] = "disconnected";
				break;
		}

		// Add iceSelectedTuple.
		if (this->iceServer->GetSelectedTuple())
			this->iceServer->GetSelectedTuple()->FillJson(jsonObject["iceSelectedTuple"]);

		// Add dtlsParameters.
		jsonObject["dtlsParameters"] = json::object();
		auto jsonDtlsParametersIt    = jsonObject.find("dtlsParameters");

		// Add dtlsParameters.fingerprints.
		(*jsonDtlsParametersIt)["fingerprints"] = json::array();
		auto jsonDtlsParametersFingerprintsIt   = jsonDtlsParametersIt->find("fingerprints");
		auto& fingerprints                      = this->dtlsTransport->GetLocalFingerprints();

		for (size_t i{ 0 }; i < fingerprints.size(); ++i)
		{
			jsonDtlsParametersFingerprintsIt->emplace_back(json::value_t::object);

			auto& jsonEntry   = (*jsonDtlsParametersFingerprintsIt)[i];
			auto& fingerprint = fingerprints[i];

			jsonEntry["algorithm"] =
			  RTC::DtlsTransport::GetFingerprintAlgorithmString(fingerprint.algorithm);
			jsonEntry["value"] = fingerprint.value;
		}

		// Add dtlsParameters.role.
		switch (this->dtlsRole)
		{
			case RTC::DtlsTransport::Role::NONE:
				(*jsonDtlsParametersIt)["role"] = "none";
				break;
			case RTC::DtlsTransport::Role::AUTO:
				(*jsonDtlsParametersIt)["role"] = "auto";
				break;
			case RTC::DtlsTransport::Role::CLIENT:
				(*jsonDtlsParametersIt)["role"] = "client";
				break;
			case RTC::DtlsTransport::Role::SERVER:
				(*jsonDtlsParametersIt)["role"] = "server";
				break;
		}

		// Add dtlsState.
		switch (this->dtlsTransport->GetState())
		{
			case RTC::DtlsTransport::DtlsState::NEW:
				jsonObject["dtlsState"] = "new";
				break;
			case RTC::DtlsTransport::DtlsState::CONNECTING:
				jsonObject["dtlsState"] = "connecting";
				break;
			case RTC::DtlsTransport::DtlsState::CONNECTED:
				jsonObject["dtlsState"] = "connected";
				break;
			case RTC::DtlsTransport::DtlsState::FAILED:
				jsonObject["dtlsState"] = "failed";
				break;
			case RTC::DtlsTransport::DtlsState::CLOSED:
				jsonObject["dtlsState"] = "closed";
				break;
		}
	}

	void WebRtcTransport::FillJsonStats(json& jsonArray)
	{
		MS_TRACE();

		assertm(this->iceServer, "no iceServer");
		assertm(this->dtlsTransport, "no dtlsTransport");

		// Call the parent method.
		RTC::Transport::FillJsonStats(jsonArray);

		auto& jsonObject = jsonArray[0];

		// Add type.
		jsonObject["type"] = "webrtc-transport";

		// Add iceRole (we are always "controlled").
		jsonObject["iceRole"] = "controlled";

		// Add iceState.
		switch (this->iceServer->GetState())
		{
			case RTC::IceServer::IceState::NEW:
				jsonObject["iceState"] = "new";
				break;
			case RTC::IceServer::IceState::CONNECTED:
				jsonObject["iceState"] = "connected";
				break;
			case RTC::IceServer::IceState::COMPLETED:
				jsonObject["iceState"] = "completed";
				break;
			case RTC::IceServer::IceState::DISCONNECTED:
				jsonObject["iceState"] = "disconnected";
				break;
		}

		if (this->iceServer->GetSelectedTuple())
		{
			// Add iceSelectedTuple.
			this->iceServer->GetSelectedTuple()->FillJson(jsonObject["iceSelectedTuple"]);
		}

		// Add dtlsState.
		switch (this->dtlsTransport->GetState())
		{
			case RTC::DtlsTransport::DtlsState::NEW:
				jsonObject["dtlsState"] = "new";
				break;
			case RTC::DtlsTransport::DtlsState::CONNECTING:
				jsonObject["dtlsState"] = "connecting";
				break;
			case RTC::DtlsTransport::DtlsState::CONNECTED:
				jsonObject["dtlsState"] = "connected";
				break;
			case RTC::DtlsTransport::DtlsState::FAILED:
				jsonObject["dtlsState"] = "failed";
				break;
			case RTC::DtlsTransport::DtlsState::CLOSED:
				jsonObject["dtlsState"] = "closed";
				break;
		}
	}

	void WebRtcTransport::HandleRequest(Channel::Request* request)
	{
		MS_TRACE();

		switch (request->methodId)
		{
			case Channel::Request::MethodId::TRANSPORT_CONNECT:
			{
				// Ensure this method is not called twice.
				if (this->connectCalled)
					base::uv::throwError("connect() already called");

				RTC::DtlsTransport::Fingerprint dtlsRemoteFingerprint;
				RTC::DtlsTransport::Role dtlsRemoteRole;

				auto jsonDtlsParametersIt = request->data.find("dtlsParameters");

				if (jsonDtlsParametersIt == request->data.end() || !jsonDtlsParametersIt->is_object())
					base::uv::throwError("missing dtlsParameters");

				auto jsonFingerprintsIt = jsonDtlsParametersIt->find("fingerprints");

				if (jsonFingerprintsIt == jsonDtlsParametersIt->end() || !jsonFingerprintsIt->is_array())
					base::uv::throwError("missing dtlsParameters.fingerprints");

				// NOTE: Just take the first fingerprint.
				for (auto& jsonFingerprint : *jsonFingerprintsIt)
				{
					if (!jsonFingerprint.is_object())
						base::uv::throwError("wrong entry in dtlsParameters.fingerprints (not an object)");

					auto jsonAlgorithmIt = jsonFingerprint.find("algorithm");

					if (jsonAlgorithmIt == jsonFingerprint.end())
						base::uv::throwError("missing fingerprint.algorithm");
					else if (!jsonAlgorithmIt->is_string())
						base::uv::throwError("wrong fingerprint.algorithm (not a string)");

					dtlsRemoteFingerprint.algorithm =
					  RTC::DtlsTransport::GetFingerprintAlgorithm(jsonAlgorithmIt->get<std::string>());

					if (dtlsRemoteFingerprint.algorithm == RTC::DtlsTransport::FingerprintAlgorithm::NONE)
						base::uv::throwError("invalid fingerprint.algorithm value");

					auto jsonValueIt = jsonFingerprint.find("value");

					if (jsonValueIt == jsonFingerprint.end())
						base::uv::throwError("missing fingerprint.value");
					else if (!jsonValueIt->is_string())
						base::uv::throwError("wrong fingerprint.value (not a string)");

					dtlsRemoteFingerprint.value = jsonValueIt->get<std::string>();

					// Just use the first fingerprint.
					break;
				}

				auto jsonRoleIt = jsonDtlsParametersIt->find("role");

				if (jsonRoleIt != jsonDtlsParametersIt->end())
				{
					if (!jsonRoleIt->is_string())
						base::uv::throwError("wrong dtlsParameters.role (not a string)");

					dtlsRemoteRole = RTC::DtlsTransport::StringToRole(jsonRoleIt->get<std::string>());

					if (dtlsRemoteRole == RTC::DtlsTransport::Role::NONE)
						base::uv::throwError("invalid dtlsParameters.role value");
				}
				else
				{
					dtlsRemoteRole = RTC::DtlsTransport::Role::AUTO;
				}

				// Set local DTLS role.
				switch (dtlsRemoteRole)
				{
					case RTC::DtlsTransport::Role::CLIENT:
					{
						this->dtlsRole = RTC::DtlsTransport::Role::SERVER;

						break;
					}
					case RTC::DtlsTransport::Role::SERVER:
					{
						this->dtlsRole = RTC::DtlsTransport::Role::CLIENT;

						break;
					}
					// If the peer has role "auto" we become "client" since we are ICE controlled.
					case RTC::DtlsTransport::Role::AUTO:
					{
						this->dtlsRole = RTC::DtlsTransport::Role::CLIENT;

						break;
					}
					case RTC::DtlsTransport::Role::NONE:
					{
						base::uv::throwError("invalid remote DTLS role");
					}
				}

				this->connectCalled = true;

				// Pass the remote fingerprint to the DTLS transport.
				if (this->dtlsTransport->SetRemoteFingerprint(dtlsRemoteFingerprint))
				{
					// If everything is fine, we may run the DTLS transport if ready.
					MayRunDtlsTransport();
				}

				// Tell the caller about the selected local DTLS role.
				json data = json::object();

				switch (this->dtlsRole)
				{
					case RTC::DtlsTransport::Role::CLIENT:
						data["dtlsLocalRole"] = "client";
						break;

					case RTC::DtlsTransport::Role::SERVER:
						data["dtlsLocalRole"] = "server";
						break;

					default:
						MS_ABORT("invalid local DTLS role");
				}

				request->Accept(data);

				break;
			}

			case Channel::Request::MethodId::TRANSPORT_RESTART_ICE:
			{
				std::string usernameFragment = Utils::Crypto::GetRandomString(16);
				std::string password         = Utils::Crypto::GetRandomString(32);

				this->iceServer->SetUsernameFragment(usernameFragment);
				this->iceServer->SetPassword(password);

				MS_DEBUG_DEV(
				  "WebRtcTransport ICE usernameFragment and password changed [id:%s]", this->id.c_str());

				// Reply with the updated ICE local parameters.
				json data = json::object();

				data["iceParameters"]    = json::object();
				auto jsonIceParametersIt = data.find("iceParameters");

				(*jsonIceParametersIt)["usernameFragment"] = this->iceServer->GetUsernameFragment();
				(*jsonIceParametersIt)["password"]         = this->iceServer->GetPassword();
				(*jsonIceParametersIt)["iceLite"]          = true;

				request->Accept(data);

				break;
			}

			default:
			{
				// Pass it to the parent class.
				RTC::Transport::HandleRequest(request);
			}
		}
	}

	inline bool WebRtcTransport::IsConnected() const
	{
		MS_TRACE();

		assertm(this->iceServer, "no iceServer");
		assertm(this->dtlsTransport, "no dtlsTransport");

		
		return (
			(
				this->iceServer->GetState() == RTC::IceServer::IceState::CONNECTED ||
				this->iceServer->GetState() == RTC::IceServer::IceState::COMPLETED
			) &&
			this->dtlsTransport->GetState() == RTC::DtlsTransport::DtlsState::CONNECTED
		);
		
	}

	void WebRtcTransport::MayRunDtlsTransport()
	{
		MS_TRACE();

		assertm(this->iceServer, "no iceServer");
		assertm(this->dtlsTransport, "no dtlsTransport");

		// Do nothing if we have the same local DTLS role as the DTLS transport.
		// NOTE: local role in DTLS transport can be NONE, but not ours.
		if (this->dtlsTransport->GetLocalRole() == this->dtlsRole)
			return;

		// Check our local DTLS role.
		switch (this->dtlsRole)
		{
			// If still 'auto' then transition to 'server' if ICE is 'connected' or
			// 'completed'.
			case RTC::DtlsTransport::Role::AUTO:
			{
				
				if (
					this->iceServer->GetState() == RTC::IceServer::IceState::CONNECTED ||
					this->iceServer->GetState() == RTC::IceServer::IceState::COMPLETED
				)
				
				{
					MS_DEBUG_TAG(
					  dtls, "transition from DTLS local role 'auto' to 'server' and running DTLS transport");

					this->dtlsRole = RTC::DtlsTransport::Role::SERVER;
					this->dtlsTransport->Run(RTC::DtlsTransport::Role::SERVER);
				}

				break;
			}

			// 'client' is only set if a 'connect' request was previously called with
			// remote DTLS role 'server'.
			//
			// If 'client' then wait for ICE to be 'completed' (got USE-CANDIDATE).
			//
			// NOTE: This is the theory, however let's be more flexible as told here:
			//   https://bugs.chromium.org/p/webrtc/issues/detail?id=3661
			case RTC::DtlsTransport::Role::CLIENT:
			{
				
				if (
					this->iceServer->GetState() == RTC::IceServer::IceState::CONNECTED ||
					this->iceServer->GetState() == RTC::IceServer::IceState::COMPLETED
				)
				
				{
					MS_DEBUG_TAG(dtls, "running DTLS transport in local role 'client'");

					this->dtlsTransport->Run(RTC::DtlsTransport::Role::CLIENT);
				}

				break;
			}

			// If 'server' then run the DTLS transport if ICE is 'connected' (not yet
			// USE-CANDIDATE) or 'completed'.
			case RTC::DtlsTransport::Role::SERVER:
			{
				
				if (
					this->iceServer->GetState() == RTC::IceServer::IceState::CONNECTED ||
					this->iceServer->GetState() == RTC::IceServer::IceState::COMPLETED
				)
				
				{
					MS_DEBUG_TAG(dtls, "running DTLS transport in local role 'server'");

					this->dtlsTransport->Run(RTC::DtlsTransport::Role::SERVER);
				}

				break;
			}

			case RTC::DtlsTransport::Role::NONE:
			{
				MS_ABORT("local DTLS role not set");
			}
		}
	}

	void WebRtcTransport::SendRtpPacket(RTC::RtpPacket* packet, RTC::Transport::onSendCallback* cb)
	{
		MS_TRACE();

		if (!IsConnected())
		{
			if (cb)
			{
				(*cb)(false);

				delete cb;
			}

			return;
		}

		// Ensure there is sending SRTP session.
		if (!this->srtpSendSession)
		{
			MS_WARN_DEV("ignoring RTP packet due to non sending SRTP session");

			if (cb)
			{
				(*cb)(false);

				delete cb;
			}

			return;
		}

		const uint8_t* data = packet->GetData();
		size_t len          = packet->GetSize();

		if (!this->srtpSendSession->EncryptRtp(&data, &len))
		{
			if (cb)
			{
				(*cb)(false);

				delete cb;
			}

			return;
		}

		this->iceServer->GetSelectedTuple()->Send(data, len, cb);

		// Increase send transmission.
		RTC::Transport::DataSent(len);
	}

	void WebRtcTransport::SendRtcpPacket(RTC::RTCP::Packet* packet)
	{
		MS_TRACE();

		if (!IsConnected())
			return;

		const uint8_t* data = packet->GetData();
		size_t len          = packet->GetSize();

		// Ensure there is sending SRTP session.
		if (!this->srtpSendSession)
		{
			MS_WARN_DEV("ignoring RTCP packet due to non sending SRTP session");

			return;
		}

		if (!this->srtpSendSession->EncryptRtcp(&data, &len))
			return;

		this->iceServer->GetSelectedTuple()->Send(data, len);

		// Increase send transmission.
		RTC::Transport::DataSent(len);
	}

	void WebRtcTransport::SendRtcpCompoundPacket(RTC::RTCP::CompoundPacket* packet)
	{
		MS_TRACE();

		if (!IsConnected())
			return;

		const uint8_t* data = packet->GetData();
		size_t len          = packet->GetSize();

		// Ensure there is sending SRTP session.
		if (!this->srtpSendSession)
		{
			MS_WARN_TAG(rtcp, "ignoring RTCP compound packet due to non sending SRTP session");

			return;
		}

		if (!this->srtpSendSession->EncryptRtcp(&data, &len))
			return;

		this->iceServer->GetSelectedTuple()->Send(data, len);

		// Increase send transmission.
		RTC::Transport::DataSent(len);
	}

	void WebRtcTransport::SendSctpData(const uint8_t* data, size_t len)
	{
		MS_TRACE();

		
		if (!IsConnected())
		{
			MS_WARN_TAG(sctp, "DTLS not connected, cannot send SCTP data");

			return;
		}

		this->dtlsTransport->SendApplicationData(data, len);
	}

	inline void WebRtcTransport::OnPacketReceived(
	  RTC::TransportTuple* tuple, const uint8_t* data, size_t len)
	{
		MS_TRACE();

		assertm(this->iceServer, "no iceServer");
		assertm(this->dtlsTransport, "no dtlsTransport");

		// Increase receive transmission.
		RTC::Transport::DataReceived(len);

		// Check if it's STUN.
		if (RTC::StunPacket::IsStun(data, len))
		{
			OnStunDataReceived(tuple, data, len);
		}
		// Check if it's RTCP.
		else if (RTC::RTCP::Packet::IsRtcp(data, len))
		{
			OnRtcpDataReceived(tuple, data, len);
		}
		// Check if it's RTP.
		else if (RTC::RtpPacket::IsRtp(data, len))
		{
			OnRtpDataReceived(tuple, data, len);
		}
		// Check if it's DTLS.
		else if (RTC::DtlsTransport::IsDtls(data, len))
		{
			OnDtlsDataReceived(tuple, data, len);
		}
		else
		{
			MS_WARN_DEV("ignoring received packet of unknown type");
		}
	}

	inline void WebRtcTransport::OnStunDataReceived(
	  RTC::TransportTuple* tuple, const uint8_t* data, size_t len)
	{
		MS_TRACE();

		assertm(this->iceServer, "no iceServer");

		RTC::StunPacket* packet = RTC::StunPacket::Parse(data, len);

		if (!packet)
		{
			MS_WARN_DEV("ignoring wrong STUN packet received");

			return;
		}

		// Pass it to the IceServer.
		this->iceServer->ProcessStunPacket(packet, tuple);

		delete packet;
	}

	inline void WebRtcTransport::OnDtlsDataReceived(
	  const RTC::TransportTuple* tuple, const uint8_t* data, size_t len)
	{
		MS_TRACE();

		assertm(this->iceServer, "no iceServer");
		assertm(this->dtlsTransport, "no dtlsTransport");

		// Ensure it comes from a valid tuple.
		if (!this->iceServer->IsValidTuple(tuple))
		{
			MS_WARN_TAG(dtls, "ignoring DTLS data coming from an invalid tuple");

			return;
		}

		// Trick for clients performing aggressive ICE regardless we are ICE-Lite.
		this->iceServer->ForceSelectedTuple(tuple);

		// Check that DTLS status is 'connecting' or 'connected'.
		if (
		  this->dtlsTransport->GetState() == RTC::DtlsTransport::DtlsState::CONNECTING ||
		  this->dtlsTransport->GetState() == RTC::DtlsTransport::DtlsState::CONNECTED)
		{
			MS_DEBUG_DEV("DTLS data received, passing it to the DTLS transport");

			this->dtlsTransport->ProcessDtlsData(data, len);
		}
		else
		{
			MS_WARN_TAG(dtls, "Transport is not 'connecting' or 'connected', ignoring received DTLS data");

			return;
		}
	}

	inline void WebRtcTransport::OnRtpDataReceived(
	  RTC::TransportTuple* tuple, const uint8_t* data, size_t len)
	{
		MS_TRACE();

		assertm(this->iceServer, "no iceServer");
		assertm(this->dtlsTransport, "no dtlsTransport");

		// Ensure DTLS is connected.
		if (this->dtlsTransport->GetState() != RTC::DtlsTransport::DtlsState::CONNECTED)
		{
			MS_DEBUG_2TAGS(dtls, rtp, "ignoring RTP packet while DTLS not connected");

			return;
		}

		// Ensure there is receiving SRTP session.
		if (!this->srtpRecvSession)
		{
			MS_DEBUG_TAG(srtp, "ignoring RTP packet due to non receiving SRTP session");

			return;
		}

		// Ensure it comes from a valid tuple.
		if (!this->iceServer->IsValidTuple(tuple))
		{
			MS_WARN_TAG(rtp, "ignoring RTP packet coming from an invalid tuple");

			return;
		}

		// Decrypt the SRTP packet.
		if (!this->srtpRecvSession->DecryptSrtp(const_cast<uint8_t*>(data), &len))
		{
			RTC::RtpPacket* packet = RTC::RtpPacket::Parse(data, len);

			if (!packet)
			{
				MS_WARN_TAG(srtp, "DecryptSrtp() failed due to an invalid RTP packet");
			}
			else
			{
				MS_WARN_TAG(
				  srtp,
				  "DecryptSrtp() failed [ssrc:%" PRIu32 ", payloadType:%" PRIu8 ", seq:%" PRIu16 "]",
				  packet->GetSsrc(),
				  packet->GetPayloadType(),
				  packet->GetSequenceNumber());

				delete packet;
			}

			return;
		}

		RTC::RtpPacket* packet = RTC::RtpPacket::Parse(data, len);

		if (!packet)
		{
			MS_WARN_TAG(rtp, "received data is not a valid RTP packet");

			return;
		}

		// Trick for clients performing aggressive ICE regardless we are ICE-Lite.
		this->iceServer->ForceSelectedTuple(tuple);

		// Pass the packet to the parent transport.
		RTC::Transport::ReceiveRtpPacket(packet);
	}

	inline void WebRtcTransport::OnRtcpDataReceived(
	  RTC::TransportTuple* tuple, const uint8_t* data, size_t len)
	{
		MS_TRACE();

		assertm(this->iceServer, "no iceServer");
		assertm(this->dtlsTransport, "no dtlsTransport");

		// Ensure DTLS is connected.
		if (this->dtlsTransport->GetState() != RTC::DtlsTransport::DtlsState::CONNECTED)
		{
			MS_DEBUG_2TAGS(dtls, rtcp, "ignoring RTCP packet while DTLS not connected");

			return;
		}

		// Ensure there is receiving SRTP session.
		if (!this->srtpRecvSession)
		{
			MS_DEBUG_TAG(srtp, "ignoring RTCP packet due to non receiving SRTP session");

			return;
		}

		// Ensure it comes from a valid tuple.
		if (!this->iceServer->IsValidTuple(tuple))
		{
			MS_WARN_TAG(rtcp, "ignoring RTCP packet coming from an invalid tuple");

			return;
		}

		// Decrypt the SRTCP packet.
		if (!this->srtpRecvSession->DecryptSrtcp(const_cast<uint8_t*>(data), &len))
			return;

		RTC::RTCP::Packet* packet = RTC::RTCP::Packet::Parse(data, len);

		if (!packet)
		{
			MS_WARN_TAG(rtcp, "received data is not a valid RTCP compound or single packet");

			return;
		}

		// Pass the packet to the parent transport.
		RTC::Transport::ReceiveRtcpPacket(packet);
	}

	inline void WebRtcTransport::OnUdpSocketPacketReceived(
	  RTC::UdpSocket* socket, const uint8_t* data, size_t len, const struct sockaddr* remoteAddr)
	{
		MS_TRACE();

		RTC::TransportTuple tuple(socket, remoteAddr);

		OnPacketReceived(&tuple, data, len);
	}

       inline void WebRtcTransport::on_close(base::net::Listener* conn)
	{
		RTC::TcpConnection* connection = (RTC::TcpConnection*) conn;

		RTC::TransportTuple tuple(connection);

		this->iceServer->RemoveTuple(&tuple);
	}


        void WebRtcTransport::on_read(base::net::Listener* conn, const char* data, size_t len) {

            RTC::TcpConnection* connection = (RTC::TcpConnection*) conn;
            RTC::TransportTuple tuple(connection);

            OnPacketReceived(&tuple, (const uint8_t*)data, len);

        }

	inline void WebRtcTransport::OnIceServerSendStunPacket(
	  const RTC::IceServer* /*iceServer*/, const RTC::StunPacket* packet, RTC::TransportTuple* tuple)
	{
		MS_TRACE();

		// Send the STUN response over the same transport tuple.
		tuple->Send(packet->GetData(), packet->GetSize());

		// Increase send transmission.
		RTC::Transport::DataSent(packet->GetSize());
	}

	inline void WebRtcTransport::OnIceServerSelectedTuple(
	  const RTC::IceServer* /*iceServer*/, RTC::TransportTuple* /*tuple*/)
	{
		MS_TRACE();

		assertm(this->iceServer, "no iceServer");

		/*
		 * RFC 5245 section 11.2 "Receiving Media":
		 *
		 * ICE implementations MUST be prepared to receive media on each component
		 * on any candidates provided for that component.
		 */

		MS_DEBUG_TAG(ice, "ICE selected tuple");

		// Notify the Node WebRtcTransport.
		json data = json::object();

		this->iceServer->GetSelectedTuple()->FillJson(data["iceSelectedTuple"]);

		Channel::Notifier::Emit(this->id, "iceselectedtuplechange", data);
	}

	inline void WebRtcTransport::OnIceServerConnected(const RTC::IceServer* /*iceServer*/)
	{
		MS_TRACE();

		assertm(this->iceServer, "no iceServer");

		MS_DEBUG_TAG(ice, "ICE connected");

		// Notify the Node WebRtcTransport.
		json data = json::object();

		data["iceState"] = "connected";

		Channel::Notifier::Emit(this->id, "icestatechange", data);

		// If ready, run the DTLS handler.
		MayRunDtlsTransport();

		// If DTLS was already connected, notify the parent class.
		if (this->dtlsTransport->GetState() == RTC::DtlsTransport::DtlsState::CONNECTED)
			RTC::Transport::Connected();
	}

	inline void WebRtcTransport::OnIceServerCompleted(const RTC::IceServer* /*iceServer*/)
	{
		MS_TRACE();

		assertm(this->iceServer, "no iceServer");

		MS_DEBUG_TAG(ice, "ICE completed");

		// Notify the Node WebRtcTransport.
		json data = json::object();

		data["iceState"] = "completed";

		Channel::Notifier::Emit(this->id, "icestatechange", data);

		// If ready, run the DTLS handler.
		MayRunDtlsTransport();

		// If DTLS was already connected, notify the parent class.
		if (this->dtlsTransport->GetState() == RTC::DtlsTransport::DtlsState::CONNECTED)
			RTC::Transport::Connected();
	}

	inline void WebRtcTransport::OnIceServerDisconnected(const RTC::IceServer* /*iceServer*/)
	{
		MS_TRACE();

		assertm(this->iceServer, "no iceServer");

		MS_DEBUG_TAG(ice, "ICE disconnected");

		// Notify the Node WebRtcTransport.
		json data = json::object();

		data["iceState"] = "disconnected";

		Channel::Notifier::Emit(this->id, "icestatechange", data);

		// If DTLS was already connected, notify the parent class.
		if (this->dtlsTransport->GetState() == RTC::DtlsTransport::DtlsState::CONNECTED)
			RTC::Transport::Disconnected();
	}

	inline void WebRtcTransport::OnDtlsTransportConnecting(const RTC::DtlsTransport* /*dtlsTransport*/)
	{
		MS_TRACE();

		assertm(this->iceServer, "no iceServer");
		assertm(this->dtlsTransport, "no dtlsTransport");

		MS_DEBUG_TAG(dtls, "DTLS connecting");

		// Notify the Node WebRtcTransport.
		json data = json::object();

		data["dtlsState"] = "connecting";

		Channel::Notifier::Emit(this->id, "dtlsstatechange", data);
	}

	inline void WebRtcTransport::OnDtlsTransportConnected(
	  const RTC::DtlsTransport* /*dtlsTransport*/,
	  RTC::SrtpSession::Profile srtpProfile,
	  uint8_t* srtpLocalKey,
	  size_t srtpLocalKeyLen,
	  uint8_t* srtpRemoteKey,
	  size_t srtpRemoteKeyLen,
	  std::string& remoteCert)
	{
		MS_TRACE();

		assertm(this->iceServer, "no iceServer");
		assertm(this->dtlsTransport, "no dtlsTransport");

		MS_DEBUG_TAG(dtls, "DTLS connected");

		// Close it if it was already set and update it.
		if (this->srtpSendSession)
		{
			delete this->srtpSendSession;
			this->srtpSendSession = nullptr;
		}
		if (this->srtpRecvSession)
		{
			delete this->srtpRecvSession;
			this->srtpRecvSession = nullptr;
		}

		try
		{
			this->srtpSendSession = new RTC::SrtpSession(
			  RTC::SrtpSession::Type::OUTBOUND, srtpProfile, srtpLocalKey, srtpLocalKeyLen);
		}
		catch (const std::exception& error)
		{
			MS_ERROR("error creating SRTP sending session: %s", error.what());
		}

		try
		{
			this->srtpRecvSession = new RTC::SrtpSession(
			  RTC::SrtpSession::Type::INBOUND, srtpProfile, srtpRemoteKey, srtpRemoteKeyLen);
		}
		catch (const std::exception& error)
		{
			MS_ERROR("error creating SRTP receiving session: %s", error.what());

			delete this->srtpSendSession;
			this->srtpSendSession = nullptr;
		}

		// Notify the Node WebRtcTransport.
		json data = json::object();

		data["dtlsState"]      = "connected";
		data["dtlsRemoteCert"] = remoteCert;

		Channel::Notifier::Emit(this->id, "dtlsstatechange", data);

		// Tell the parent class.
		RTC::Transport::Connected();
	}

	inline void WebRtcTransport::OnDtlsTransportFailed(const RTC::DtlsTransport* /*dtlsTransport*/)
	{
		MS_TRACE();

		assertm(this->iceServer, "no iceServer");
		assertm(this->dtlsTransport, "no dtlsTransport");

		MS_WARN_TAG(dtls, "DTLS failed");

		// Notify the Node WebRtcTransport.
		json data = json::object();

		data["dtlsState"] = "failed";

		Channel::Notifier::Emit(this->id, "dtlsstatechange", data);
	}

	inline void WebRtcTransport::OnDtlsTransportClosed(const RTC::DtlsTransport* /*dtlsTransport*/)
	{
		MS_TRACE();

		assertm(this->iceServer, "no iceServer");
		assertm(this->dtlsTransport, "no dtlsTransport");

		MS_WARN_TAG(dtls, "DTLS remotely closed");

		// Notify the Node WebRtcTransport.
		json data = json::object();

		data["dtlsState"] = "closed";

		Channel::Notifier::Emit(this->id, "dtlsstatechange", data);

		// Tell the parent class.
		RTC::Transport::Disconnected();
	}

	inline void WebRtcTransport::OnDtlsTransportSendData(
	  const RTC::DtlsTransport* /*dtlsTransport*/, const uint8_t* data, size_t len)
	{
		MS_TRACE();

		assertm(this->iceServer, "no iceServer");
		assertm(this->dtlsTransport, "no dtlsTransport");

		assertm(this->iceServer, "no iceServer");

		if (!this->iceServer->GetSelectedTuple())
		{
			MS_WARN_TAG(dtls, "no selected tuple set, cannot send DTLS packet");

			return;
		}

		this->iceServer->GetSelectedTuple()->Send(data, len);

		// Increase send transmission.
		RTC::Transport::DataSent(len);
	}

	inline void WebRtcTransport::OnDtlsTransportApplicationDataReceived(
	  const RTC::DtlsTransport* /*dtlsTransport*/, const uint8_t* data, size_t len)
	{
		MS_TRACE();

		assertm(this->iceServer, "no iceServer");
		assertm(this->dtlsTransport, "no dtlsTransport");

		// Pass it to the parent transport.
		RTC::Transport::ReceiveSctpData(data, len);
	}
} // namespace RTC
