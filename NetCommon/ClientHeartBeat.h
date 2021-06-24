#ifndef __NETHEARTBEAT_H__
#define __NETHEARTBEAT_H__

#include "NetCommon.h"
#include "NetTsqueue.h"
#include "NetMessages.h"
#include "NetConection.h"

namespace netCommon
{
	template<typename T>
	class heartbeat_Interface
	{
		
	public:
		using self_type = heartbeat_Interface<T>;
		using connection_type = Connection<T>;
		using ownmessage_type = owned_Message<T>;
	public:
		heartbeat_Interface(Ts_queue<ownmessage_type>& MessageIn) : m_qMessageIn{ MessageIn }
		{}
		virtual ~heartbeat_Interface()
		{
			Disconnect();
		}

	public:
		bool Connect(const std::string& host, const uint16_t port)
		{
			try
			{
				asio::ip::tcp::resolver resolver(m_context);
				asio::ip::tcp::resolver::results_type endpoint = resolver.resolve(host, std::to_string(port));
				m_connection = std::make_shared<connection_type>(connection_type::owner::client, m_context, asio::ip::tcp::socket(m_context), m_qMessageIn);
				m_connection->ConnectHeartbeatToServer(endpoint);
				heartbeat_timer = std::make_unique<asio::steady_timer>(m_context);
				contextThread = std::thread([this]() {m_context.run(); });
			}
			catch (std::exception& e)
			{
				std::cerr << "Client Exception: " << e.what() << "\n";
				return false;
			}



			return true;
		}
		void Disconnect()
		{
			if (IsConnected())
			{
				m_connection->Disconnect();
			}
			m_context.stop();
			heartbeat_timer->cancel();
			if (contextThread.joinable()) contextThread.join();
			m_connection = nullptr;
		}
		bool IsConnected()
		{
			return nullptr == m_connection ? false : m_connection->IsConnected();
		}

		void Send(const message<T>& msg)
		{
			if (IsConnected())
				m_connection->Send(msg);
		}



		void startHeartBeat()
		{

			send_heartbeatHeader();
		}

	private:
		void send_heartbeatHeader()
		{
			if (!IsConnected())
				return;
			std::shared_ptr<ownmessage_type::message_type> msg = std::make_shared<ownmessage_type::message_type>();
			msg->header.id = T::HeartBeat;
			(*msg) << "are u still there";
			m_connection->async_write(asio::buffer(&(msg->header), sizeof(ownmessage_type::message_type::header_type)),
				[this, msg](asio::error_code ec, std::size_t length)
				{

					if (ec)
					{
						std::cout << "[" << m_connection->GetId() << "] Heartbeat Header Fail." << "\n";
						m_connection->Disconnect();
						return;
					}
					if (msg->size() > 0)
					{

						m_connection->async_write(asio::buffer(msg->body.data(), msg->body.size()),
							[this](asio::error_code ec, std::size_t length)
							{
								if (ec)
								{
									std::cout << "[" << m_connection->GetId() << "] Write Body Fail." << "\n";
									m_connection->Disconnect();
									return;
								}
								//std::cout << "send Heartbeat\n";
								handle_heartbeat(ec, length);
							});
					}

				});
		}



		void handle_heartbeat(const asio::error_code& error, std::size_t length)
		{
			if (!IsConnected())
				return;

			if (!error)
			{
				// Wait 10 seconds before sending the next heartbeat.
				heartbeat_timer->expires_after(std::chrono::seconds(10));
				heartbeat_timer->async_wait(std::bind(&self_type::send_heartbeatHeader, this));
			}
			else
			{
				std::cout << "Error on heartbeat: " << error.message() << "\n";

				//stop();
				m_connection->Disconnect();
			}
		}

	protected:
		asio::io_context m_context;
		std::thread contextThread;
		//asio::ip::tcp::socket m_socket;

		std::shared_ptr<connection_type> m_connection;
		std::unique_ptr<asio::steady_timer> heartbeat_timer;
		Ts_queue<ownmessage_type>& m_qMessageIn;
	};
} // end of namespace netCommon


#endif // end  of __NETCLIENT_H__
