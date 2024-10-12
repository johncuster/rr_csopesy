#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>   
#include <iomanip>   
#include <ctime>  
#include <thread>
#include <string>
#include <queue>
#include <cstdlib>

using namespace std;

struct Process {
    int pid;
    int instructions_left;
    int initial_instructions;
    int finish = 0;
    int processing = 0;
    string starttime;
    string finishtime;
    int core_id = -1;
};

string print_datetime() {
    auto now = chrono::system_clock::now();
    time_t current_time = chrono::system_clock::to_time_t(now);

    tm local_time;
    localtime_s(&local_time, &current_time);

    char date_buffer[100];
    strftime(date_buffer, sizeof(date_buffer), "%m/%d/%Y %I:%M:%S", &local_time);

    string am_pm = (local_time.tm_hour >= 12) ? "PM" : "AM";

    string formatted_time = string(date_buffer) + " " + am_pm;

    return formatted_time;
}

string int_to_string(int number) {
    char buffer[20];
    sprintf_s(buffer, "%d", number);
    return string(buffer);
}

vector<Process> processes;
queue<int> ready_queue;
int quantum = 100; 
int numCPUs = 4;  
int minInstructions = 1000; 
int maxInstructions = 2000;
int core_progress[10] = { 0 };  
int core_active_time[4] = { 0 }; 
chrono::time_point<chrono::high_resolution_clock> start_time; 


void round_robin_core(int core_id) {
    while (!ready_queue.empty()) {
        int process_index = ready_queue.front();
        ready_queue.pop();

        Process& process = processes[process_index];
        process.starttime = print_datetime();
        process.core_id = core_id;

        string filename = "process" + int_to_string(process.pid) + ".txt";

        int execution_time = min(quantum, process.instructions_left);
        this_thread::sleep_for(chrono::milliseconds(execution_time));  

        process.instructions_left -= execution_time;
        core_progress[process.pid] += execution_time;
        core_active_time[core_id] += execution_time;

        int instructions_finished = process.initial_instructions - process.instructions_left;

        ofstream log_file(filename, ios::app);
        if (log_file.is_open()) {
            log_file << print_datetime()
                << ", Core: " << (core_id + 1)
                << ", Executing process " << process.pid
                << " for " << execution_time << "ms"
                << ", Progress: " << instructions_finished << " / "
                << process.initial_instructions << "\n";
            log_file.close();
        }

        if (process.instructions_left > 0) {
            ready_queue.push(process_index); 
        }
        else {
            process.finish = 1;
            process.finishtime = print_datetime();
            ofstream log_file(filename, ios::app);
            if (log_file.is_open()) {
                log_file << "Process " << process.pid << " finished at " << process.finishtime << endl;
                log_file.close();
            }
        }
    }
}


int main() {
    processes = {
        {1, minInstructions + rand() % (maxInstructions - minInstructions + 1)},
        {2, minInstructions + rand() % (maxInstructions - minInstructions + 1)},
        {3, minInstructions + rand() % (maxInstructions - minInstructions + 1)},
        {4, minInstructions + rand() % (maxInstructions - minInstructions + 1)},
        {5, minInstructions + rand() % (maxInstructions - minInstructions + 1)},
        {6, minInstructions + rand() % (maxInstructions - minInstructions + 1)},
        {7, minInstructions + rand() % (maxInstructions - minInstructions + 1)},
        {8, minInstructions + rand() % (maxInstructions - minInstructions + 1)},
        {9, minInstructions + rand() % (maxInstructions - minInstructions + 1)},
        {10, minInstructions + rand() % (maxInstructions - minInstructions + 1)}
    };

    for (auto& process : processes) {
        process.initial_instructions = process.instructions_left; 
    }

    for (int i = 0; i < processes.size(); ++i) {
        ready_queue.push(i);
    }

    start_time = chrono::high_resolution_clock::now(); 

    cout << "Round Robin Scheduler\n";
    thread core_threads[4];
    bool threads_started = false;
    while (true) {
        cout << "\Command >> ";
        string cmd;
        cin >> cmd;

        if (cmd == "screen") {
            if (!threads_started) { 
                for (int i = 0; i < numCPUs; i++) {
                    core_threads[i] = thread(round_robin_core, i);
                    this_thread::sleep_for(chrono::milliseconds(100));
                }
                threads_started = true; 
            }
            cout << "\nRunning Processes:\n";
            for (const auto& process : processes) {
                if (process.finish == 0 || process.processing == 1) {
                    int instructions_finished = process.initial_instructions - process.instructions_left; 
                     cout << "Process " << process.pid 
                     << "\t(" << process.starttime 
                     << ") \tCore:" << process.core_id 
                     << "\tProgress: " << instructions_finished << " / " 
                     << process.initial_instructions << endl; 
                }
            }

            cout << "\nFinished Processes:\n";
            for (const auto& process : processes) {
                if (process.finish == 1) {
                    cout << "Process " << process.pid << "\t(" << process.finishtime
                        << ") \tFINISHED\n";
                }
            }

        }
        else {
            cout << "\nUnknown command. Type 'screen -ls' to check progress.\n";
        }

        bool all_finished = true;
        for (const auto& process : processes) {
            if (process.finish == 0) {
                all_finished = false;
                break;
            }
        }

        if (all_finished) {
            break;
        }

    }

    for (int i = 0; i < 4; i++) {
        if (core_threads[i].joinable()) {
            core_threads[i].join();
        }
    }
    return 0;
}