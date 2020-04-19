

#include <utility>

#include "LoggerTag.h"
#include "RTC/IceServer.h"

namespace RTC
{
	/* Static. */

	static constexpr size_t StunSerializeBufferSize{ 65536 };
	static uint8_t StunSerializeBuffer[StunSerializeBufferSize];

	/* Instance methods. */

	IceServer::IceServer(Listener* listener, const std::string& usernameFragment, const std::string& password)
	  : listener(listener), usernameFragment(usernameFragment), password(password)
	{
		
	}

	void IceServer::ProcessStunPacket(RTC::StunPacket* packet, RTC::TransportTuple* tuple)
	{

		// Must be a Binding method.
		if (packet->GetMethod() != RTC::StunPacket::Method::BINDING)
		{
			if (packet->GetClass() == RTC::StunPacket::Class::REQUEST)
			{
				MS_WARN_TAG(
				  ice,
				  "unknown method in STUN Request => 400 ",
				  static_cast<unsigned int>(packet->GetMethod()));

				// Reply 400.
				RTC::StunPacket* response = packet->CreateErrorResponse(400);

				response->Serialize(StunSerializeBuffer);
				this->listener->OnIceServerSendStunPacket(this, response, tuple);

				delete response;
			}
			else
			{
				MS_WARN_TAG(
				  ice,
				  "ignoring STUN Indication or Response with unknown method ",
				  static_cast<unsigned int>(packet->GetMethod()));
			}

			return;
		}

		// Must use FINGERPRINT (optional for ICE STUN indications).
		if (!packet->HasFingerprint() && packet->GetClass() != RTC::StunPacket::Class::INDICATION)
		{
			if (packet->GetClass() == RTC::StunPacket::Class::REQUEST)
			{
				MS_WARN_TAG(ice, "STUN Binding Request without FINGERPRINT => 400");

				// Reply 400.
				RTC::StunPacket* response = packet->CreateErrorResponse(400);

				response->Serialize(StunSerializeBuffer);
				this->listener->OnIceServerSendStunPacket(this, response, tuple);

				delete response;
			}
			else
			{
				MS_WARN_TAG(ice, "ignoring STUN Binding Response without FINGERPRINT");
			}

			return;
		}

		switch (packet->GetClass())
		{
			case RTC::StunPacket::Class::REQUEST:
			{
				// USERNAME, MESSAGE-INTEGRITY and PRIORITY are required.
				if (!packet->HasMessageIntegrity() || (packet->GetPriority() == 0u) || packet->GetUsername().empty())
				{
					MS_WARN_TAG(ice, "missing required attributes in STUN Binding Request => 400");

					// Reply 400.
					RTC::StunPacket* response = packet->CreateErrorResponse(400);

					response->Serialize(StunSerializeBuffer);
					this->listener->OnIceServerSendStunPacket(this, response, tuple);

					delete response;

					return;
				}

				// Check authentication.
				switch (packet->CheckAuthentication(this->usernameFragment, this->password))
				{
					case RTC::StunPacket::Authentication::OK:
					{
						if (!this->oldPassword.empty())
						{
							MS_DEBUG_TAG(ice, "new ICE credentials applied");

							this->oldUsernameFragment.clear();
							this->oldPassword.clear();
						}

						break;
					}

					case RTC::StunPacket::Authentication::UNAUTHORIZED:
					{
						// We may have changed our usernameFragment and password, so check
						// the old ones.
						
						if (
							!this->oldUsernameFragment.empty() &&
							!this->oldPassword.empty() &&
							packet->CheckAuthentication(this->oldUsernameFragment, this->oldPassword) == RTC::StunPacket::Authentication::OK
						)
						
						{
							MS_DEBUG_TAG(ice, "using old ICE credentials");

							break;
						}

						MS_WARN_TAG(ice, "wrong authentication in STUN Binding Request => 401");

						// Reply 401.
						RTC::StunPacket* response = packet->CreateErrorResponse(401);

						response->Serialize(StunSerializeBuffer);
						this->listener->OnIceServerSendStunPacket(this, response, tuple);

						delete response;

						return;
					}

					case RTC::StunPacket::Authentication::BAD_REQUEST:
					{
						MS_WARN_TAG(ice, "cannot check authentication in STUN Binding Request => 400");

						// Reply 400.
						RTC::StunPacket* response = packet->CreateErrorResponse(400);

						response->Serialize(StunSerializeBuffer);
						this->listener->OnIceServerSendStunPacket(this, response, tuple);

						delete response;

						return;
					}
				}

				// TODO: Should be rejected with 487, but this makes Chrome happy:
				//   https://bugs.chromium.org/p/webrtc/issues/detail?id=7478
				// The remote peer must be ICE controlling.
				// if (packet->GetIceControlled())
				// {
				// 	MS_WARN_TAG(ice, "peer indicates ICE-CONTROLLED in STUN Binding Request => 487");
				//
				// 	// Reply 487 (Role Conflict).
				// 	RTC::StunPacket* response = packet->CreateErrorResponse(487);
				//
				// 	response->Serialize(StunSerializeBuffer);
				// 	this->listener->OnIceServerSendStunPacket(this, response, tuple);
				//
				// 	delete response;
				//
				// 	return;
				// }

                                SDebug <<  "processing STUN Binding Request [Priority: ] " << static_cast<uint32_t>(packet->GetPriority()) <<  " UseCandidate: ]" <<   (packet->HasUseCandidate() ? "true" : "false") ;
//				MS_DEBUG_DEV(
//				  "processing STUN Binding Request [Priority: PRIu32 ", UseCandidate:%s]",
//				  static_cast<uint32_t>(packet->GetPriority()),
//				  packet->HasUseCandidate() ? "true" : "false");

				// Create a success response.
				RTC::StunPacket* response = packet->CreateSuccessResponse();

				// Add XOR-MAPPED-ADDRESS.
				response->SetXorMappedAddress(tuple->GetRemoteAddress());

				// Authenticate the response.
				if (this->oldPassword.empty())
					response->Authenticate(this->password);
				else
					response->Authenticate(this->oldPassword);

				// Send back.
				response->Serialize(StunSerializeBuffer);
				this->listener->OnIceServerSendStunPacket(this, response, tuple);

				delete response;

				// Handle the tuple.
				HandleTuple(tuple, packet->HasUseCandidate());

				break;
			}

			case RTC::StunPacket::Class::INDICATION:
			{
				MS_DEBUG_TAG(ice, "STUN Binding Indication processed");

				break;
			}

			case RTC::StunPacket::Class::SUCCESS_RESPONSE:
			{
				MS_DEBUG_TAG(ice, "STUN Binding Success Response processed");

				break;
			}

			case RTC::StunPacket::Class::ERROR_RESPONSE:
			{
				MS_DEBUG_TAG(ice, "STUN Binding Error Response processed");

				break;
			}
		}
	}

