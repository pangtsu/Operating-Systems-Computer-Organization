#include <cassert>
#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <deque>          
#include <list>          
#include <queue> 
#include "p.h"
using namespace std;

/*
void update_tau(Process * current, float alpha, int num, int old_tau){ 
        int tau =ceil((alpha * num) + ((1-alpha) * old_tau));
    //    int x = which_process(processes_, current);
        current->setTau(tau); 
    }
*/

// determines the index of the processes_ the "current" is pointing to in order to get the referenve
int which_process(vector<Process_> processes_, list<Process_>::iterator itr){
    int i=0;
    for (i = 0; i<processes_.size(); i++){
        if (processes_[i].get_id() == itr->get_id()){
            return i;
        }
    }
    return i;
}

// prints contents in the queue
string print(list<Process_> queue){
    if (queue.size() == 0){
        return "[Q <empty>]";
    }
    else{
        string start = "[Q ";
        for (list<Process_>::iterator i = queue.begin(); i != queue.end(); i++){
            if (i == prev(queue.end())){
                start += string(1,i->get_id()) + "]";
            }
            else{
                start += string(1,i->get_id()) + " ";
            }
        }
        return start;
    }
}

// simout.txt output
void write(ofstream& sim, const vector<float> & answer, string algo){
    float averageburst = answer[0];
    float averagewait = answer[1];
    float averageturn = answer[2];
    int contextswitch = round(answer[3]);
    int preemption = round(answer[4]);

    sim << "Algorithm " << algo << "\n";
    sim << "-- average CPU burst time: " << averageburst << " ms\n";
    sim << "-- average wait time: " << averagewait << " ms\n";
    sim << "-- average turnaround time: " << averageturn << " ms\n";
    sim << "-- total number of context switches: " << round(contextswitch) << "\n";
    sim << "-- total number of preemptions: " << round(preemption) << "\n";  
}

float total_burst(const vector<Process_>& processes_){
    float total_burst = 0;
    for(int i = 0 ; i < processes_.size(); i ++){
        total_burst += (float)(processes_[i].get_num());
    }
    return total_burst;
}



