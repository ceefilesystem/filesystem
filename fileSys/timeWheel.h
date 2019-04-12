#pragma once
#include<list>
#include<mutex>

typedef void(*Fun)(void);
typedef int timerID;

typedef struct TimePos
{
	int ms_pos;
	int s_pos;
	int min_pos;
}TimePos;

typedef struct EventInfo
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
	/*step 以毫秒为单位，表示定时器最小时间粒度
	 *max_timer 表示定时器所能接受的分钟时间间隔
	 */
	int InitTimerWheel(int step, int max_min);
	timerID AddTimer(int interval, Fun call_back);
	int DeleteTimer(timerID timer_id);

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
extern timerID addTimerWheel(TimeWheel* tw, int interval, Fun callBack);
extern int deleteTimer(TimeWheel* tw, timerID timerId);
extern timerID updateTimer(TimeWheel* tw, timerID timerId, int interval, Fun callBack);
