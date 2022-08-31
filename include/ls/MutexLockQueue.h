#ifndef LS_MUTEXLOCKQUEUE_H
#define LS_MUTEXLOCKQUEUE_H

#include "mutex"
#include "ls/Queue.h"
#include "ls/Exception.h"

namespace ls
{
	template<typename T>
	class Node
	{
		public:
			Node() : next(nullptr)
			{
			
			}
			
			Node(const T& val) : val(val), next(nullptr)
			{
			
			}

			T val;
			Node *next;
	};

	template<typename T>
	class MutexLockQueue : public Queue<T>
	{
		public:
			MutexLockQueue()
			{
				head = tail = new Node<T>();
			}

			~MutexLockQueue() override
			{
				while(!empty())
				{
					auto tmp = head -> next;
					head -> next = head -> next -> next;
					delete tmp;
				}
				delete head;
			}

			int push(const T& val)
			{
				std::lock_guard<std::mutex> lock_(mtx);
				this -> tail -> next = new Node<T>(val);
				this -> tail = this -> tail -> next;
				return Exception::LS_OK;
			}

			T pop(int &ec)
			{
				ec = Exception::LS_OK;
				Node<T> *tmp;
				{
					std::lock_guard<std::mutex> lock_(mtx);
					if(empty())
					{
						ec = Exception::LS_ENOCONTENT;
						return T(0);
					}
					tmp = this -> head -> next;
					this -> head -> next = this -> head -> next -> next;
				}
				auto result = tmp -> val;
				delete tmp;
				return result;
			}

			void reset()
			{
				int ec;
				while(!empty())
					pop(ec);
			}
		protected:
			bool empty()
			{
				return head -> next == nullptr;
			}
			std::mutex mtx;
			Node<T> *head;
			Node<T> *tail;
	};
}

#endif
