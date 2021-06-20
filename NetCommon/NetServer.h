#ifndef __NETSERVER_H__
#define __NETSERVER_H__

#include "NetCommon.h"
#include "NetTsqueue.h"
#include "NetMessages.h"
#include "NetConection.h"

namespace netCommon
{
	template <typename T>
	class server_Interface
	{
	public:
		using connection_type = Connection<T>;
		using message_type = message<T>;

	public:
		server_Interface(uint16_t port)
			:m_asioAcceptor(m_asioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
		{

		}
		virtual ~server_Interface()
		{
			Stop();
		}

		bool Start()
		{
			try
			{
				WaitForClientConnection();

				m_contextthread = std::thread([this]() {m_asioContext.run(); });
			}
			catch (std::exception& e)
			{
				std::cerr << "[SERVER] EXCEPTION : " << e.what() << '\n';
				return false;
			}
			std::cout << "[Server] Start!" << "\n";
			return true;
		}
		void Stop()
		{
			m_asioContext.stop();
			if (m_contextthread.joinable()) m_contextthread.join();
			std::cout << "[Server] Stop!" << "\n";
		}

		void WaitForClientConnection()
		{
			m_asioAcceptor.async_accept([this](asio::error_code ec, asio::ip::tcp::socket socket) 
				{
					if (ec)
					{
						std::cout << "[Server] New Connection ERROR: " << ec.message() << "\n";

					}
					else
					{
						std::cout << "[Server] New Connection: " << socket.remote_endpoint() << "\n";
						std::shared_ptr<connection_type> newconn = std::make_shared<connection_type>(
							connection_type::owner::server, m_asioContext, std::move(socket), m_qMessageIn);
						if (OnClientConnect(newconn))
						{
							newconn->ConnectToClient(this, m_nIDCounter++);
							std::cout << "[" << newconn->GetId() << "] Connection Approved" << "\n";
							m_deqConnection.emplace_back(std::move(newconn));
							

						}
						else
						{
							std::cout << "[----] Connection Denied!" << "\n";
						}
					}
					WaitForClientConnection();
				});
		}
		void NoticeClient(std::shared_ptr<connection_type> client, const message_type& msg)
		{
			if (client)
			{ 
				if (client->IsConnected())
				{
					client->Send(msg);
				}
				else
				{
					OnClientDisconnect(client);
					client.reset();
					m_deqConnection.erase(std::remove(m_deqConnection.begin(), m_deqConnection.end(), client), m_deqConnection.end());
				}
			}
		}

		bool NoticeAllClient(const message_type& msg , std::shared_ptr<connection_type> ignoreclient = nullptr)
		{
			bool InvaildClient = false;
			for (auto& client : m_deqConnection)
			{
				if (client && client->IsConnected())
				{
					if (ignoreclient != client)
					{
						
						client->Send(msg);
					}
				}
				else
				{
					OnClientDisconnect(client);
					client.reset();
					InvaildClient = true;
				}
			}
			if (InvaildClient)
			{
				m_deqConnection.erase(std::remove(m_deqConnection.begin(), m_deqConnection.end(), nullptr), m_deqConnection.end());
			}
			return true;
		}

		void Update(size_t n_MaxMessage = -1, bool bWait = false)
		{
			if (bWait) m_qMessageIn.wait();
			size_t MessageCount = 0;
			while (MessageCount < n_MaxMessage && !m_qMessageIn.empty())
			{
				auto msg = m_qMessageIn.pop_front();
				OnMessage(msg.remote, msg.msg);
				MessageCount++;
			}
		}

	protected:
		virtual bool OnClientConnect(std::shared_ptr<connection_type> client)
		{
			return false;
		}

		virtual void OnClientDisconnect(std::shared_ptr<connection_type> client)
		{
			
		}

		virtual bool OnMessage(std::shared_ptr<connection_type> client, message_type& msg)
		{
			return false;
		}
public:
		virtual void OnClientValidated(std::shared_ptr<connection_type > client)
		{

		}

	private:
		uint32_t m_nIDCounter = 10000;
		asio::io_context m_asioContext;
		std::thread m_contextthread;
		asio::ip::tcp::acceptor m_asioAcceptor;
		
		Ts_queue<owned_Message<T>> m_qMessageIn;
		std::deque<std::shared_ptr<connection_type>> m_deqConnection;
	};
}



#endif 