	bool IceServer::IsValidTuple(const RTC::TransportTuple* tuple) const
	{

		return HasTuple(tuple) != nullptr;
	}

	void IceServer::RemoveTuple(RTC::TransportTuple* tuple)
	{
		

		RTC::TransportTuple* removedTuple{ nullptr };

		// Find the removed tuple.
		auto it = this->tuples.begin();

		for (; it != this->tuples.end(); ++it)
		{
			RTC::TransportTuple* storedTuple = std::addressof(*it);

			if (storedTuple->Compare(tuple))
			{
				removedTuple = storedTuple;

				break;
			}
		}

		// If not found, ignore.
		if (!removedTuple)
			return;

		// Remove from the list of tuples.
		this->tuples.erase(it);

		// If this is not the selected tuple, stop here.
		if (removedTuple != this->selectedTuple)
			return;

		// Otherwise this was the selected tuple.
		this->selectedTuple = nullptr;

		// Mark the first tuple as selected tuple (if any).
		if (this->tuples.begin() != this->tuples.end())
		{
			SetSelectedTuple(std::addressof(*this->tuples.begin()));
		}
		// Or just emit 'disconnected'.
		else
		{
			// Update state.
			this->state = IceState::DISCONNECTED;
			// Notify the listener.
			this->listener->OnIceServerDisconnected(this);
		}
	}

	void IceServer::ForceSelectedTuple(const RTC::TransportTuple* tuple)
	{
		

		assertm(
		  this->selectedTuple, "cannot force the selected tuple if there was not a selected tuple");

		auto* storedTuple = HasTuple(tuple);

		assertm(
		  storedTuple,
		  "cannot force the selected tuple if the given tuple was not already a valid tuple");

		// Mark it as selected tuple.
		SetSelectedTuple(storedTuple);
	}