vector<float> sjf(vector<Process_>& processes_, int switch_duration, float alpha){

    // our ready queue
    list<Process_> queue;
    // current process, pointing to the element in the vector processes_ by reference
    Process_ * current = NULL;

    int t = 0; // keep track of time
    unsigned int completed = 0; // # of processes_ completed
    
    int out_time = -1; // the time we switchout
    int in_time = -1; // the time we switch in
    string s; 

    // variables to keep track
    vector<float> result; // the final average output
    int total_burst_time = 0;
    int total_wait = 0;
    int total_switch = 0;
    int total_preempts = 0;

    int is_it_switching_out = false; // switch_duration/2
    int is_it_switching_in = false; // switch_duration/2
    cout << "time 0ms: Simulator started for SJF [Q <empty>]\n";

    // while loop for time
    while(true){ 

        /*
        not arrived - process has not arrived in the ready queue
        ready - process in ready queue
        switching - process is either switching in to the cpu or switching out from cpu
        running - running in the cpu
        blocked - process goes to the I/O state
        done - current process done



        */
        // process finishes switching out, goes to blocked state
        if(current != NULL && is_it_switching_out && (t == out_time + switch_duration/2)){
           // cout << "000\n";
        //    int q = which_process(processes_,current);
             
            if(current->get_state() == "switching"){
          //      cout << "0.5\n";
              
                current->setState("blocked");
                current->setStartTime(t); 
                current->setTurnaround(-1);
            }
            if(current->get_state() == "done"){ // done with the block state
            }
            current = NULL;
            is_it_switching_out = false;
            out_time = -1;
        }


        // process is running
        if (current != NULL && current->get_state() != "switching" && current->get_state() != "done"){
            int x = current->get_cpu_complete(current->get_completed());
            if((current->get_time_elapsed() == x) && current->get_state() == "running"){

                is_it_switching_out = true;
                out_time = t;
                int d = current->get_cpu_complete(current->get_completed());
                total_burst_time += d;
                current->setBurstCompleted((current->get_burst_completed())+1);
                current->setCompleted((current->get_completed())+1);
                current->setCurrentPreemption(false);
                current->setRemain(0);
                current->setElapsed(0);
                total_wait += current->get_waittime();
                current->setWaitTime(0);

                // if completed all bursts
                if(current->get_burst_completed() == current->get_num()){

                    s = print(queue);
                    cout << "time " << t << "ms: Process " << string(1,current->get_id()) << " terminated " << s << endl;
                    current->setState("done");
                    completed += 1;
                    if (completed == processes_.size()){
                        t += switch_duration/2;
                        break;
                    }
                }
                else{
                    if(t<9969){
                        if (current->get_num() - current->get_burst_completed() == 1){


                            s = print(queue);
                            cout << "time " << t << "ms: Process " << string(1,current->get_id()) << " completed a CPU burst; 1 burst to go " << s << endl;
                        }
                        else{
                            cout << "time " << t << "ms: Process " << string(1,current->get_id()) << " completed a CPU burst; " << current->get_num() - current->get_burst_completed() <<" bursts to go " << s << endl;
                        }
                    }
                   // turnaroundTimeTotal += w;
                    current->setState("switching");


                    // starts I/O
                    int num = current->get_cpu_complete((current->get_completed())-1);

                    float old = current->get_tau();
                    float new_tau = (alpha * (float)num) + ((1-alpha) * old);
                    
                    int new_one = ceil(new_tau);
                

                    current->setTau(new_one);

                    if(t<9969){
                    s = print(queue);
                    cout << "time " << t << "ms: Recalculated tau = " << current->get_tau() << "ms for process " << string(1,current->get_id()) << " " << s << endl;
                    int m = current->get_cpu_complete(current->get_completed());
                    cout << "time " << t << "ms: Process " << string(1,current->get_id()) << " switching out of CPU; will block on I/O until time " << t+m << "ms " << s << endl;
                    // turnaroundTimeTotal += switch_duration/2;
                    }
                }
            }
        }
       
       // process finishes switching in, goes to the cpu
        if(is_it_switching_in && (t == in_time + switch_duration/2)){
            // end context switch in cpu start; 

            current->setState("running");

            if (t<9969){
                if(current->get_time_elapsed() > 0){
               //     cout << "222.333\n";
                    s = print(queue);
                    int y = current->get_cpu_complete(current->get_completed());
                    cout << "time " << t << "ms: Process " << string(1,current->get_id()) << " started using the CPU with " << (y - current->get_time_elapsed()) << "ms remaining " << s << endl;
                }
                else{
                    s = print(queue);
                    int y = current->get_cpu_complete(current->get_completed());
                    cout << "time " << t << "ms: Process " << string(1,current->get_id()) << " started using the CPU for " << y << "ms bursts " << s << endl;
                }
            }

            current->setStartTime(t);
          //  turnaroundTimeTotal += switch_duration/2;
            is_it_switching_in = false;
            in_time = -1;
            total_switch += 1;
        }

        // checks for I/O
        int i = 0;
        for (i = 0; i< processes_.size(); i++){
           int z = (processes_[i]).get_cpu_complete(processes_[i].get_completed());
            if((processes_[i].get_state() == "blocked") && (t == (processes_[i]).get_starttime() + z - switch_duration/2)){

                processes_[i].setCompleted((processes_[i].get_completed())+1);
                if(processes_[i].get_turnaround() == -1){
                    processes_[i].setTurnaround(t);
                }
                // i/O FINISH...
                for (list<Process_>::iterator k = queue.begin(); k != queue.end(); k++){
                    if((processes_[i].get_tau() - processes_[i].get_time_elapsed() < k->get_tau() - k->get_time_elapsed()) || ((processes_[i].get_tau() - processes_[i].get_time_elapsed() == k->get_tau() - k->get_time_elapsed()) && processes_[i].get_id() < k->get_id())){
               //          cout << "555, time is =" << t << "\n";

                        processes_[i].setState("ready");
                        queue.insert(k, processes_[i]);
                        break;
                    }
                }
                if(processes_[i].get_state() == "blocked"){
                    processes_[i].setState("ready");
                    queue.push_back(processes_[i]);
                }
                if(t<9969){
                    s = print(queue);
                  // cout << "777, time is =" << t << "\n";
                    cout << "time " << t << "ms: Process "<< string(1, processes_[i].get_id()) << " (tau " << processes_[i].get_tau() << "ms) completed I/O; added to ready queue " << s << endl;

                }
            }
        }


        // checks for process arrival
        int j = 0;
         for (j = 0; j< processes_.size(); j++){
            if(t == processes_[j].get_arrival() && processes_[j].get_state() == "not arrived"){

                // i/O FINISH
                for (list<Process_>::iterator l = queue.begin(); l != queue.end(); l++){
                    if((processes_[j].get_tau() - processes_[j].get_time_elapsed() < l->get_tau() - l->get_time_elapsed()) || ((processes_[j].get_tau() - processes_[j].get_time_elapsed() == l->get_tau() - l->get_time_elapsed()) && processes_[j].get_id() < l->get_id())){
                        processes_[j].setState("ready");
                        queue.insert(l, processes_[j]);
                        break;
                    }
                }
                if(processes_[j].get_state() == "not arrived"){
                    processes_[j].setState("ready");
                    queue.push_back(processes_[j]);
                }
                if(t<9969){
                    s = print(queue);
                    cout << "time " << t << "ms: Process "<< string(1, processes_[j].get_id()) << " (tau " << processes_[j].get_tau() << "ms) arrived; added to ready queue " << s << endl;
                }
            }
        }

        // finds the next process to run
        if(current == NULL && queue.size() > 0){

            // start a context switch in...
            int which = which_process(processes_, queue.begin());
            current = &(processes_[which]);
            queue.pop_front();
            current->setState("switching");
            is_it_switching_in = true;
            in_time = t;

        }

        // increment elapsed time
        if(current != NULL && current->get_state() == "running"){
            current->setElapsed((current->get_time_elapsed()) + 1);
        }


        // increment wait time everytime a process is in ready state.
        for (int h = 0; h< processes_.size(); h++){
            if(processes_[h].get_state() == "ready"){
                processes_[h].setWaitTime((processes_[h].get_waittime())+1);
            }
        }

        t += 1;
    }
    cout << "time " << t << "ms: Simulator ended for SJF [Q <empty>]\n";

    // calclate the averages and return them.
    float total_b = total_burst(processes_);
    float averageCPUBurst = roundf((total_burst_time/total_b)*9969)/9969;
    float averageWait = roundf((total_wait/total_b)*9969)/9969;

    // turnaround is simply the total wait time + total burst time (duration) + total context switch times / number of burst times
    float averageturn = roundf(((total_wait+total_burst_time+total_switch*switch_duration)/total_b)*9969)/9969;
    result.push_back(averageCPUBurst);
    result.push_back(averageWait);
    result.push_back(averageturn);
    result.push_back((float)total_switch);
    result.push_back((float)total_preempts);

    return result;
}

