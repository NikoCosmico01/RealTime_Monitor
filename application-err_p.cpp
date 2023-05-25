/*Claudio Praticò 340404, Giuseppe Gabriele Tarollo 343707*/

#include "executive.h"
#include "busy_wait.h"

#define BUSY_WAIT_GENERATION
#define N_MS 8 //In funzione della CPU utilizzata potrebbero verificarsi Deadline Miss, è possibile abbassare la busy waiting qui

Executive exec(5, 4);

std::random_device rd;							// inizializzazione
std::mt19937 gen(rd());							// generatore
std::uniform_int_distribution<> dis(0, 4);		// random
unsigned int rand_gen = (int) dis(gen);	

unsigned int count = 0;

void task0()
{
	std::cout << "- Task 0 start execution" << std::endl;

	#ifdef BUSY_WAIT_GENERATION
		busy_wait(2*N_MS); //Raddoppio la durata del task 0 per forzare delle Deadline Miss, volendo si può aumentare per peggiorare la situazione
	#endif

	std::cout << "- Task 0 stop execution" << std::endl;

}

void task1()
{
	std::cout << "- Task 1 start execution" << std::endl;

	#ifdef BUSY_WAIT_GENERATION
		busy_wait(2*N_MS);
	#endif

	std::cout << "- Task 1 stop execution" << std::endl;
}

void task2()
{
	std::cout << "- Task 2 start execution" << std::endl;

	#ifdef BUSY_WAIT_GENERATION
		busy_wait(1*N_MS);
	#endif

	std::cout << "- Task 2 stop execution" << std::endl;
}

void task3()
{
	std::cout << "- Task 3 start execution" << std::endl;

	#ifdef BUSY_WAIT_GENERATION
		busy_wait(1*N_MS);
	#endif

	if(++count == rand_gen) {						// ap_ task lanciato in modo sporadico;
		std::cout << "	Launching aperiodic request..." << std::endl;
		exec.ap_task_request();
		rand_gen = dis(gen);						// random cambia ad ogni esecuzione di ap_task;
		count = 0;
	}

	std::cout << "- Task 3 stop execution" << std::endl;
}

void task4()
{
	std::cout << "- Task 4 start execution" << std::endl;

	#ifdef BUSY_WAIT_GENERATION
		busy_wait(1*N_MS);
	#endif

	std::cout << "- Task 4 stop execution" << std::endl;
}

/* Nota: nel codice di uno o piu' task periodici e' lecito chiamare Executive::ap_task_request() */

void ap_task()
{
	std::cout << "- Task Aperiodico start execution" << std::endl;

	#ifdef BUSY_WAIT_GENERATION
		busy_wait(2*N_MS);
	#endif

	std::cout << "- Task Aperiodico stop execution" << std::endl;
}

int main()
{
	if (getuid()){
			std::cout<<"Need to start with sudo!"<<std::endl;
			exit(-1);
	}

	busy_wait_init();
	// wcet= tempo massimo di esecuzione previsto
	exec.set_periodic_task(0, task0, 1); // tau_1
	exec.set_periodic_task(1, task1, 2); // tau_2
	exec.set_periodic_task(2, task2, 1); // tau_3,1
	exec.set_periodic_task(3, task3, 3); // tau_3,2
	exec.set_periodic_task(4, task4, 1); // tau_3,3
	
	/* ... */

	exec.set_aperiodic_task(ap_task, 2);

	exec.add_frame({0,1,2});
	exec.add_frame({0,3});
	exec.add_frame({0,1});
	exec.add_frame({0,1});
	exec.add_frame({0,1,4});
	/* ... */

	exec.run();

	return 0;
}
