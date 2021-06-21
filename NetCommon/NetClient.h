#ifndef __NETCLIENT_H__
#define __NETCLIENT_H__

#include "CommonConst.h"
#include "NetCommon.h"
#include "NetTsqueue.h"
#include "NetMessages.h"
#include "NetConection.h"
#include "ClientHeartBeat.h"

namespace netCommon
{
	template<typename T>
	class client_Interface
	{
	public:
		using self_type = client_Interface<T>;
		using connection_type = Connection<T>;
		using ownmessage_type = owned_Message<T>;
	public:
		client_Interface()
		{}
		virtual ~client_Interface()
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
				m_connection->ConnectToServer(endpoint);
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
			
			if (contextThread.joinable()) contextThread.join();
			//m_connection.release(); 
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

		Ts_queue<ownmessage_type>& Incoming()
		{
			return m_qMessageIn;
		}

		void startHeartBeat(const std::string& host, const uint16_t port)
		{
			if(nullptr == heartbeat)
			{ 
				heartbeat = std::make_unique<netCommon::heartbeat_Interface<T>>(m_qMessageIn);
				heartbeat->Connect(host, port + heartbeatoffportset);
				heartbeat->startHeartBeat();
			}
		}

	protected:
		std::shared_ptr<connection_type> m_connection;
		asio::io_context m_context;
		std::thread contextThread;
		//asio::ip::tcp::socket m_socket;
		std::unique_ptr<netCommon::heartbeat_Interface<T>> heartbeat;
	private:
		Ts_queue<ownmessage_type> m_qMessageIn;
	};
} // end of namespace netCommon


#endif // end  of __NETCLIENT_H__
