#ifndef __process__h_
#define __process__h_
#include <cassert>
#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <list>    
using namespace std;
// ====================================================================


class Process_ {
public:
   Process_(int id_, int arrival_, int burst_num1, const vector<int>& io_b1, const vector<int>& cpu_b1, int init_tau);
  
    // accessor
    int get_arrival() const { return arrival; }
    int get_num() const { return burst_num; }
    string get_state() {return state;}
    char get_id() const {return proc_id;}
 	int get_tau() const {return tau;}
    int get_turnaround() const {return turnaroundStart;}
    int get_time_elapsed() const {return elapsedTime;}
    int get_completed() const {return completed;}
    int get_burst_completed() const {return burstComplete;}
    int get_waittime() const {return waitTime;}
    int get_waittime_start() const {return waitTimeStart;}
    int get_starttime() const {return startTime;}
    int get_time_remain() const {return timeRemain;}
    int get_preemption() const {return preemption;}
    bool get_current_preemption() const {return currentPreempt;}
    int get_cpu_complete(int complete) {return cpu_b[complete];}
   

    // modifier
    void setState(string S) { state = S; }
    void setStartTime(int x) { startTime = x; }
    void setWaitTime(int x) { waitTime = x; }
    void setWaitTimeStart(int x) { waitTimeStart = x; }
    void setTurnaround(int x) { turnaroundStart = x; }
    void setRemain(int x) { timeRemain = x; }
    void setElapsed(int x) { elapsedTime = x; }
    void setBurstCompleted(int x){burstComplete = x;}
    void setCompleted(int x){completed= x;}
    void setPreemption(int x){preemption = x;}
    void setCurrentPreemption(bool x){currentPreempt = x;}
    void setTau(int x){tau = x;}


    // others
    void reset(float lambda); 
    void update_tau(float alpha, int num);                                                                           
   
private:
    int arrival; // the arrival time
    char proc_id; // the process id
    int burst_num; // # of bursts for this particular process
    int completed; // # of bursts the process has completed
    int waitTime; // waittime
    int waitTimeStart; 
    int timeRemain; 
    int startTime;
    int tau;
    int preemption; // # of preemptions
    bool currentPreempt; // whether there's preemption
    int turnaroundStart;
    vector<int> cpu_b; // the cpu_burst times vector
    vector<int> io_b;
    int elapsedTime; // current elapsed time
    string state;
    int burstComplete; // completed bursts of this process

    };
#endif
