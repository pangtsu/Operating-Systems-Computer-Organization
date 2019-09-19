#include "p.h"


/* states:

        not arrived - process has not arrived in the ready queue
        ready - process in ready queue
        switching - process is either switching in to the cpu or switching out from cpu
        running - running in the cpu
        blocked - process goes to the I/O state
        done - current process done
*/

Process_::Process_(int id_, int arrival_, int burst_num1, const vector<int>& io_b1, const vector<int>& cpu_b1, int init_tau){
        arrival = arrival_;
        char answer = 'A';
        if(id_ == 0){
            proc_id = answer;
        }
        else{
            answer += id_;
            proc_id = answer;
        }
        completed = 0;
        waitTime = 0;
        waitTimeStart = -1;
        timeRemain = 0;
        preemption= 0;
        currentPreempt= false;
        turnaroundStart = -1;
        elapsedTime = 0;
        state = "not arrived"; 
        burstComplete = 0;  
        startTime = 0;   
        io_b = io_b1;
        cpu_b = cpu_b1;
        burst_num = burst_num1;
        tau = init_tau;
}



 void Process_::reset(float lambda){
        completed = 0;
        waitTime = 0;
        timeRemain = 0;
        startTime = 0;
        tau = ceil(1/lambda);
        currentPreempt = false;
        preemption = 0;
        state = "not arrived";
        turnaroundStart = -1;
        burstComplete = 0;
    }

void Process_::update_tau(float alpha, int num) {
    int new_tau =ceil((alpha * num) + ((1-alpha) * tau));
    tau = new_tau;
    }  