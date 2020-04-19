/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#ifndef PORT_MANAGER_H
#define PORT_MANAGER_H

#include <json.hpp>
#include <uv.h>
#include <string>
#include <unordered_map>
#include <vector>
using json = nlohmann::json;
namespace base
{
namespace net
{
	class PortManager
	{
	private:
		enum class Transport : uint8_t
		{
			UDP = 1,
			TCP
		};

	public:
		static uv_udp_t* BindUdp(std::string& ip);
		static uv_tcp_t* BindTcp(std::string& ip);
		static void UnbindUdp(std::string& ip, uint16_t port);
		static void UnbindTcp(std::string& ip, uint16_t port);
		static void FillJson(json& jsonObject);

	private:
		static uv_handle_t* Bind(Transport transport, std::string& ip);
		static void Unbind(Transport transport, std::string& ip, uint16_t port);
		static std::vector<bool>& GetPorts(Transport transport, const std::string& ip);

	private:
		static std::unordered_map<std::string, std::vector<bool>> mapUdpIpPorts;
		static std::unordered_map<std::string, std::vector<bool>> mapTcpIpPorts;
	};

	/* Inline static methods. */

	inline uv_udp_t* PortManager::BindUdp(std::string& ip)
	{
            int foundPort;
		return reinterpret_cast<uv_udp_t*>(Bind(Transport::UDP, ip));
	}

	inline uv_tcp_t* PortManager::BindTcp(std::string& ip)
	{
		return reinterpret_cast<uv_tcp_t*>(Bind(Transport::TCP, ip));
	}

	inline void PortManager::UnbindUdp(std::string& ip, uint16_t port)
	{
		return Unbind(Transport::UDP, ip, port);
	}

	inline void PortManager::UnbindTcp(std::string& ip, uint16_t port)
	{
		return Unbind(Transport::TCP, ip, port);
	}
} // namespace net
}
#endif
