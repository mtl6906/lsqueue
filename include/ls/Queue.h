#ifndef LS_QUEUE_H
#define LS_QUEUE_H

namespace ls
{
	template<typename T>
	class Queue
	{
		public:
			virtual ~Queue() {}
			virtual int push(const T& val) = 0;
			virtual T pop(int &ec) = 0;
			virtual void reset() = 0;
			virtual int size() {return 0;}
	};
}

#endif
