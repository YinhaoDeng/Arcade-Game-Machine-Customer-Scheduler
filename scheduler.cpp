
#include <iostream>
#include <fstream>
#include <deque>
#include <vector>
#include <typeinfo>
using namespace std;

// std is a namespace: https://www.cplusplus.com/doc/oldtutorial/namespaces/
const int TIME_ALLOWANCE = 9;  // allow to use up to this number of time slots at once
const int PRINT_LOG = 0; // print detailed execution trace

class Customer
{
public:
    std::string name;
    int priority;
    int arrival_time;
    int slots_remaining; // how many time slots are still needed
    int playing_since;
    int has_excuted;

    Customer(std::string par_name, int par_priority, int par_arrival_time, int par_slots_remaining, int par_has_excuted)
    {
        name = par_name;
        priority = par_priority;
        arrival_time = par_arrival_time;
        slots_remaining = par_slots_remaining;
        playing_since = -1;
        has_excuted = par_has_excuted;
    }
};

class Event
{
public:
    int event_time;
    int customer_id;  // each event involes exactly one customer

    Event(int par_event_time, int par_customer_id)
    {
        event_time = par_event_time;
        customer_id = par_customer_id;
    }
};



void initialize_system(
    std::ifstream &in_file,
    std::deque<Event> &arrival_events,
    std::vector<Customer> &customers)
{
    std::string name;
    int priority, arrival_time, slots_requested;

    // read input file line by line
    // https://stackoverflow.com/questions/7868936/read-file-line-by-line-using-ifstream-in-c
    int customer_id = 0;
    while (in_file >> name >> priority >> arrival_time >> slots_requested)
    {
        Customer customer_from_file(name, priority, arrival_time, slots_requested, 0);
        customers.push_back(customer_from_file);

        // new customer arrival event
        Event arrival_event(arrival_time, customer_id);
        arrival_events.push_back(arrival_event);

        customer_id++;
    }
}

void print_state(
    std::ofstream &out_file,
    int current_time,
    int current_id,
    const std::deque<Event> &arrival_events,
    const std::deque<int> &customer_queue)
{
    out_file << current_time << " " << current_id << '\n';
    if (PRINT_LOG == 0)
    {
        return;
    }
    std::cout << current_time << ", " << current_id << '\n';
    for (int i = 0; i < arrival_events.size(); i++)
    {
        std::cout << "\t" << arrival_events[i].event_time << ", " << arrival_events[i].customer_id << ", ";
    }
    std::cout << '\n';
    for (int i = 0; i < customer_queue.size(); i++)
    {
        std::cout << "\t" << customer_queue[i] << ", ";
    }
    std::cout << '\n';
}


void self_defined_print_state(
    std::ofstream &out_file,  //指定输出文件
    int current_time,  //当前时间
    int current_id)  //当前process id名字
{
    out_file << current_time << " " << current_id << '\n';
}



