#include "executive.h"
#include <iostream>

#include "busy_wait.h"

void task0()
{
	std::cout << "Sono il task n.0" << std::endl;
	busy_wait(90);
}

void task1()
{
	std::cout << "Sono il task n.1" << std::endl;
	busy_wait(200);
}
void task2()
{
	std::cout << "Sono il task n.2" << std::endl;
	busy_wait(88);
}

void task3()
{
	std::cout << "Sono il task n.3" << std::endl;
	busy_wait(270);
}

void task4()
{
	std::cout << "Sono il task n.4" << std::endl;
	busy_wait(80);
}

int main()
{
	//Parametri exec: task in totale, quanto grande il frame (400 millisecondi: parametro due * parametro tre), dimensione in millisecondi del quanto temporale
	Executive exec(5, 4, 100);

	exec.set_periodic_task(0, task0, 1); // tau_1
	exec.set_periodic_task(1, task1, 2); // tau_2
	exec.set_periodic_task(2, task2, 1); // tau_3,1
	exec.set_periodic_task(3, task3, 3); // tau_3,2
	exec.set_periodic_task(4, task4, 1); // tau_3,3
	
	//serve per aggiungere il frame
	exec.add_frame({0,1,2});
	exec.add_frame({0,3});
	exec.add_frame({0,1});
	exec.add_frame({0,1});
	exec.add_frame({0,1,4});
	
	//pone in esecuzione tutto
	exec.run();
	

	
	return 0;
}
