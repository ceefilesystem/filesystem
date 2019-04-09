#pragma once
#include<functional>
#include<list>
#include<thread>
#include<mutex>

typedef struct TimePos_
{
	int ms_pos;
	int s_pos;
	int min_pos;
}TimePos;

typedef void (*Fun)(void);

typedef struct EventInfo_
{
	int interval;
	Fun call_back;
	TimePos time_pos;
	int timer_id;

}EventInfo;

class TimeWheel
{
public:
	TimeWheel();
	~TimeWheel();
public:
	/*step �Ժ���Ϊ��λ����ʾ��ʱ����Сʱ������
	 *max_timer ��ʾ��ʱ�����ܽ��ܵķ���ʱ����
	 */
	int InitTimerWheel(int step, int max_min);
	int AddTimer(int interval, Fun call_back);
	int DeleteTimer(int timer_id);

private:
	int DoLoop();
	int GenerateTimerID();
	int InsertTimer(int diff_ms, EventInfo& einfo);
	int GetNextTrigerPos(int interval, TimePos& time_pos);
	int GetMS(TimePos time_pos);
	int DealTimeWheeling(std::list<EventInfo> leinfo);
private:
	std::list<EventInfo> *_pCallbackList = nullptr;
	std::mutex _mutex;

	TimePos _time_pos;

	int _lowCount = 0;
	int _midCount = 0;
	int _highCount = 0;

	int _step_ms = 0;

	int _timer_count = 0;
};

extern TimeWheel* createTimerWheel(int step, int max_min);
extern int addTimerWheel(TimeWheel* tw, int interval, Fun callBack);
extern int deleteTimer(TimeWheel* tw, int timerId);
extern int updateTimer(TimeWheel* tw, int timerId, int interval, Fun callBack);
