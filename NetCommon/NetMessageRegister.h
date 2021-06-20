#ifndef __NETMESSAGEREGISTER_H__
#define __NETMESSAGEREGISTER_H__

#include <vector>
#include <map>

namespace netCommon
{


	class HandlerBase
	{
	public:
		virtual ~HandlerBase() {}

		virtual bool operator()(void* pdata) = 0;

		virtual void* GetThisAddr() = 0;

		virtual void EmptyThisAddr() = 0;
	};

	template<typename TClass, typename TParam>
	class HandlerSlot : public HandlerBase
	{
		typedef BOOL(TClass::* FuncType)(TParam*);
	public:
		HandlerSlot(BOOL(TClass::* FuncType)(TParam*), TClass* pObj)
			: m_pFuncPtr(FuncType), m_pThis(pObj)
		{
		}
		virtual ~HandlerSlot()
		{
		}

		virtual BOOL operator() (void* pData)
		{
			if (m_pThis != NULL && m_pFuncPtr != NULL)
			{
				(m_pThis->*m_pFuncPtr)(reinterpret_cast<TParam*>(pData));
				return true;
			}
			else
			{
				return false;
			}
		}

		virtual VOID* GetThisAddr()
		{
			return reinterpret_cast<VOID*>(m_pThis);
		}

		virtual void EmptyThisAddr()
		{
			m_pThis = NULL;
		}


	private:
		FuncType m_pFuncPtr;
		TClass* m_pThis;
	};

	class HandlerManager
	{
	public:
		HandlerManager()
		{
		}

		~HandlerManager()
		{
			ClearAll();
		}

		template<typename TClass, typename TParam>
		bool RegisterMessageHandle(int nMsgID, bool(TClass::* FuncPtr)(TParam*), TClass* pObj)
		{
			HandlerBase* pSlot = new HandlerSlot<TClass, TParam>(FuncPtr, pObj);
			if (pSlot == NULL)
			{
				return false;
			}

			std::vector<HandlerBase*>* pMsgVector = NULL;
			std::map<int, std::vector<HandlerBase* >* >::iterator it = m_mapHandler.find(nMsgID);
			if (it == m_mapHandler.end())
			{
				pMsgVector = new std::vector<HandlerBase*>;
				pMsgVector->push_back(pSlot);
				m_mapHandler.insert(std::make_pair(nMsgID, pMsgVector));
			}
			else
			{
				pMsgVector = it->second;
				pMsgVector->push_back(pSlot);
			}

			return true;
		}

		template<typename T>
		bool UnregisterMessageHandle(int nMsgID, T* pObj)
		{
			std::map<int, std::vector<HandlerBase* >* >::iterator itor = m_mapHandler.find(nMsgID);
			if (itor == m_mapHandler.end())
			{
				return true;
			}

			std::vector<HandlerBase* >* vtHandler = itor->second;

			for (std::vector<HandlerBase* >::iterator itor = vtHandler->begin(); itor != vtHandler->end(); ++itor)
			{
				if ((*itor)->GetThisAddr() != reinterpret_cast<void*>(pObj))
				{
					continue;
				}

				//反注册并不真实删除，只是将this指针置空
				(*itor)->EmptyThisAddr();
			}
			return true;
		}
		template<typename TParam>
		bool ExeMessage(int nMsgID, TParam* pData)
		{
			std::map<int, std::vector<HandlerBase* >* >::iterator itor = m_mapHandler.find(nMsgID);
			if (itor == m_mapHandler.end())
			{
				return false;
			}

			std::vector<HandlerBase* >* vtHandler = itor->second;

			for (std::vector<HandlerBase* >::iterator itorHandler = vtHandler->begin(); itorHandler != vtHandler->end(); itorHandler++)
			{
				if ((*itorHandler)->GetThisAddr() != 0)
				{
					(**itorHandler)((void*)pData);
				}
			}
			return true;
		}


		bool ClearAll()
		{
			for (std::map<int, std::vector<HandlerBase*>*>::iterator itor = m_mapHandler.begin(); itor != m_mapHandler.end(); itor++)
			{
				std::vector<HandlerBase* >* pVtHandler = itor->second;
				for (std::vector<HandlerBase* >::iterator itor2 = pVtHandler->begin(); itor2 != pVtHandler->end(); itor2++)
				{
					delete* itor2;
				}

				pVtHandler->clear();

				delete pVtHandler;
			}

			m_mapHandler.clear();

			return true;
		}

	protected:
		std::map<int, std::vector<HandlerBase* >* > m_mapHandler;
	};


	class CMsgHandlerManager : public HandlerManager
	{
	private:
		CMsgHandlerManager(void) {};
		virtual ~CMsgHandlerManager(void) {};

	public:
		static CMsgHandlerManager* GetInstancePtr()
		{
			static CMsgHandlerManager _Instance;

			return &_Instance;
		}
	};

}//end of namespace netcommon

#endif // __NETMESSAGEREGISTER_H__