#include "ls/MutexLockQueue.h"
#include "ls/CASQueue.h"
#include "iostream"
#include "thread"
#include "unistd.h"
#include "queue"
#include "ls/time/API.h"
#include "ls/DefaultLogger.h"
#include "condition_variable"
#include "mutex"

using namespace std;
using namespace ls;

const int data_size = 8000000;
//	测试互斥锁队列
MutexLockQueue<int> mlq;
atomic<int> cnt(0);

void MLQWriter(int n)
{
	long long start_time = time::api.getCurrentMS();
	for(int i=0;i<n;++i)
		mlq.push(i);
	long long end_time = time::api.getCurrentMS();
	cout << "time: " << end_time - start_time << endl;
}

void MLQReader(int n)
{
	long long start_time = time::api.getCurrentMS();
	for(;;)
	{
		int ec;
		mlq.pop(ec);
		if(ec < 0)
		{
			if(cnt.load() == n*4)
				break;
			usleep(5);
			continue;
		}
		cnt.fetch_add(1);
	}
	long long end_time = time::api.getCurrentMS();
	cout << "time: " << end_time - start_time << endl;
}

void testMLQ()
{
	long long start_time = time::api.getCurrentMS();
	thread tv[8];
	for(int i=0;i<4;++i)
		tv[i] = thread(MLQWriter, data_size);
	for(int i=4;i<8;++i)
		tv[i] = thread(MLQReader, data_size);
	for(int i=0;i<8;++i)
		tv[i].join();
	long long end_time = time::api.getCurrentMS();
	LOGGER(ls::INFO) << "mlq time: " << end_time - start_time << ls::endl;
}

//	测试无锁队列(使用自旋锁来进行重置队列状态时的线程同步)

struct TMP
{
	int val;
	int t;
};

CASQueue<TMP *> casq(800000);
int arr[data_size * 6];

condition_variable cv;
mutex mtx;
atomic<int> status[8];

void CASWriter(int n)
{
	int i = 0;
	for(i=0;i<data_size;++i)
	{
		TMP* tmp = new TMP();
		tmp -> val = i + data_size * n;
//		casq.push(i+data_size*n);
		int ec = casq.push(tmp);
		if(ec < 0)
		{
			if(ec == Exception::LS_ERESET)
			{
				unique_lock<mutex> lc(mtx);
//				cout << "reading locki" << endl;
				status[n]++;
				cv.wait(lc);
				status[n]--;
			}
			--i;
			continue;
		}
	}
	status[n] = 2;
}


int check()
{
	int ct = 0;
	for(int i=0;i<4*data_size;++i)
		if(arr[i] != 1)
		{
			++ct;
		}
	cout << "ct: " << ct << endl;
	return ct == 0;
}


void CASReader(int n)
{
	long long start_time = time::api.getCurrentMS();
	for(;;) 
	{
		int ec;
		auto tmp = casq.pop(ec);
		if(ec < 0)
		{
			/*
		*/
			if(ec == Exception::LS_ERESET)
			{
				unique_lock<mutex> lc(mtx);
				status[n]++;
				cv.wait(lc);
				status[n]--;
			}
			if(check())
			{
				status[n] = 2;
				break;
			}
			usleep(5);
			continue;
		}
		arr[tmp -> val]++;
	}
	long long end_time = time::api.getCurrentMS();
	cout << "time: " << end_time - start_time  << "ms" << endl;
}

void shouhu()
{
		for(;;)
		{
			bool isok = true;
			for(int i=0;i<8;++i)
				if(status[i].load() == 0)
				{
					isok = false;
					break;
				}
			int j;
			for(j=0;j<8;++j)
				if(status[j].load() != 2)
					break;
			if(j == 8)
				break;	
			if(isok)
			{
				casq.reset();
				for(;;)
				{
					bool notify_ok = true;
					cv.notify_all();
			/*		
					for(int i=0;i<8;++i)
						if(status[i].load()==1)
						{
							notify_ok = false;
							usleep(5);
							break;
					
						}
			*/			
					break;
				}
			}
	//		cout << "failed" << endl;
	//		for(int i=0;i<8;++i)
	//			cout << status[i] << endl;
			usleep(5);
		}
}

void testCASQ()
{
	long long start_time = time::api.getCurrentMS();
	vector<thread> tv(9);
	for(int i=0;i<4;++i)
	{
		tv[i] = thread(CASWriter, i);
	}
	for(int i=4;i<8;++i)
	{
		tv[i] = thread(CASReader, i);
	}
	tv[8] = thread(shouhu);
	tv[8].detach();
	for(int i=0;i<8;++i)
		tv[i].join();
	long long end_time = time::api.getCurrentMS();
	LOGGER(ls::INFO) << "casq time: " <<  end_time - start_time << "ms" << ls::endl;
}

//	测试标准队列

queue<int> q;

void testQueue(int n)
{
	long long start_time = time::api.getCurrentMS();
	for(int i=0;i<n;++i)
		q.push(i);
	for(int i=0;i<n;++i)
		q.pop();
	long long end_time = time::api.getCurrentMS();
	LOGGER(ls::INFO) << "normalq time: " << end_time - start_time << ls::endl;
}

int main()
{
	
	testCASQ();
//	testMLQ();
//	testQueue(data_size * 4);
	return 0;
}
