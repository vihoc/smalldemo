#ifndef __NETMESSAGE_H__
#define __NETMESSAGE_H__


#include "NetCommon.h"

namespace netCommon
{
	template <typename T>
	struct Message_Header
	{
		T id{};
		uint32_t size = 0;
	}; // end of struct Message_Header

	template<typename T>
	struct message
	{
		using self_type = message<T>;
		using header_type = Message_Header<T>;
		header_type header{};
		std::vector<uint8_t> body;

		size_t size() const
		{
			return /*sizeof(header_type) +*/ body.size();
		}

		friend std::ostream& operator << (std::ostream& os, const self_type& msg)
		{
			os << "ID:" << int(msg.header.id) << " SIZE: " << msg.header.size;
			return os;
		}

		template <typename DataType>
		friend self_type& operator << (self_type& msg, const DataType& data)
		{
			static_assert(std::is_standard_layout_v<DataType>, "Data is too complex to insert");

			size_t pos = msg.body.size();

			msg.body.resize(msg.body.size() + sizeof(DataType));
			std::memcpy(msg.body.data() + pos, &data, sizeof(DataType));
			msg.header.size = msg.size();

			return msg;
		}

		template <typename DataType>
		friend self_type& operator >> (self_type& msg, DataType& data)
		{
			static_assert(std::is_standard_layout_v<DataType>, "Data is too complex to insert");

			size_t pos = msg.body.size() - sizeof(DataType);
			std::memcpy(&data, msg.body.data() + pos, sizeof(DataType));
			msg.body.resize(pos);
			
			msg.header.size = msg.size();

			return msg;
		}
	}; //end of struct message


	template <typename T>
	class Connection;

	template <typename T>
	struct owned_Message
	{
		using self_type = owned_Message<T>;
		using message_type = message<T>;
		using connection_type = Connection<T>;

		std::shared_ptr<Connection<T>> remote = nullptr;
		message_type msg;

		friend std::ostream& operator <<(std::ostream& os, const self_type& msg)
		{
			os << msg.msg;
			return os;
		}
	};


} //end of name space netCommon

#endif