// process command line arguments
// https://www.geeksforgeeks.org/command-line-arguments-in-c-cpp/
int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Provide input and output file names." << std::endl;
        return -1;
    }
    std::ifstream in_file(argv[1]);
    std::ofstream out_file(argv[2]);
    if ((!in_file) || (!out_file))
    {
        std::cerr << "Cannot open one of the files." << std::endl;
        return -1;
    }

    // deque: https://www.geeksforgeeks.org/deque-cpp-stl/
    // vector: https://www.geeksforgeeks.org/vector-in-cpp-stl/
    std::deque<Event> arrival_events; // new customer arrivals
    std::vector<Customer> customers; // information about each customer

    // read information from file, initialize events queue
    initialize_system(in_file, arrival_events, customers);

    int current_id = -1; // who is using the machine now, -1 means nobody
    int time_out = -1; // time when current customer will be preempted

    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    std::deque<int> high_q;  // I add this
    std::deque<int> low_q;   // I add this


    // step by step simulation of each time slot
    bool SUPER_FLAG = false;  //我加的概率等分flag！！！！！
    bool all_done = false;
    for (int current_time = 0; !all_done; current_time++)
    {
        // welcome newly arrived customers
        while (!arrival_events.empty() && (current_time == arrival_events[0].event_time))
        {
            if(customers[0].priority == 0)
                high_q.push_back(arrival_events[0].customer_id);
            else
                low_q.push_back(arrival_events[0].customer_id);

            arrival_events.pop_front();
        }
        // 正在运行阶段 check if we need to take a customer off the machine
        if (current_id >= 0)
        {
            if (current_time == time_out) //规定时间到了，还未结束
            {
                int last_run = current_time - customers[current_id].playing_since;
                customers[current_id].slots_remaining -= last_run;
                if (customers[current_id].slots_remaining > 0)
                {
                    // customer is not done yet, waiting for the next chance to play
                    // queue.push_back(current_id);
                    if(customers[current_id].priority == 0) //重回queue中
                        high_q.push_back(current_id);
                    else
                        low_q.push_back(current_id);
                }
                current_id = -1; // the machine is free now
            }
        }
        // 找人阶段 if machine is empty, schedule a new customer
        

        if (current_id == -1)
        {
            int a = low_q.size();
            int b = high_q.size();
            
            if (a>8 && b>10) {  //新的机制 self-defined mechanism   特殊情况
                bool has_not_excuted = false;  //是否存在  还未被运行过的顾客

                for (int i=0; i<low_q.size(); i++) // iterate low queue
                {
                    int id = low_q[i];
                    if (customers[id].has_excuted == 0) //如果low queue里找到还没执行的customer，就优先执行他
                    {
                        current_id = id;
                        customers[current_id].has_excuted = 1; // update the flag
                        low_q.erase(low_q.begin()+i); // erase
                        has_not_excuted = true; //我找到一个还未被运行的顾客
                        break;
                    }
                }

                if (has_not_excuted==false)
                {
                    for (int i=0; i<high_q.size(); i++) // iterate low queue
                    {
                        int id = high_q[i];
                        if (customers[id].has_excuted == 0) //给high q里从未被执行的人 一次运行的机会
                        {
                            current_id = id;
                            customers[current_id].has_excuted = 1; // update the flag
                            high_q.erase(high_q.begin()+i); // erase
                            has_not_excuted = true;
                            break;
                        }else if (i==high_q.size()-1){
                            if (SUPER_FLAG == false)
                            {
                                current_id = high_q.front();
                                customers[current_id].has_excuted = 1; // update has_excuted flag
                                high_q.pop_front();
                                SUPER_FLAG = true;
                            }else if (SUPER_FLAG == true)
                            {
                                current_id = low_q.front();
                                customers[current_id].has_excuted = 1; // update has_excuted flag
                                low_q.pop_front();
                                SUPER_FLAG = false;
                            }
                            
                        }
                    }
                }

               

                if (TIME_ALLOWANCE > customers[current_id].slots_remaining)  // 提前结束
                {
                    time_out = current_time + customers[current_id].slots_remaining;
                }
                else
                {
                    time_out = current_time + TIME_ALLOWANCE;  //用满给定时间
                }
                customers[current_id].playing_since = current_time;
            }
            else if (!high_q.empty()) // is anyone waiting?
            {
                current_id = high_q.front();
                customers[current_id].has_excuted = 1; // update has_excuted flag
                high_q.pop_front();

                if (TIME_ALLOWANCE > customers[current_id].slots_remaining)  // 提前结束
                {
                    time_out = current_time + customers[current_id].slots_remaining;
                }
                else
                {
                    time_out = current_time + TIME_ALLOWANCE;  //用满给定时间
                }
                customers[current_id].playing_since = current_time;
            }
            else if(!low_q.empty()){ //low queue is considered only when high queue is empty
                current_id = low_q.front();
                customers[current_id].has_excuted = 1; // update has_excuted flag
                low_q.pop_front();

                if (TIME_ALLOWANCE > customers[current_id].slots_remaining)  // 提前结束
                {
                    time_out = current_time + customers[current_id].slots_remaining;
                }
                else
                {
                    time_out = current_time + TIME_ALLOWANCE;  //用满给定时间
                }
                customers[current_id].playing_since = current_time;
            }
        }

        self_defined_print_state(out_file, current_time, current_id);
        // print_state(out_file, current_time, current_id, arrival_events, queue);

        // exit loop when there are no new arrivals, no waiting and no playing customers  更新flag
        // all_done = (arrival_events.empty() && queue.empty() && (current_id == -1));
        all_done = (arrival_events.empty() && high_q.empty() && low_q.empty() && (current_id == -1));
    }

    return 0;
}