vector<float> srt(vector<Process_>& processes_, int switch_duration, float alpha){

    list<Process_> queue;
    Process_ * current = NULL;

    int t = 0; // keep track of time
    unsigned int completed = 0; // # of processes_ completed
    
    int out_time = -1; // the time we switchout
    int in_time = -1; // the time we switch in
    string s; 

    // variables to keep track
    vector<float> result; // the final average output
    int total_burst_time = 0;
    int total_wait = 0;
    int total_switch = 0;
    int total_preempts = 0;

    int is_it_switching_out = false; // switch_duration/2
    int is_it_switching_in = false; // switch_duration/2


    cout << "time 0ms: Simulator started for SRT [Q <empty>]\n";

    while(true){ 

        // if the process finishes switching out: 3 different possiblities
        if(current != NULL && is_it_switching_out && (t == out_time + switch_duration/2)){

            if(current->get_current_preemption() == true){
                for (list<Process_>::iterator i = queue.begin(); i != queue.end(); i++){

                    // the below if statement measures the preemption condition
                    if((current->get_tau() - current->get_time_elapsed() < i->get_tau() - i->get_time_elapsed()) || ((current->get_tau() - current->get_time_elapsed() == i->get_tau() - i->get_time_elapsed()) && current->get_id() < i->get_id())){
                        current->setState("ready");
                        queue.insert(i,*current);
                        break;
                    }
                }
                if(current->get_state() == "switching"){
                    current->setState("ready");
                    queue.push_back(*current);
                }
            }
            else if(current->get_state() == "switching"){
                current->setState("blocked");
                current->setStartTime(t);
                current->setTurnaround(-1);
            }
            else if(current->get_state() == "done"){
                current->setTurnaround(-1);
            }
            current = NULL;
            is_it_switching_out = false;
            out_time = -1;
        }




        // if the process is not switching or not done: running in the cpu
        if (current != NULL && current->get_state() != "switching" && current->get_state() != "done"){
            int x = current->get_cpu_complete(current->get_completed());
            if((current->get_time_elapsed() == x) && current->get_state() == "running"){

                is_it_switching_out = true;
                out_time = t;
                current->setRemain(0);
                current->setElapsed(0);
                int d = current->get_cpu_complete(current->get_completed());
                total_burst_time += d;
                current->setBurstCompleted((current->get_burst_completed())+1);
                current->setCompleted((current->get_completed())+1);
                current->setCurrentPreemption(false);

                total_wait += current->get_waittime();
                current->setWaitTime(0);

                if(current->get_burst_completed() == current->get_num()){

                    s = print(queue);
                    cout << "time " << t << "ms: Process " << string(1,current->get_id()) << " terminated " << s << endl;
                    current->setState("done");
                    completed += 1;
                    if (completed == processes_.size()){
                        t += (int)switch_duration/2;
                        break;
                    }
                }
                else{
                    if(t<1000){
                        if (current->get_num() - current->get_burst_completed() == 1){

                            s = print(queue);
                            cout << "time " << t << "ms: Process " << string(1,current->get_id()) << " completed a CPU burst; 1 burst to go " << s << endl;
                        }
                        else{
                            s = print(queue);
                            cout << "time " << t << "ms: Process " << string(1,current->get_id()) << " completed a CPU burst; " << current->get_num() - current->get_burst_completed() <<" bursts to go " << s << endl;
                        }
                    }
                    current->setState("switching");

                    // starts I/O
                    int num = current->get_cpu_complete((current->get_completed())-1);
                    float old = current->get_tau();
                    float new_tau = (alpha * (float)num) + ((1-alpha) * old);

                    if (fmod(new_tau,1) >= 0.5){
                        new_tau = ceil(new_tau);
                    }
                    else{
                        new_tau = round(new_tau);
                   }
                    current->setTau(new_tau);
                    if(t<1000){
                    s = print(queue);
                    cout << "time " << t << "ms: Recalculated tau = " << current->get_tau() << "ms for process " << string(1,current->get_id()) << " " << s << endl;
                    int m = current->get_cpu_complete(current->get_completed());
                    cout << "time " << t << "ms: Process " << string(1,current->get_id()) << " switching out of CPU; will block on I/O until time " << t+m << "ms " << s << endl;
                    }
                }
            }
        }
       
        // if finishes switching in, starting of cpu
        if(is_it_switching_in && (t == in_time + switch_duration/2)){
            // end context switch in cpu start; 
            current->setState("running");

            if(t<1000){
                if(current->get_time_elapsed() > 0){
               //     cout << "222.333\n";
                    s = print(queue);
                    int y = current->get_cpu_complete(current->get_completed());
                    cout << "time " << t << "ms: Process " << string(1,current->get_id()) << " started using the CPU with " << (y - current->get_time_elapsed()) << "ms remaining " << s << endl;
                }
                else{
                    s = print(queue);
                    int y = current->get_cpu_complete(current->get_completed());
                    cout << "time " << t << "ms: Process " << string(1,current->get_id()) << " started using the CPU for " << y << "ms bursts " << s << endl;
                }
            }

            current->setStartTime(t);
            is_it_switching_in = false;
            in_time = -1;
            if(queue.size() > 0){
                if((((queue.begin())->get_tau()) - ((queue.begin())->get_time_elapsed()) < current->get_tau() - current->get_time_elapsed()) || ((((queue.begin())->get_tau()) - ((queue.begin())->get_time_elapsed()) == current->get_tau() - current->get_time_elapsed()) && (queue.begin())->get_id()<current->get_id())){
                    if(t<1000){
                    s = print(queue);
                    cout << "time " << t << "ms: Process " << string(1,(queue.begin())->get_id()) << " will preempt " << current->get_id() << " " << s << endl;
                    }
                    current->setState("switching");
                    is_it_switching_out = true;
                    out_time = t;
                    current->setCurrentPreemption(true);
                    total_preempts +=1;
                }
            }
            total_switch += 1;
        }


        // 12 extra processes entered the if statement with "running". what's wrong?
        // check for I/O completion
        unsigned int i = 0;
        for (i = 0; i< processes_.size(); i++){
           int z = (processes_[i]).get_cpu_complete(processes_[i].get_completed());
            if((processes_[i].get_state() == "blocked") && (t == (processes_[i]).get_starttime() + z - (int)(switch_duration/2))){
                processes_[i].setCompleted((processes_[i].get_completed())+1);

                // if process is running and 
                if((current != NULL) && (current->get_state() == "running") && (processes_[i].get_tau() - processes_[i].get_time_elapsed() < current->get_tau() - current->get_time_elapsed())){
                    processes_[i].setState("ready");
                    queue.insert(queue.begin(),processes_[i]);
                    if(t<1000){
                        s = print(queue);
                        cout << "time " << t << "ms: Process " << string(1,processes_[i].get_id()) << " completed I/O and will preempt " << current->get_id() << " " << s << endl;
                    }
                    current->setCurrentPreemption(true);
                    current->setState("switching");
                    is_it_switching_out = true;
                    out_time = t;
                    total_preempts += 1;
                }
                // i/O FINISH
                else { 
                    for (list<Process_>::iterator k = queue.begin(); k != queue.end(); k++){
                        if((processes_[i].get_tau() - processes_[i].get_time_elapsed() < (k->get_tau()) - (k->get_time_elapsed())) || ((processes_[i].get_tau() - processes_[i].get_time_elapsed() == k->get_tau() - k->get_time_elapsed()) && processes_[i].get_id() < k->get_id())){
                            processes_[i].setState("ready");
                            queue.insert(k, processes_[i]);
                            break;
                        }   
                    }
                    if(processes_[i].get_state() == "blocked"){
                        processes_[i].setState("ready");
                        queue.push_back(processes_[i]);
                    }
                    if(t<1000){
                        s = print(queue);
                        cout << "time " << t << "ms: Process "<< string(1, processes_[i].get_id()) << " (tau " << processes_[i].get_tau() << "ms) completed I/O; added to ready queue " << s << endl;
                    }
               }
            }
        }

        unsigned int j = 0;
        // some processes pass the t == get_arrival when 
         for (j = 0; j< processes_.size(); j++){
          //  cout << "process " << string(1,processes_[j].get_id()) << " is on " << processes_[j].get_state() << " at " << t << "ms "<<endl;
            if(t == processes_[j].get_arrival() && processes_[j].get_state() == "not arrived"){

                // i/O FINISH

                // if a process has arrived and has less remaining time than the current running process
                if(current != NULL && current->get_state() == "running" && ((processes_[j].get_tau() - processes_[j].get_time_elapsed() < current->get_tau() - current->get_time_elapsed()) || ((processes_[j].get_tau() - processes_[j].get_time_elapsed() == current->get_tau() - current->get_time_elapsed()) && processes_[j].get_id() < current->get_id()))){
                    processes_[j].setState("ready");
                    queue.insert(queue.begin(),processes_[j]);
                    if(t<1000){
                        s = print(queue);
                        cout << "time " << t << "ms: Process " << string(1,processes_[j].get_id()) << " arrived and will preempt " << current->get_id() << " " << s << endl;
                    }
                    current->setState("switching");
                    current->setCurrentPreemption(true);
                    is_it_switching_out = true;
                    out_time = t;
                    total_preempts += 1;
                }
                else{
                    // if process has arrived and has less remaining time than any process in the queue, put this arrived process before this process in the queue
                    for (list<Process_>::iterator l = queue.begin(); l != queue.end(); l++){
                        if((processes_[j].get_tau() - processes_[j].get_time_elapsed() < l->get_tau() - l->get_time_elapsed()) || ((processes_[j].get_tau() - processes_[j].get_time_elapsed() == l->get_tau() - l->get_time_elapsed()) && processes_[j].get_id() < l->get_id())){
                            processes_[j].setState("ready");
                            queue.insert(l, processes_[j]);
                            break;
                        }
                    }
                    if(processes_[j].get_state() == "not arrived"){
                        processes_[j].setState("ready");
                        queue.push_back(processes_[j]);
                    }
                    if(t<1000){
                        s = print(queue);
                        cout << "time " << t << "ms: Process "<< string(1, processes_[j].get_id()) << " (tau " << processes_[j].get_tau() << "ms) arrived; added to ready queue " << s << endl;
                    }
                }
            }
        }

        // if ready queue has something and we need the next processe
        if(current == NULL && queue.size() > 0){

            // start a context switch in...
            int which = which_process(processes_, queue.begin());
            current = &(processes_[which]); // reference
            queue.pop_front();
            current->setState("switching");
            is_it_switching_in = true;
            in_time = t;
        }

        // increment cpu elapsed time
        if(current != NULL && current->get_state() == "running"){
            current->setElapsed((current->get_time_elapsed()) + 1);
        }


        // increment waittime
        for (unsigned int h = 0; h< processes_.size(); h++){
            if(processes_[h].get_state() == "ready"){
                processes_[h].setWaitTime((processes_[h].get_waittime())+1);
            }
        }

        t += 1;
    }
    cout << "time " << t << "ms: Simulator ended for SRT [Q <empty>]\n";

    float total_b = total_burst(processes_);
    float averageCPUBurst = roundf((total_burst_time/total_b)*1000)/1000;
    float averageWait = roundf((total_wait/total_b)*1000)/1000;
    float averageturn = roundf(((total_wait+total_burst_time+total_switch*switch_duration)/total_b)*1000)/1000;
    result.push_back(averageCPUBurst);
    result.push_back(averageWait);
    result.push_back(averageturn);
    result.push_back((float)total_switch);
    result.push_back((float)total_preempts);
    return result;
}

