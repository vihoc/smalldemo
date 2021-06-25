#ifndef __NETCONNECTION_H__
#define __NETCONNECTION_H__

#include "NetCommon.h"
#include "NetTsqueue.h"
#include "NetMessages.h"
#include "NetServer.h"

#include "ServerHeartBeat.h"
namespace netCommon
{
	template <typename T>
	class server_Interface;

	template <typename T>
	class heartbeat_Interface;

	template <typename T>
	struct CheckValidation
	{
	
	};

	template <>
	struct CheckValidation<uint64_t>
	{
	public:
		uint64_t m_nHandshakeOut = 0;
		uint64_t m_nHandshakeIn = 0;
		uint64_t m_nHandshakeCheck = 0;

	public:
		CheckValidation() = default;
		virtual uint64_t scramble(uint64_t nInput)
		{
			uint64_t out = nInput ^ 0xDEADBEEFC0DECAFE;
			out = (out & 0xF0F0F0F0F0F0F0) >> 4 | (out & 0x0F0F0F0F0F0F0F) << 4;
			return out ^ 0xC0DEFACE12345678;
		}

	};
	//TODO
	template <typename T>
	class read_buffer
	{
	public:
		// Construct from a std::string.
		explicit read_buffer(size_t length)
			: data_(new std::vector<char>(length)),
			buffer_(asio::buffer(*data_))
		{
		}

		// Implement the ConstBufferSequence requirements.
		typedef asio::const_buffer value_type;
		typedef const asio::const_buffer* const_iterator;
		const asio::const_buffer* begin() const { return &buffer_; }
		const asio::const_buffer* end() const { return &buffer_ + 1; }

	private:
		std::shared_ptr<std::vector<char> > data_;
		asio::const_buffer buffer_;
	};

	template <typename T>
	class shared_const_buffer
	{
	public:
		// Construct from a std::string.
		explicit shared_const_buffer(const T& data)
			: data_(new std::vector<char>(data.begin(), data.end())),
			buffer_(asio::buffer(*data_))
		{
		}

		// Implement the ConstBufferSequence requirements.
		typedef asio::const_buffer value_type;
		typedef const asio::const_buffer* const_iterator;
		const asio::const_buffer* begin() const { return &buffer_; }
		const asio::const_buffer* end() const { return &buffer_ + 1; }

	private:
		std::shared_ptr<std::vector<char> > data_;
		asio::const_buffer buffer_;
	};

	template <typename T>
	class Connection : public std::enable_shared_from_this<Connection<T>>
	{
	public:
		using self_type = Connection<T>;
		using outmessage_type = message<T>;
		using inmessage_type = owned_Message<T>;
	public:
		enum class owner
		{
			server, 
			client
		};
	public:
		Connection(owner parent, asio::io_context& asio_context, asio::ip::tcp::socket socket, Ts_queue<owned_Message<T>>& queuereference)
			:m_asioContext(asio_context), m_socket(std::move(socket)), m_qMessageIn(queuereference)
		{
			m_ownerType = parent;
			time_out = std::make_unique<asio::steady_timer>(m_asioContext);
			if (owner::server == m_ownerType)
			{
				m_handeshake.m_nHandshakeOut = uint64_t(std::chrono::system_clock::now().time_since_epoch().count());

				m_handeshake.m_nHandshakeCheck = m_handeshake.scramble(m_handeshake.m_nHandshakeOut);
			}
		}

		virtual ~Connection()
		{
			//std::cerr << "Connection Crushed" << "\n";
		}
		uint32_t GetId()
		{
			return id;
		}
		uint32_t GetId()const
		{
			return id;
		}
	public:
		
		void ConnectToClient(netCommon::server_Interface<T>* server, uint32_t uid = 0)
		{
			if (owner::server == m_ownerType)
			{
				if (m_socket.is_open())
				{
					//time_out->cancel();
					id = uid;
					WriteValidation();
					ReadValidation(server);
				}
			}
			//ConnectTimeOutHander();
		}

		void ConnectToServer(const asio::ip::tcp::resolver::results_type& endpoints)
		{
			if (owner::client == m_ownerType)
			{
				// Request asio attempts to connect to an endpoint
				asio::async_connect(m_socket, endpoints,
					[this](std::error_code ec, asio::ip::tcp::endpoint endpoint)
					{
						if (!ec)
						{
							time_out->cancel();
							ReadValidation();
						}
					});
				ConnectTimeOutHander();
			}

		}

