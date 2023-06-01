#include "executive.h"
#include <iostream>
#include <random>

#include "busy_wait.h"

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> dis5(1, 5);
std::uniform_int_distribution<> dis10(1, 10);

void task0() { //1
	std::cout << "Sono il task n.0" << std::endl;
	busy_wait(90);
}

void task1() //2
{
	std::cout << "Sono il task n.1" << std::endl;
	busy_wait(320); //180
}
void task2() //31
{
	std::cout << "Sono il task n.2" << std::endl;
	busy_wait(1600);
}

void task3() //32
{
	std::cout << "Sono il task n.3" << std::endl;
	busy_wait(310);
}

void task4() //33
{
	std::cout << "Sono il task n.4" << std::endl;
	busy_wait(80);
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
		std::this_thread::sleep_for(std::chrono::seconds(3));
		
		global_stats stat = exec.get_global_stats();
		
		std::cout << "*** Global Stats #" << stat.cycle_count << ": E/M/C=" << stat.exec_count << "/" << stat.miss_count << "/"  << stat.canc_count << std::endl;
	}
}

int main()
{
	Executive exec(5, 4, 100);

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

	//global_stat_print(exec);
	
	exec.wait();
	
	return 0;
}
