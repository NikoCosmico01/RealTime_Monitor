#include "executive.h"
#include <iostream>

#include "busy_wait.h"

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
	exec.add_frame({0,1});
	
	exec.run();
	
	return 0;
}
