#ifndef __MSG_DETAIL_H__
#define __MSG_DETAIL_H__
#include "BaseHeader.h"

namespace netCommon
{ 

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
		msg << data.msgtext << data.seed;
		return msg;
	}

	template <typename T>
	friend message<T>& operator >> (message<T>& msg, RequestServerSeedack& data)
	{
		msg >> data.msgtext >> data.seed;

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



}
#endif // __MSG_DETAIL_H__
