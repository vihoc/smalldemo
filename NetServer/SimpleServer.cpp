#include "BaseHeader.h"
#include "TypeHeader.h"
#include "NetClient.h"
#include "NetServer.h"
#include "CommonMacro.h"



class CustomServer : public netCommon::server_Interface<netCommon::Type::ServerMsgType>
{
	using serverPtr = std::shared_ptr<netCommon::Connection<netCommon::Type::ServerMsgType>>;
public:
	CustomServer(uint16_t nPort) : netCommon::server_Interface<netCommon::Type::ServerMsgType>(nPort)
	{

	}

protected:
	virtual bool OnClientConnect(serverPtr client) override
	{
		netCommon::message<netCommon::Type::ServerMsgType> msg = netCommon::message<netCommon::Type::ServerMsgType>();
		msg.header.id = netCommon::Type::ServerMsgType::ServerAccept;
		client->Send(msg);
		return true;
	}

	virtual void OnClientDisconnect(serverPtr client) override
	{
		std::cout << "Removing client [" << client->GetId() << "]\n";
	}

	virtual bool OnMessage(serverPtr client, message_type& msg) override
	{
		BEGIN_PROCESS_MESSAGE(msg.header.id)
			PROCESS_MESSAGE_ITEM(netCommon::Type::ServerMsgType::ServerPing, Pingmsg, client, msg)
			PROCESS_MESSAGE_ITEM(netCommon::Type::ServerMsgType::MessageAll, NoticeAll, msg, client)
		END_PROCESS_MESSAGE

	}

public:

	virtual void OnClientValidated(serverPtr client) override
	{

		netCommon::message<netCommon::Type::ServerMsgType> msg;
		msg.header.id = netCommon::Type::ServerMsgType::Client_Accepted;
		client->Send(msg);
	}


public:
	bool Pingmsg(serverPtr client, CustomServer::message_type& msg)
	{
		std::cout << "[" << client->GetId() << "]: Server Ping\n";

		client->Send(msg);
		return true;
	}

	bool NoticeAll(CustomServer::message_type& msg, serverPtr ignoreclient = nullptr)
	{
		std::cout << "[" << ignoreclient->GetId() << "]: Message All\n";
		 
		netCommon::message<netCommon::Type::ServerMsgType> sendmsg;
		sendmsg.header.id = netCommon::Type::ServerMsgType::ServerMessage;
		sendmsg << ignoreclient->GetId();
		NoticeAllClient(sendmsg, ignoreclient);
		return true;
	}

};





int main()
{
	CustomServer server(60000);
	server.Start();

	while (1)
	{
		server.Update(-1, true);
	}



	return 0;
}