#ifndef __NETSERVERHEARTBEAT_H__
#define __NETSERVERHEARTBEAT_H__

#include "NetCommon.h"
#include "NetTsqueue.h"
#include "NetMessages.h"
#include "NetConection.h"

namespace netCommon
{
	template <typename T>
	class serverHeartbeat_Interface
	{
	public:
		using self_type = serverHeartbeat_Interface<T>;
		using connection_type = Connection<T>;
		using message_type = message<T>;

	public:
		serverHeartbeat_Interface(uint16_t port)
			:m_asioAcceptor(m_asioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
			, cleantime(m_asioContext)
		{

		}
		virtual ~serverHeartbeat_Interface()
		{
			Stop();
		}

		bool Start()
		{
			try
			{
				WaitForClientConnection();

				m_contextthread = std::thread([this]() {m_asioContext.run(); });
				heartbeatthread = std::thread([this]() {Update(-1, true); });
			}
			catch (std::exception& e)
			{
				std::cerr << "[ServerHeartBeat] EXCEPTION : " << e.what() << '\n';
				return false;
			}
			std::cout << "[ServerHeartBeat] Start!" << "\n";
			return true;
		}
		void Stop()
		{
			m_asioContext.stop();
			if (m_contextthread.joinable()) m_contextthread.join();
			if (heartbeatthread.joinable()) heartbeatthread.join();
			cleantime.cancel();
			std::cout << "[ServerHeartBeat] Stop!" << "\n";
		}

		void WaitForClientConnection()
		{
			m_asioAcceptor.async_accept([this](asio::error_code ec, asio::ip::tcp::socket socket)
				{
					if (ec)
					{
						std::cout << "[ServerHeartBeat] New ServerHeartBeat Connection ERROR: " << ec.message() << "\n";

					}
					else
					{
						std::cout << "[ServerHeartBeat] New ServerHeartBeat Connection: " << socket.remote_endpoint() << "\n";
						std::shared_ptr<connection_type> newconn = std::make_shared<connection_type>(
							connection_type::owner::server, m_asioContext, std::move(socket), m_qMessageIn);
						if (OnClientConnect(newconn))
						{
							newconn->ConnectHeartbeatToClient(this, m_nIDCounter++);
							std::cout << "[" << newconn->GetId() << "] Connection Approved" << "\n";
							HeartBeatStart(newconn);
							
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

		void clientDisconnect(std::shared_ptr<connection_type> client)
		{
			OnClientDisconnect(client);
			client.reset();
		}

		void clearConnectDeque()
		{
			m_deqConnection.erase(std::remove(m_deqConnection.begin(), m_deqConnection.end(), nullptr), m_deqConnection.end());
		}

		void Update(size_t n_MaxMessage = -1, bool bWait = false)
		{
			while(1)
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
		}


	public:
		void HeartBeatStart(std::shared_ptr<connection_type> client)
		{
			std::cout << "[" << client->GetId() << "]: HeateBeatStart\n";
			message_type msg;
			msg.header.id = T::HeartBeatStart;
			client->Send(msg);

		}

		bool HeartBeat(std::shared_ptr<connection_type> client, message_type& msg)
		{
			std::cout << "[" << client->GetId() << "]: HeateBeat\n";

			client->Send(msg);
			return true;
		}

		
	protected:
		virtual bool OnClientConnect(std::shared_ptr<connection_type> client)
		{
			return true;
		}

		virtual void OnClientDisconnect(std::shared_ptr<connection_type> client)
		{

		}

		virtual bool OnMessage(std::shared_ptr<connection_type> client, message_type& msg)
		{
			switch (msg.header.id)
			{
			case netCommon::Type::ServerMsgType::HeartBeat:
			{
				HeartBeat(client, msg);
				return true;
			}
			default:
				return false;
			}
			return false;
		}


	private:
		void clean()
		{
			for (auto client : m_deqConnection)
			{
				if (!client->IsConnected())
				{
					clientDisconnect();
				}
				
			}
			clearConnectDeque();
		}

		void setclean()
		{
			cleantime.expires_after(std::chrono::seconds(10));
			cleantime.async_wait(std::bind(&self_type::clean, this));
		}

	private:
		uint32_t m_nIDCounter = 10000;
		asio::io_context m_asioContext;
		std::thread m_contextthread;
		std::thread heartbeatthread;
		asio::ip::tcp::acceptor m_asioAcceptor;
		asio::steady_timer cleantime;
		Ts_queue<owned_Message<T>> m_qMessageIn;
		std::deque<std::shared_ptr<connection_type>> m_deqConnection;
	};
}



#endif 
