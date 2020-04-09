#define MS_CLASS "SignalsHandler"
// #define MS_LOG_DEV_LEVEL 3

#include "handles/SignalsHandler.h"
#include "base/application.h"
#include "LoggerTag.h"
#include "base/error.h"

/* Static methods for UV callbacks. */

inline static void onSignal(uv_signal_t* handle, int signum)
{
	static_cast<SignalsHandler*>(handle->data)->OnUvSignal(signum);
}

inline static void onClose(uv_handle_t* handle)
{
	delete handle;
}

/* Instance methods. */

SignalsHandler::SignalsHandler(Listener* listener) : listener(listener)
{
	MS_TRACE();
}

SignalsHandler::~SignalsHandler()
{
	MS_TRACE();

	if (!this->closed)
		Close();
}

void SignalsHandler::Close()
{
	MS_TRACE();

	if (this->closed)
		return;

	this->closed = true;

	for (auto* uvHandle : this->uvHandles)
	{
		uv_close(reinterpret_cast<uv_handle_t*>(uvHandle), static_cast<uv_close_cb>(onClose));
	}
}

void SignalsHandler::AddSignal(int signum, const std::string& name)
{
	MS_TRACE();

	if (this->closed)
		base::uv::throwError("closed");

	int err;
	auto uvHandle = new uv_signal_t;

	uvHandle->data = static_cast<void*>(this);

	err = uv_signal_init(base::Application::uvGetLoop(), uvHandle);

	if (err != 0)
	{
		delete uvHandle;

		base::uv::throwError("uv_signal_init() failed for signal " + name, err);
	}

	err = uv_signal_start(uvHandle, static_cast<uv_signal_cb>(onSignal), signum);

	if (err != 0)
		base::uv::throwError("uv_signal_start() failed for signal " + name,  err);

	// Enter the UV handle into the vector.
	this->uvHandles.push_back(uvHandle);
}

inline void SignalsHandler::OnUvSignal(int signum)
{
	MS_TRACE();

	// Notify the listener.
	this->listener->OnSignal(this, signum);
}
