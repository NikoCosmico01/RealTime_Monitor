#include "executive.h"
#include <iomanip>
#include <sstream>
#include <iostream>
#include <random>

#include "busy_wait.h"

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> dis50(30, 50);
std::uniform_int_distribution<> dis100(60, 100);

void task0()
{
	busy_wait(dis100(gen) + 80);
}

void task1()
{
	busy_wait(dis50(gen) + 30);
}

void task2()
{
	busy_wait(dis50(gen) + 210);
}

void task3()
{
	busy_wait(dis100(gen) + 70);
}

void task4()
{
	busy_wait(dis50(gen) + 310);
}

void task5()
{
	busy_wait(dis50(gen) + 42);
}

void task_stat_print(const task_stats & stat)
{
	std::ostringstream os;
		
	os << "*** Task Stats #" << stat.cycle_id << " [" << stat.task_id << "]: E/M/C="
		<< stat.exec_count << "/" << stat.miss_count << "/"  << stat.canc_count 
		<< "  avg/max=" << stat.avg_exec_time << "/" << stat.max_exec_time << std::endl;
		
	std::cout << os.str();
}

void global_stat_print(Executive & exec)
{
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::seconds(6));
		
		global_stats stat = exec.get_global_stats();
		
		std::ostringstream os;
		
		os << "*** Global Stats #" << stat.cycle_count << ": E/M/C=" << stat.exec_count << "/" << stat.miss_count << "/"  << stat.canc_count << std::endl;
		
		std::cout << os.str();
	}
}

int main()
{
	Executive exec(6, 5, 100);

	exec.set_periodic_task(0, task0, 2);
	exec.set_periodic_task(1, task1, 1);
	exec.set_periodic_task(2, task2, 2);
	exec.set_periodic_task(3, task3, 2);
	exec.set_periodic_task(4, task4, 3);
	exec.set_periodic_task(5, task5, 1);
	
	exec.add_frame({0,1,2});
	exec.add_frame({3,4});
	exec.add_frame({0,3});
	exec.add_frame({1,4,5});
	exec.add_frame({0,2});
	exec.add_frame({1,5,2});
	
	exec.set_stats_observer(task_stat_print);

	exec.start();
	
	global_stat_print(exec);
	
	exec.wait();
	
	return 0;
}
