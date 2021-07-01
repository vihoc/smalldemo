#ifndef __MSG_DETAIL_H__
#define __MSG_DETAIL_H__
#include "BaseHeader.h"
#include "Player.h"

#include "TypeHeader.h"
namespace netCommon
{ 
using ServerMsgType = netCommon::Type::DemoMsgType;
struct RequestServerSeedReq
{
	std::string msgtext = "RequestServerSeedReq";

	friend std::ostream& operator << (std::ostream& os, const RequestServerSeedReq& msg)
	{
		os << "msgtext:" << msg.msgtext;
		return os;
	}

	template <typename T>
	friend message<T>& operator << (message<T>& msg, const RequestServerSeedReq& data)
	{
		msg.header.id = ServerMsgType::RequestServerSeedReq;
		msg << data.msgtext;
		return msg;
	}

	template <typename T>
	friend message<T>& operator >> (message<T>& msg, RequestServerSeedReq& data)
	{
		msg >> data.msgtext;

		return msg;
	}

};

struct RequestServerSeedack
{
	std::string msgtext = "RequestServerSeedack";
	uint32_t seed;

	friend std::ostream& operator << (std::ostream& os, const RequestServerSeedack& msg)
	{
		os << "msgtext:" << msg.msgtext << "seed: " << msg.seed;
		return os;
	}

	template <typename T>
	friend message<T>& operator << (message<T>& msg, const RequestServerSeedack& data)
	{
		msg.header.id = ServerMsgType::RequestServerSeedReq;
		msg << data.msgtext << data.seed;
		return msg;
	}

	template <typename T>
	friend message<T>& operator >> (message<T>& msg, RequestServerSeedack& data)
	{
		msg >> data.seed >> data.msgtext ;

		return msg;
	}

};
struct RequestNoticeAllClientReq
	{
		std::string msgtext = "RequestNoticeAllClientReq";

		friend std::ostream& operator << (std::ostream& os, const RequestNoticeAllClientReq& msg)
		{
			os << "msgtext:" << msg.msgtext;
			return os;
		}

		template <typename T>
		friend message<T>& operator << (message<T>& msg, const RequestNoticeAllClientReq& data)
		{
			msg.header.id = ServerMsgType::RequestNoticeAllClientReq;
			msg << data.msgtext;
			return msg;
		}

		template <typename T>
		friend message<T>& operator >> (message<T>& msg, RequestNoticeAllClientReq& data)
		{
			msg >> data.msgtext;

			return msg;
		}
	};

struct RequestNoticeAllClientAck
		{
			std::string msgtext = "RequestNoticeAllClientReq";

			friend std::ostream& operator << (std::ostream& os, const RequestNoticeAllClientAck& msg)
			{
				os << "msgtext:" << msg.msgtext;
				return os;
			}

			template <typename T>
			friend message<T>& operator << (message<T>& msg, const RequestNoticeAllClientAck& data)
			{
				msg.header.id = ServerMsgType::RequestNoticeAllClientAck;
				msg << data.msgtext;
				return msg;
			}

			template <typename T>
			friend message<T>& operator >> (message<T>& msg, RequestNoticeAllClientAck& data)
			{
				msg >> data.msgtext;

				return msg;
			}

	};


struct RequestRoomInfoReq
{
	uint32_t roomid;
	std::string msgtext = "RequestRoomInfoReq";

	friend std::ostream& operator << (std::ostream& os, const RequestRoomInfoReq& msg)
	{
		os << "msgtext:" << msg.msgtext;
		return os;
	}

	template <typename T>
	friend message<T>& operator << (message<T>& msg, const RequestRoomInfoReq& data)
	{
		msg.header.id = ServerMsgType::RequestRoomInfoReq;
		msg << data.msgtext << data.roomid;
		return msg;
	}

	template <typename T>
	friend message<T>& operator >> (message<T>& msg, RequestRoomInfoReq& data)
	{
		msg >>data.roomid >> data.msgtext ;

		return msg;
	}

};

struct RequestRoomInfoAck
{
	uint32_t roomid;
	uint32_t userid;
	uint32_t playercount;
	std::vector<server_Util::PlayerInfo> info;
	std::string msgtext = "RequestRoomInfoAck";

	friend std::ostream& operator << (std::ostream& os, const RequestRoomInfoAck& msg)
	{
		os << "msgtext:" << msg.msgtext;
		return os;
	}

	template <typename T>
	friend message<T>& operator << (message<T>& msg, const RequestRoomInfoAck& data)
	{
		msg.header.id = ServerMsgType::RequestAddPlayerAck;
		msg << data.msgtext << data.roomid << data.userid;
		
		for (auto& player : data.info)
		{
			msg << player;
		}
		msg << data.info.size();
		return msg;
	}

