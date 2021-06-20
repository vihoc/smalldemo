#include "BaseHeader.h"
#include "TypeHeader.h"
#include "NetClient.h"
// template<typename MessageType>
// class CustomClient :public netCommon::client_Interface<MessageType>
// {
// public:
// 
// 	bool FireBullet(float x, float y)
// 	{
// 		netCommon::message<MessageType> msg;
// 		msg.header.id = MessageType::FireBullet;
// 		msg << x << y;
// 		Send(msg);
// 	}
// };
// 
// int main()
// {
// 	netCommon::message<Client::Type::CustomMsgType> msg;
// 	msg.header.id = Client::Type::CustomMsgType::FireBullet;
// 
// 	int a = 1;
// 	bool b = false;
// 	float c = 3.1415926f;
// 	struct 
// 	{
// 		int x;
// 		int y;
// 	}d[5];
// 
// 	msg << a << b << c << d;
// 	msg >> d >> c >> b >> a;
// 	return 0;
// }
using ServerMsgType = netCommon::Type::ServerMsgType;
class CustomClient : public netCommon::client_Interface<ServerMsgType>
{
public:
	void PingServer()
	{
		netCommon::message<ServerMsgType> msg = netCommon::message<ServerMsgType>();
		msg.header.id = ServerMsgType::ServerPing;

		// Caution with this...
		std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();

		msg << timeNow;
		Send(msg);
	}

	void MessageAll()
	{
		netCommon::message<ServerMsgType> msg = netCommon::message<ServerMsgType>();
		msg.header.id = ServerMsgType::MessageAll;
		Send(msg);
	}

	void Login()
	{
		netCommon::message<ServerMsgType> msg = netCommon::message<ServerMsgType>();
		msg.header.id = ServerMsgType::User_Loginin;
		Send(msg);
	}
};

int main()
{
	CustomClient c;
	c.Connect("127.0.0.1", 60000);

	bool key[3] = { false, false, false };
	bool old_key[3] = { false, false, false };

	bool bQuit = false;
	while (!bQuit)
	{
		if (GetForegroundWindow() == GetConsoleWindow())
		{
			key[0] = GetAsyncKeyState('1') & 0x8000;
			key[1] = GetAsyncKeyState('2') & 0x8000;
			key[2] = GetAsyncKeyState('3') & 0x8000;
		}

		if (key[0] && !old_key[0]) c.PingServer();
		if (key[1] && !old_key[1]) c.MessageAll();
		if (key[2] && !old_key[2]) bQuit = true;

		for (int i = 0; i < 3; i++) old_key[i] = key[i];

		if (c.IsConnected())
		{
			if (!c.Incoming().empty())
			{


				auto msg = c.Incoming().pop_front().msg;

				switch (msg.header.id)
				{
				case ServerMsgType::ServerAccept:
				{			
					std::cout << "Server Accepted Connection\n";
				}
				break;


				case ServerMsgType::ServerPing:
				{
					std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
					std::chrono::system_clock::time_point timeThen;
					msg >> timeThen;
					std::cout << "Ping: " << std::chrono::duration<double>(timeNow - timeThen).count() << "\n";
				}
				break;

				case ServerMsgType::ServerMessage:
				{
					uint32_t clientID;
					msg >> clientID;
					std::cout << "Hello from [" << clientID << "]\n";
				}
				break;
				}
			}
		}
		else
		{
			std::cout << "Server Down\n";
			bQuit = true;
		}

	}

	return 0;
}