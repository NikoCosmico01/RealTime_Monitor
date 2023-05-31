#ifndef EXECUTIVE_H
#define EXECUTIVE_H

#include <iostream>
#include <vector>
#include <functional>
#include <deque>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "rt/priority.h"
#include "rt/affinity.h"
#include "statistics.h"

class Executive
{
public:
	/* Inizializza l'executive, impostando i parametri di scheduling:
		num_tasks: numero totale di task presenti nello schedule;
		frame_length: lunghezza del frame (in quanti temporali);
		unit_duration: durata dell'unita di tempo, in millisecondi (default 10ms).
	*/
	Executive(size_t num_tasks, unsigned int frame_length, unsigned int unit_duration = 10);

	/* Imposta il task periodico di indice "task_id" (da invocare durante la creazione dello schedule):
		task_id: indice progressivo del task, nel range [0, num_tasks);
		periodic_task: funzione da eseguire al rilascio del task;
		wcet: tempo di esecuzione di caso peggiore (in quanti temporali).
	*/
	void set_periodic_task(size_t task_id, std::function<void()> periodic_task, unsigned int wcet);

	/* Lista di task da eseguire in un dato frame (da invocare durante la creazione dello schedule):
		frame: lista degli id corrispondenti ai task da eseguire nel frame, in sequenza
	*/
	void add_frame(std::vector<size_t> frame);

	/* [STAT] Imposta la funzione da eseguire quanto è pronta una nuova statistica per un task */
	void set_stats_observer(std::function<void(const task_stats &)> obs);

	/* [STAT] Ritorna la statistica di funzionamento globale dell'applicazione */
	global_stats get_global_stats();

	/* [RUN] Lancia l'applicazione */

	void start();

	/* [RUN] Attende (all'infinito) finchè gira l'applicazione */
	void wait();

private:
	struct task_data
	{
		std::function<void()> function;
		unsigned int wcet;
		rt::priority priority;

		std::thread thread;
		std::condition_variable cond;
		int status;
		int index;
		bool was_missed;
		std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
		task_stats stats;
	};

	std::vector<task_data> p_tasks;
		
	std::thread exec_thread;
		
	std::vector< std::vector<size_t> > frames;
		
	const unsigned int frame_length;
	const std::chrono::milliseconds unit_time;
	std::mutex mutex;

	static void task_function(task_data &task, std::mutex &mutex);

	void exec_function();

	// Statistiche ......

	std::condition_variable cond_buffer;
	std::deque<std::vector<task_stats>> buffer;

	std::function<void(const task_stats &)> stats_observer;
	std::thread stats_thread;
	void stats_function();
};

#endif