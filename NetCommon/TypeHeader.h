#ifndef __TYPEHEADER_H__
#define __TYPEHEADER_H__

#include "NetCommon.h"

namespace netCommon
{
	namespace Type
	{
		enum class CustomMsgType : uint32_t
		{
			FireBullet,
			MovePlayer
		};


		
		enum class ServerMsgType : uint32_t
		{
			ServerAccept,
			ServerDeny,
			ServerPing,
			MessageAll,
			Client_Accepted,
			//begin user message
			User_Loginin,
			//end user message

			//begin 
			
			//end

			HeartBeat,
			HeartBeatStart,
			ServerMessage
		};

		enum class GameMsgType : uint32_t
		{
			User_Loginin
		};


		enum class DemoMsgType : uint32_t
		{
			ServerAccept,
			ServerDeny,
			Client_Accepted,
			HeartBeat,
			HeartBeatStart,
			RemovePLayer,

			RequestRoomInfoReq,
			RequestRoomInfoAck,
			RequestServerSeedReq,
			RequestServerSeedAck,
			RequestAddPLayerReq,
			RequestAddPlayerAck,
			RequestMovePLayerReq,
			RequestMovePlayerAck,
			SendPlayerMovement // for now

		};
	}
}


#endif