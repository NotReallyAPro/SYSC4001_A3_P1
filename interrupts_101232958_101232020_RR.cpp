/**
 * @file interrupts.cpp
 * @author Sasisekhar Govind
 * @brief template main.cpp file for Assignment 3 Part 1 of SYSC4001
 * 
 */

#include<interrupts_101232958_101232020.hpp>
#define TIME_SLICE 100

void FCFS(std::vector<PCB> &ready_queue) {
    std::sort( 
                ready_queue.begin(),
                ready_queue.end(),
                []( const PCB &first, const PCB &second ){
                    return (first.arrival_time > second.arrival_time); 
                } 
            );
}

std::tuple<std::string /* add std::string for bonus mark */ > run_simulation(std::vector<PCB> list_processes) {

    std::vector<PCB> ready_queue;   //The ready queue of processes
    std::vector<PCB> wait_queue;    //The wait queue of processes
    std::vector<PCB> job_list;      //A list to keep track of all the processes. This is similar
                                    //to the "Process, Arrival time, Burst time" table that you
                                    //see in questions. You don't need to use it, I put it here
                                    //to make the code easier :).
    std::vector<PCB> na_list;       //List of unassigned processes;
    unsigned int current_time = 0;
    unsigned int counter = 0;
    PCB running;

    //Initialize an empty running process
    idle_CPU(running);

    std::string execution_status;
    std::string system_status;

    //make the output table (the header row)
    execution_status = print_exec_header();

    //Loop while till there are no ready or waiting processes.
    //This is the main reason I have job_list, you don't have to use it.
    while(!all_process_terminated(job_list) || job_list.empty()) {

        //Inside this loop, there are three things you must do:
        // 1) Populate the ready queue with processes as they arrive
        // 2) Manage the wait queue
        // 3) Schedule processes from the ready queue

        //Population of ready queue is given to you as an example.
        //Go through the list of proceeses
        for(auto &process : list_processes) {
            if(process.arrival_time == current_time) {//check if the AT = current time
                //if so, attempt to assign memory and put the process into the ready queue
                if(assign_memory(process)) {
                    process.state = READY;  //Set the process state to READY
                    ready_queue.insert(ready_queue.begin(), process); //Add the process to the back(front of vector) of the ready queue
                    execution_status += print_exec_status(current_time, process.PID, NEW, READY);
                }
                else {
                    na_list.push_back(process);
                }
                job_list.push_back(process); //Add it to the list of processes
                system_status += "time: " + std::to_string(current_time) + "; Process " + std::to_string(process.PID) + ": Arrived\n";
                system_status += print_PCB(job_list);
            }
        }

        //Try to assign memory to unassigned processes if there are any.
        if(!na_list.empty()) {
            for(size_t i = 0; i < na_list.size(); i++) {
                if(assign_memory(na_list[i])) {
                    na_list[i].state = READY;  //Set the process state to READY
                    execution_status += print_exec_status(current_time, na_list[i].PID, NEW, READY);
                    ready_queue.insert(ready_queue.begin(), na_list[i]); //Add the process to the back(front of vector) of the ready queue
                    sync_queue(job_list, na_list[i]);
                    na_list.erase(na_list.begin() + i);
                    i--;       //Accounts for the shifting of indices of elements.
                }
            }
        }

        ///////////////////////MANAGE WAIT QUEUE/////////////////////////
        //This mainly involves keeping track of how long a process must remain in the ready queue
        for(size_t i = 0; i < wait_queue.size(); i++) {
            if(wait_queue[i].processing_time == 0) {
                wait_queue[i].state = READY;
                execution_status += print_exec_status(current_time, wait_queue[i].PID, WAITING, READY);
                sync_queue(job_list, wait_queue[i]);
                ready_queue.insert(ready_queue.begin(), wait_queue[i]); //Add the process to the back(front of vector) of the ready queue
                wait_queue.erase(wait_queue.begin() + i);
                i--;       //Accounts for the shifting of indices of elements.
            }
            else {
                wait_queue[i].processing_time--;
            }
        }
        /////////////////////////////////////////////////////////////////

        //////////////////////////SCHEDULER//////////////////////////////
        //Checks if the process is finished, requires I/O, used up its time quantum, or needs more time to process.
        if(running.PID != -1) {
            if(running.remaining_time == 0) {
                execution_status += print_exec_status(current_time, running.PID, RUNNING, TERMINATED);
                system_status += "time: " + std::to_string(current_time) + "; Process " + std::to_string(running.PID) + ": Terminated\n";
                terminate_process(running, job_list);
                system_status += print_PCB(job_list);
                idle_CPU(running);
            }
            else if(running.processing_time - running.remaining_time == running.io_freq) {
                execution_status += print_exec_status(current_time, running.PID, RUNNING, WAITING);
                wait_process(running, job_list, wait_queue);
                idle_CPU(running);
            }
            else if(counter == 0) {
                running.state = READY;
                execution_status += print_exec_status(current_time, running.PID, RUNNING, READY);
                sync_queue(job_list, running);
                ready_queue.insert(ready_queue.begin(), running); //Add the process to the back(front of vector) of the ready queue
                idle_CPU(running);
            }
            else {
                running.remaining_time--;
                counter--;
            }
        }

        //If the CPU is idle, pick and run a process from ready queue if possible.
        if(running.PID == -1) {
            if(!ready_queue.empty()) {
                run_process(running, job_list, ready_queue, current_time);
                execution_status += print_exec_status(current_time, running.PID, READY, RUNNING);
                counter = TIME_SLICE;
                if(running.processing_time == 0) {
                    running.processing_time = running.remaining_time;
                }
                running.remaining_time--;
                counter--;
            }
        }
        /////////////////////////////////////////////////////////////////

        current_time++;
    }
    
    //Close the output table
    execution_status += print_exec_footer() + system_status;

    return std::make_tuple(execution_status);
}


int main(int argc, char** argv) {

    //Get the input file from the user
    if(argc != 2) {
        std::cout << "ERROR!\nExpected 1 argument, received " << argc - 1 << std::endl;
        std::cout << "To run the program, do: ./interrutps <your_input_file.txt>" << std::endl;
        return -1;
    }

    //Open the input file
    auto file_name = argv[1];
    std::ifstream input_file;
    input_file.open(file_name);

    //Ensure that the file actually opens
    if (!input_file.is_open()) {
        std::cerr << "Error: Unable to open file: " << file_name << std::endl;
        return -1;
    }

    //Parse the entire input file and populate a vector of PCBs.
    //To do so, the add_process() helper function is used (see include file).
    std::string line;
    std::vector<PCB> list_process;
    while(std::getline(input_file, line)) {
        auto input_tokens = split_delim(line, ", ");
        auto new_process = add_process(input_tokens);
        list_process.push_back(new_process);
    }
    input_file.close();

    //With the list of processes, run the simulation
    auto [exec] = run_simulation(list_process);

    write_output(exec, "execution_RR.txt");

    return 0;
}