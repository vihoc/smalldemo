#ifndef __COMMONMACRO_H__
#define __COMMONMACRO_H__

#define BEGIN_PROCESS_MESSAGE(msg)\
		switch((msg))\
		{

#define PROCESS_MESSAGE_ITEM(dwMsgID, Func, Client, Msg) \
		case (dwMsgID):{\
		if(Func(Client, Msg)){return true;}}break;

#define PROCESS_MESSAGE_ITEMBC(dwMsgID, Func, msg) \
		case (dwMsgID):{\
		if(Func(msg)){return true;}}break;

#define PROCESS_MESSAGE_ITEMEX(dwMsgID, Func) \
		case (dwMsgID):{\
		if(Func(pNetPacket)){return true;}}break;

#define END_PROCESS_MESSAGE \
		default: \
			{ return false;   break;} \
	}\
	return false;


#endif __COMMONMACRO_H__