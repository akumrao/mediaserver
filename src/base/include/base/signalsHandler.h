/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#ifndef SIGNALS_HANDLER_HP
#define SIGNALS_HANDLER_HP

#include <uv.h>
#include <string>
#include <vector>

namespace base
{

class SignalsHandler
{
public:
	class Listener
	{
	public:
		virtual ~Listener() = default;

	public:
		virtual void OnSignal(SignalsHandler* signalsHandler, int signum) = 0;
           
	};

public:
	explicit SignalsHandler(Listener* listener);
	~SignalsHandler();

public:
	void Close();
	void AddSignal(int signum, const std::string& name);

	/* Callbacks fired by UV events. */
public:
	void OnUvSignal(int signum);

private:
	// Passed by argument.
	Listener* listener{ nullptr };
	// Allocated by this.
	std::vector<uv_signal_t*> uvHandles;
	// Others.
	bool closed{ false };
};

}//end namespace base
#endif