		void ConnectHeartbeatToServer(const asio::ip::tcp::resolver::results_type& endpoint)
		{
			if (owner::client == m_ownerType)
			{
				// Request asio attempts to connect to an endpoint
				asio::async_connect(m_socket, endpoint,
					[this](std::error_code ec, asio::ip::tcp::endpoint endpoint)
					{
						if (!ec)
						{
							time_out->cancel();
							ReadHeader();
						}
					});
				ConnectTimeOutHander();
			}
		}

		void ConnectHeartbeatToClient(netCommon::serverHeartbeat_Interface<T>* server, uint32_t uid = 0)
		{
			if (owner::server == m_ownerType)
			{
				if (m_socket.is_open())
				{
					//time_out->cancel();
					id = uid;
					ReadHeader();
					
				}
				//ConnectTimeOutHander();
			}
		}
		
		void Disconnect()
		{
			if (IsConnected())
					asio::post(m_asioContext, [this]() { m_socket.close(); });
		}
		
		bool IsConnected()
		{
			return m_socket.is_open();
		}
		bool IsConnected() const
		{
			return m_socket.is_open();
		}

	public:

		void ConnectTimeOutHander()
		{
			using namespace std::chrono_literals;
			time_out->expires_from_now(2000ms);
			time_out->async_wait([this](const asio::error_code& ec) {
				if (!ec) 
				{
					m_socket.close(); 
					std::cerr << "Connect TimeOut!\n"; 
				} 
				});
		}

		void Send(const outmessage_type& msg)
		{
			auto self =this->shared_from_this();
			asio::post(m_asioContext,
				[self, msg]()
				{
					bool bWritingMessage = !self->m_qMessageOut.empty();
					self->m_qMessageOut.emplace_back(msg);
					if (!bWritingMessage)
					{
						self->WriteHeader();
					}
				});
		}

		template <typename Buffer>
		void async_read(Buffer readbuffer, std::function<void(asio::error_code, std::size_t)> callback)
		{
			//auto self = this->shared_from_this();
			asio::async_read(m_socket, readbuffer, std::bind(callback, std::placeholders::_1, std::placeholders::_2));

		}
		template <typename Buffer>
		void async_write(Buffer writebuffer, std::function<void(asio::error_code, std::size_t)> callback)
		{
			//auto self = this->shared_from_this();
			asio::async_write(m_socket, writebuffer, std::bind(callback, std::placeholders::_1, std::placeholders::_2));

		}

	private:

		void WriteHeader()
		{
			//std::cout << "start Write Header" << "\n";
			auto self = this->shared_from_this();
			if(!m_qMessageOut.empty())
			{ 
				std::shared_ptr<outmessage_type> buffer = std::make_shared<outmessage_type>(std::move(m_qMessageOut.front()));
				m_qMessageOut.pop_front();
				//auto& outBuffer = m_qMessageOut.front();
				asio::async_write(m_socket, asio::buffer(&(buffer->header), sizeof(outmessage_type::header_type)),
					[self, buffer](asio::error_code ec, std::size_t length)
					{
						//std::cout << "Write Header Finished" << "\n";
						if (ec)
						{
							std::cout << "[" << self->id << "] Write Header Fail." << "\n";
							self->m_socket.close();
							return;
						}
						if (buffer->size() > 0)
						{
							self->WriteBody(buffer);
						}
						self->WriteHeader();

					}
				);
			}
		}
		void WriteBody(std::shared_ptr<outmessage_type> msg)
		{
			//std::cout << "start Write Body" << "\n";
			auto self = this->shared_from_this();
			asio::async_write(m_socket, asio::buffer(msg->body.data(), msg->body.size()),
				[self, msg](asio::error_code ec, std::size_t length)
				{
					//std::cout << "Write Body Finished " << "\n";
					if (ec)
					{
						std::cout << "[" << self->id << "] Write Body Fail." << "\n";
						self->m_socket.close();
						return;
					}
					if (!self->m_qMessageOut.empty())
					{
						self->WriteHeader();
					}
				});
		}