void reset(vector<Process_>& processes_, float lambda){
    for (int i = 0; i< processes_.size(); i++){
        processes_[i].reset(lambda);
     }
}


void printprocess(const vector<Process_>& processes_){
     for (int i = 0; i< processes_.size(); i++){
        if (processes_[i].get_num() == 1){
            cout << "Process "<< processes_[i].get_id() << " [NEW] (arrival time "<<processes_[i].get_arrival()<<" ms) " << processes_[i].get_num() << 
             "CPU burst\n";
        }
        else{
            cout << "Process "<< processes_[i].get_id() << " [NEW] (arrival time "<<processes_[i].get_arrival()<<" ms) " << processes_[i].get_num() << 
             "CPU bursts\n";
        }
     }
}





int main(int argc, char* argv[]){

    
    int seed = atoi(argv[1]);
    srand48(seed);
    float lambda = atof(argv[2]);
    int MaxRand = atoi(argv[3]);
    int n = atoi(argv[4]); // number of processes_
    int t_switch = atoi(argv[5]); // duration of a switch
    float alpha = atof(argv[6]);
   // int t_slice = atoi(argv[7]); // for RR
    string RR; 
    if (argc == 9){
        RR = argv[8];
    }
    else if(argc == 8){
        RR = "END";
    }
    else{
	  cerr<<"Invalid command arguments"<<endl;
      return -1;
    }
    int init_tau = ceil(1.0/lambda);
    vector<Process_> processes_;
    //Vector<int> totalburst;

    
    for(int i = 0; i < n; i ++ ){
        double r= 999;
        while(1){
            double ram = drand48();
            r = -log(ram) / (double)lambda;
            if (r < MaxRand){
                break;
            }
        }
        int arrive_time = floor(r);
        
        // # of bursts for this process
        double ram2 = drand48();
        int burst_num = floor(ram2*100)+1;
       
        // the actual cpu burst times of all bursts in this process
        vector<int> cpu_b;
        vector<int> io_b;

        bool io = false;
        
        for(int i = 0; i < (burst_num-1)*2+1; i++){

            if (io){
                 while(1){
                    double ram = drand48();
                    r = -log(ram) / (double)lambda;
                    if (r < MaxRand){
                        break;
                     }
                 }

                int half = t_switch/2;
                int cpu_time = ceil(r) + half;
                cpu_b.push_back(cpu_time);
                io_b.push_back(cpu_time);
                io = false;
            }
            else{
                 while(1){
                    double ram = drand48();
                    r = -log(ram) / (double)lambda;
                    if (r < MaxRand){
                        break;
                     }
                 }

                int cpu_time = ceil(r);
                cpu_b.push_back(cpu_time);
                io = true;
            }
        }
        Process_ one_process(i, arrive_time,burst_num, io_b, cpu_b, init_tau);
        processes_.push_back(one_process);        
    }
    ofstream myfile;
    myfile.open ("simout.txt");

    printprocess(processes_);
    vector<float> answers = sjf(processes_,t_switch, alpha);
    write(myfile, answers, "SJF");
    reset(processes_,lambda);

    cout << endl << endl << endl << endl;


    printprocess(processes_);
    vector<float> answers2 = srt(processes_,t_switch, alpha);
    write(myfile, answers2, "SRT");    
    reset(processes_, lambda);
    myfile.close();
    return 0;
}

