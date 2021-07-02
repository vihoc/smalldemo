#ifndef __NETSERVER_H__
#define __NETSERVER_H__

#include "NetCommon.h"
#include "NetTsqueue.h"
#include "NetMessages.h"
#include "NetConection.h"

#include "ServerHeartBeat.h"
namespace netCommon
{
	constexpr uint32_t connectCountStart = 10000;
	template <typename T>
	class server_Interface
	{
	public:
		using connection_type = Connection<T>;
		using message_type = message<T>;

	public:
		server_Interface(uint16_t port)
			:m_asioAcceptor(m_asioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
			, m_port{port}
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
			
			heartbeat = std::make_unique<serverHeartbeat_Interface<T>>(m_port + heartbeatoffportset);
			heartbeat->Start();

			return true;
		}
		void Stop()
		{
			m_asioContext.stop();
			heartbeat->Stop();
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
							if (m_ConnectionMap.end() != m_ConnectionMap.find(m_nIDCounter))
							{
								std::cerr << "[" << m_nIDCounter << "] exist!" << std::endl;
								newconn->Disconnect();
								return;
							}
							newconn->ConnectToClient(this, m_nIDCounter);
							std::cout << "[" << newconn->GetId() << "] Connection Approved" << "\n";
							
							m_ConnectionMap.emplace(m_nIDCounter++, std::move(newconn));
							

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
			
			needDelete.emplace_back(client->GetId());
			//client.reset();
		}

		void clearConnectDeque()
		{
			if(!needDelete.empty())
			{
				for (auto& id : needDelete)
				{ 
					auto itor = m_ConnectionMap.find(id);
					if(m_ConnectionMap.end() != itor)
					{ 
						m_ConnectionMap.erase(itor);
					}
				}
			}

		}

		void NoticeClient(std::shared_ptr<connection_type> client, const message_type& msg)
		{
			if (client)
			{ 
				if (client->IsConnected())
				{
					std::cout << "[" << client->GetId() << "]" << "Sending msg : " << (uint32_t)msg.header.id << "\n";
 					client->Send(msg);
				}
				else
				{
					clientDisconnect(client);
					clearConnectDeque();
				}
			}
		}

		bool NoticeAllClient(const message_type& msg , std::shared_ptr<connection_type> ignoreclient = nullptr)
		{
			bool InvaildClient = false;
			for (auto& connectPair : m_ConnectionMap)
			{
				auto& client = connectPair.second;
				if (client && client->IsConnected())
				{
					if (ignoreclient != client)
					{
						std::cout << "[" << client->GetId() << "]" << "Broadcast msg : " << (uint32_t)msg.header.id << "\n";
						client->Send(msg);
					}
				}
				else
				{
					clientDisconnect(client);
					InvaildClient = true;
				}
			}
			if (InvaildClient)
			{
				clearConnectDeque();
			}
			return true;
		}

		bool NoticeClientById(uint32_t id, message_type& msg)
		{
			auto itor = m_ConnectionMap.find(id);
			if (m_ConnectionMap.end() == itor)
			{
				return false;
			}
			itor->second->send(msg);
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

		void DisconnectClient(uint32_t id)
		{
			auto itor = m_ConnectionMap.find(id);
			if (itor != m_ConnectionMap.end())
			{
				m_ConnectionMap.erase(itor);
			}
		}

		bool is_clientConnected(uint32_t id)
		{
			auto itor = m_ConnectionMap.find(id);
			if (itor != m_ConnectionMap.end())
			{
				return itor->second->IsConnected();
			}
			return false;
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
		uint16_t m_port;
		uint32_t m_nIDCounter = connectCountStart;
		asio::io_context m_asioContext;
		std::thread m_contextthread;
		asio::ip::tcp::acceptor m_asioAcceptor;
		
		Ts_queue<owned_Message<T>> m_qMessageIn;
		std::unordered_map<uint32_t, std::shared_ptr<connection_type>> m_ConnectionMap;
		std::vector<uint32_t> needDelete;
		std::unique_ptr<serverHeartbeat_Interface<T>> heartbeat; 
	};
}



#endif 