	template <typename T>
	friend message<T>& operator >> (message<T>& msg, RequestRoomInfoAck& data)
	{
		msg  >> data.playercount;
		if(data.playercount > 0)
		{ 
			for (int index = 0; index != data.playercount; ++index)
			{
				server_Util::PlayerInfo player;
				msg >> player;
				data.info.emplace_back(std::move(player));
			}
		}
		return msg >> data.userid >> data.roomid >>  data.msgtext ;
	}

};

struct RequestAddPLayerReq
{
	std::string msgtext = "RequestAddPLayerReq";

	friend std::ostream& operator << (std::ostream& os, const RequestAddPLayerReq& msg)
	{
		os << "msgtext:" << msg.msgtext;
		return os;
	}

	template <typename T>
	friend message<T>& operator << (message<T>& msg, const RequestAddPLayerReq& data)
	{
		msg.header.id = ServerMsgType::RequestAddPLayerReq;
		msg << data.msgtext;
		return msg;
	}

	template <typename T>
	friend message<T>& operator >> (message<T>& msg, RequestAddPLayerReq& data)
	{
		msg >> data.msgtext;

		return msg;
	}
	
};

struct RequestAddPlayerAck
{
	uint32_t id;
	std::string msgtext = "RequestAddPlayerAck";

	friend std::ostream& operator << (std::ostream& os, const RequestAddPlayerAck& msg)
	{
		os << "msgtext:" << msg.msgtext;
		return os;
	}

	template <typename T>
	friend message<T>& operator << (message<T>& msg, const RequestAddPlayerAck& data)
	{
		msg.header.id = ServerMsgType::RequestAddPlayerAck;
		msg << data.msgtext << data.id;
		return msg;
	}

	template <typename T>
	friend message<T>& operator >> (message<T>& msg, RequestAddPlayerAck& data)
	{
		msg >> data.id >> data.msgtext;

		return msg;
	}

};



struct RequestMovePLayerReq
{
	uint32_t id;
	float x, y;
	std::string state;
	std::string msgtext = "RequestMovePLayerReq";

	friend std::ostream& operator << (std::ostream& os, const RequestMovePLayerReq& msg)
	{
		os << "msgtext:" << msg.msgtext;
		return os;
	}

	template <typename T>
	friend message<T>& operator << (message<T>& msg, const RequestMovePLayerReq& data)
	{
		msg.header.id = ServerMsgType::RequestMovePLayerReq;
		msg << data.msgtext << data.id << data.x << data.y << data.state;
		return msg;
	}

	template <typename T>
	friend message<T>& operator >> (message<T>& msg, RequestMovePLayerReq& data)
	{
		msg >> data.state >> data.y >> data.x >> data.id >> data.msgtext ;

		return msg;
	}
};

struct RequestMovePlayerAck
{
	uint32_t count;
	std::vector<server_Util::PlayerInfo> info;
	std::string msgtext = "RequestMovePlayerAck";

	friend std::ostream& operator << (std::ostream& os, const RequestMovePlayerAck& msg)
	{
		os << "msgtext:" << msg.msgtext;
		return os;
	}

	template <typename T>
	friend message<T>& operator << (message<T>& msg, const RequestMovePlayerAck& data)
	{
		msg.header.id = ServerMsgType::RequestMovePlayerAck;
		msg << data.msgtext;
		for (auto player : data.info)
		{
			msg << player;
		}
		msg << info.size();
		return msg;
	}

	template <typename T>
	friend message<T>& operator >> (message<T>& msg, RequestMovePlayerAck& data)
	{
		msg >>data.count ;
		for (int i = 0; i != count; ++i)
		{
			server_Util::PlayerInfo player;
			msg >> player;
			data.info.emplace_back(std::move(player));
		}

		return msg >> data.msgtext;
	}

};


struct SendPlayerMovement
{
	uint32_t id;
	float x, y;
	std::string state;
	std::string msgtext = "RequestMovePlayerAck";

	friend std::ostream& operator << (std::ostream& os, const SendPlayerMovement& msg)
	{
		os << "msgtext:" << msg.msgtext;
		return os;
	}

	template <typename T>
	friend message<T>& operator << (message<T>& msg, const SendPlayerMovement& data)
	{
		msg.header.id = ServerMsgType::SendPlayerMovement;
		msg << data.msgtext << data.id << data.x << data.y << data.state;
		return msg;
	}

	template <typename T>
	friend message<T>& operator >> (message<T>& msg, SendPlayerMovement& data)
	{
		msg >> data.state >>  data.y >> data.x >> data.id >> data.msgtext;

		return msg;
	}

};



}
#endif // __MSG_DETAIL_H__