	void IceServer::HandleTuple(RTC::TransportTuple* tuple, bool hasUseCandidate)
	{
		

		switch (this->state)
		{
			case IceState::NEW:
			{
				// There should be no tuples.
				assertm(
				  this->tuples.empty(), "state is 'new' but there are tuples");

				// There shouldn't be a selected tuple.
				assertm(!this->selectedTuple, "state is 'new' but there is selected tuple");

				if (!hasUseCandidate)
				{
					MS_DEBUG_TAG(ice, "transition from state 'new' to 'connected'");

					// Store the tuple.
					auto* storedTuple = AddTuple(tuple);

					// Mark it as selected tuple.
					SetSelectedTuple(storedTuple);
					// Update state.
					this->state = IceState::CONNECTED;
					// Notify the listener.
					this->listener->OnIceServerConnected(this);
				}
				else
				{
					MS_DEBUG_TAG(ice, "transition from state 'new' to 'completed'");

					// Store the tuple.
					auto* storedTuple = AddTuple(tuple);

					// Mark it as selected tuple.
					SetSelectedTuple(storedTuple);
					// Update state.
					this->state = IceState::COMPLETED;
					// Notify the listener.
					this->listener->OnIceServerCompleted(this);
				}

				break;
			}

			case IceState::DISCONNECTED:
			{
				// There should be no tuples.
				assertm(
				  this->tuples.empty(),
				  "state is 'disconnected' but there are tuples" );

				// There shouldn't be a selected tuple.
				assertm(!this->selectedTuple, "state is 'disconnected' but there is selected tuple");

				if (!hasUseCandidate)
				{
					MS_DEBUG_TAG(ice, "transition from state 'disconnected' to 'connected'");

					// Store the tuple.
					auto* storedTuple = AddTuple(tuple);

					// Mark it as selected tuple.
					SetSelectedTuple(storedTuple);
					// Update state.
					this->state = IceState::CONNECTED;
					// Notify the listener.
					this->listener->OnIceServerConnected(this);
				}
				else
				{
					MS_DEBUG_TAG(ice, "transition from state 'disconnected' to 'completed'");

					// Store the tuple.
					auto* storedTuple = AddTuple(tuple);

					// Mark it as selected tuple.
					SetSelectedTuple(storedTuple);
					// Update state.
					this->state = IceState::COMPLETED;
					// Notify the listener.
					this->listener->OnIceServerCompleted(this);
				}

				break;
			}

			case IceState::CONNECTED:
			{
				// There should be some tuples.
				assertm(!this->tuples.empty(), "state is 'connected' but there are no tuples");

				// There should be a selected tuple.
				assertm(this->selectedTuple, "state is 'connected' but there is not selected tuple");

				if (!hasUseCandidate)
				{
					// If a new tuple store it.
					if (!HasTuple(tuple))
						AddTuple(tuple);
				}
				else
				{
					MS_DEBUG_TAG(ice, "transition from state 'connected' to 'completed'");

					auto* storedTuple = HasTuple(tuple);

					// If a new tuple store it.
					if (!storedTuple)
						storedTuple = AddTuple(tuple);

					// Mark it as selected tuple.
					SetSelectedTuple(storedTuple);
					// Update state.
					this->state = IceState::COMPLETED;
					// Notify the listener.
					this->listener->OnIceServerCompleted(this);
				}

				break;
			}

			case IceState::COMPLETED:
			{
				// There should be some tuples.
				assertm(!this->tuples.empty(), "state is 'completed' but there are no tuples");

				// There should be a selected tuple.
				assertm(this->selectedTuple, "state is 'completed' but there is not selected tuple");

				if (!hasUseCandidate)
				{
					// If a new tuple store it.
					if (!HasTuple(tuple))
						AddTuple(tuple);
				}
				else
				{
					auto* storedTuple = HasTuple(tuple);

					// If a new tuple store it.
					if (!storedTuple)
						storedTuple = AddTuple(tuple);

					// Mark it as selected tuple.
					SetSelectedTuple(storedTuple);
				}

				break;
			}
		}
	}

	inline RTC::TransportTuple* IceServer::AddTuple(RTC::TransportTuple* tuple)
	{
		

		// Add the new tuple at the beginning of the list.
		this->tuples.push_front(*tuple);

		auto* storedTuple = std::addressof(*this->tuples.begin());

		// If it is UDP then we must store the remote address (until now it is
		// just a pointer that will be freed soon).
		if (storedTuple->GetProtocol() == TransportTuple::Protocol::UDP)
			storedTuple->StoreUdpRemoteAddress();

		// Return the address of the inserted tuple.
		return storedTuple;
	}

	inline RTC::TransportTuple* IceServer::HasTuple(const RTC::TransportTuple* tuple) const
	{
		

		// If there is no selected tuple yet then we know that the tuples list
		// is empty.
		if (!this->selectedTuple)
			return nullptr;

		// Check the current selected tuple.
		if (this->selectedTuple->Compare(tuple))
			return this->selectedTuple;

		// Otherwise check other stored tuples.
		for (const auto& it : this->tuples)
		{
			auto* storedTuple = const_cast<RTC::TransportTuple*>(std::addressof(it));

			if (storedTuple->Compare(tuple))
				return storedTuple;
		}

		return nullptr;
	}

	inline void IceServer::SetSelectedTuple(RTC::TransportTuple* storedTuple)
	{
		

		// If already the selected tuple do nothing.
		if (storedTuple == this->selectedTuple)
			return;

		this->selectedTuple = storedTuple;

		// Notify the listener.
		this->listener->OnIceServerSelectedTuple(this, this->selectedTuple);
	}
} // namespace RTC