		void ReadHeader()
		{
			try
			{
				std::shared_ptr<outmessage_type> tempBuffer = std::make_shared<outmessage_type>();
				//std::cout << "start Reading Header" << "\n";
				auto self = this->shared_from_this();
				asio::async_read(m_socket, asio::buffer(&(tempBuffer->header), sizeof(outmessage_type::header_type)),
					[self, tempBuffer](asio::error_code ec, std::size_t length)
					{
						//std::cout << " Reading Header Finished" << "\n";
						if (ec)
						{
							std::cout << "[" << self->id << "] Read Header Fail." << ec.message() << "\n";

							if (self->m_socket.is_open())
							{
								asio::error_code ecsocket;
								self->m_socket.close(ecsocket);
								if (ecsocket)
								{
									std::cout << "[" << self->id << "] close socket Fail." << ecsocket.message() << "\n";
								}
							}
							return;
						}

						if (tempBuffer->header.size > 0)
						{
							tempBuffer->body.resize(tempBuffer->header.size);
							self->ReadBody(tempBuffer);
						}
						else
						{
							self->AddToIncomingMessageQueue(*tempBuffer);
						}

					});
			}
			catch (std::exception& e)
			{
				std::cout << e.what() << std::endl;
			}
		

		}
		void ReadBody(std::shared_ptr<outmessage_type> buffer)
		{
			//std::cout << "start Reading Body" << "\n";
			auto self = this->shared_from_this();
			asio::async_read(m_socket, asio::buffer(buffer->body.data(), buffer->body.size()),
				[self, buffer](asio::error_code ec, std::size_t length)
				{
					//std::cout << " Reading Body finished" << "\n";
					if (ec)
					{
						std::cout << "[" << self->id << "] Read Body Fail." << ec.message()<< "\n";
						self->m_socket.close();
						return;
					}
					self->AddToIncomingMessageQueue(*buffer);
				});
		}



		void AddToIncomingMessageQueue(outmessage_type& buffer)
		{
			if (owner::server == m_ownerType)
			{
				m_qMessageIn.emplace_back({ this->shared_from_this(), buffer });
			}
			else
			{
				m_qMessageIn.emplace_back({ nullptr, buffer });
			}
			ReadHeader();
		}

		private:

			void WriteValidation()
			{
				
				auto self = this->shared_from_this();
				asio::async_write(m_socket, asio::buffer(&m_handeshake.m_nHandshakeOut, sizeof(uint64_t)),
					[self](std::error_code ec, std::size_t length)
					{
						if (!ec)
						{
							
							if (owner::client == self->m_ownerType) self->ReadHeader();
						}
						else
						{
							self->m_socket.close();
						}
					});
			}

			void ReadValidation(netCommon::server_Interface<T>* server = nullptr)
			{
				auto self = this->shared_from_this();

				asio::async_read(m_socket, asio::buffer(&m_handeshake.m_nHandshakeIn, sizeof(uint64_t)),
					[self, server](std::error_code ec, std::size_t length)
					{
						if (!ec)
						{
							
							if (owner::server == self->m_ownerType)
							{
								
								if (self->m_handeshake.m_nHandshakeIn == self->m_handeshake.m_nHandshakeCheck)
								{
									
									std::cout << "Client Validated" << std::endl;
									if(nullptr != server)
									{ 
										server->OnClientValidated(self);
									}

									self->ReadHeader();
								}
								else
								{
									std::cout << "Client Disconnected (Fail Validation)" << std::endl;
									self->m_socket.close();
								}
							}
							else
							{

								self->m_handeshake.m_nHandshakeOut = self->m_handeshake.scramble(self->m_handeshake.m_nHandshakeIn);
								
								self->WriteValidation();
							}
						}
						else
						{
							
							std::cout << "Client Disconnected (ReadValidation)" << std::endl;
							self->m_socket.close();
						}
					});
			}



	protected:
		uint32_t id = 0;
		CheckValidation<uint64_t> m_handeshake;
		owner m_ownerType;
		asio::ip::tcp::socket m_socket;
		asio::io_context& m_asioContext;
		std::unique_ptr<asio::steady_timer> time_out;
		Ts_queue<outmessage_type> m_qMessageOut;
		Ts_queue<inmessage_type>& m_qMessageIn;
	};
} // end of namespace netCommon

#endif //end of __NETCONNECTION_H__
