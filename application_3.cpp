#include "executive.h"
#include <iostream>
#include <random>

#include "busy_wait.h"

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> dis5(1, 5);
std::uniform_int_distribution<> dis10(1, 10);

void task0()
{
	busy_wait(dis5(gen) + 4);
}

void task1()
{
	busy_wait(dis10(gen) + 8);
}

void task2()
{
	busy_wait(dis5(gen) + 3);
}

void task3()
{
	busy_wait(2*dis10(gen) + 9);
}

void task4()
{
	busy_wait(dis5(gen) + 5);
}

void task_stat_print(const task_stats & stat)
{
	if (stat.cycle_id % 10 == 0)
	{
		std::cout << "*** Task Stats #" << stat.cycle_id << " [" << stat.task_id << "]: E/M/C="
			<< stat.exec_count << "/" << stat.miss_count << "/"  << stat.canc_count 
			<< "  avg/max=" << stat.avg_exec_time << "/" << stat.max_exec_time << std::endl;
	}
}

void global_stat_print(Executive & exec)
{
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::seconds(2));
		
		global_stats stat = exec.get_global_stats();
		
		std::cout << "*** Global Stats #" << stat.cycle_count << ": E/M/C=" << stat.exec_count << "/" << stat.miss_count << "/"  << stat.canc_count << std::endl;
	}
}

int main()
{
	Executive exec(5, 4);

	exec.set_periodic_task(0, task0, 1); // tau_1
	exec.set_periodic_task(1, task1, 2); // tau_2
	exec.set_periodic_task(2, task2, 1); // tau_3,1
	exec.set_periodic_task(3, task3, 3); // tau_3,2
	exec.set_periodic_task(4, task4, 1); // tau_3,3
	
	exec.add_frame({0,1,2});
	exec.add_frame({0,3});
	exec.add_frame({0,1});
	exec.add_frame({0,1});
	exec.add_frame({0,1,4});
	
	exec.set_stats_observer(task_stat_print);
	
	exec.start();
	
	global_stat_print(exec);
	
	exec.wait();
	
	return 0;
}
