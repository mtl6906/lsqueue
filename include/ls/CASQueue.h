#ifndef LS_CASQUEUE_H
#define LS_CASQUEUE_H

#include "ls/Queue.h"
#include "ls/Exception.h"
#include "atomic"
#include "memory"
#include "unistd.h"
#include "iostream"
#include "ls/DefaultLogger.h"

namespace ls
{
	template<typename T>
	class CASQueue : public Queue<T>
	{
		public:

			CASQueue(int _n) : n(_n), head(-1), tail(0)
			{
				data = (std::atomic<T> *)operator new (sizeof(std::atomic<long long>) * n);
				for(int i=0;i<n;++i)
					new (data + i)std::atomic<T>();
				changed = (std::atomic<bool> *)operator new(sizeof(std::atomic<bool>) * n);
				for(int i=0;i<n;++i)
					new (changed + i)std::atomic<bool>();
			}

			~CASQueue()
			{
				for(int i=0;i<n;++i)
				{
					((std::atomic<T> *)(data + i)) -> ~atomic();
					((std::atomic<bool> *)(changed + i)) -> ~atomic();
				}
				delete data;
				delete changed;
			}

			bool empty()
			{
				return (head + 1) % n == tail;
			}

			bool full()
			{
				return (tail + 1) % n == head;
			}

			int size()
			{
				return tail - head - 1;
			}

			void show()
			{
				for(int i=0;i<n;++i)
					std::cout << i <<  ": " << (data[i] & 0xffffffff) << std::endl;
			}

			int push(const T& val)
			{
				int ec = Exception::LS_OK;
				int old_tail, old_head;
				do
				{
					old_tail = tail.load();
					old_head = head.load();
					if(old_tail + 1 == n + 1)
						return Exception::LS_ERESET;
				}
				while(!tail.compare_exchange_strong(old_tail, old_tail+1));
				data[old_tail].store(val);
				changed[old_tail].store(true);
				return ec;
			}

			T pop(int &ec)
			{
				ec = Exception::LS_OK;
				int old_head, old_tail;
				do
				{
					old_head = head.load();
					old_tail = tail.load();
					if(old_head + 1 == old_tail)
					{
						if(old_tail + 1 == n + 1)
						{
							ec = Exception::LS_ERESET;
							return T(0);
						}
						ec = Exception::LS_ENOCONTENT;
						return T(0);
					}
				}
				while(!head.compare_exchange_strong(old_head, old_head+1));
				LOGGER(ls::INFO) << old_head << " " << old_tail << ls::endl;
				T result(data[old_head + 1].load());
				bool this_changed = changed[old_head+1];
				while(this_changed == false)
				{
					this_changed = changed[old_head+1].load();
					result = data[old_head+1].load();
					usleep(5);
				}
				return data[old_head + 1];
			}

			void reset()
			{
				for(int i=0;i<n;++i)
					changed[i] = false;
				head.store(-1);
				tail.store(0);
			}
		public:
			int n;
			std::atomic<bool> *changed;
			std::atomic<T> *data;
			std::atomic<int> head;
			std::atomic<int> tail;
			T nullss;
	};
}

#endif
