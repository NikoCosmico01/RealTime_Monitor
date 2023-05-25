/*Claudio Praticò 340404, Giuseppe Gabriele Tarollo 343707*/

#ifndef EXECUTIVE_H
#define EXECUTIVE_H
#define APERIODIC -1

#include <vector>
#include <functional>
#include <chrono>
#include <thread>
#include <iostream>
#include <sstream>
#include <random>
#include <mutex>
#include <unistd.h>
#include <sys/types.h>
#include <condition_variable>
#include "rt/priority.h"
#include "rt/affinity.h"



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

		/* Imposta il task aperiodico (da invocare durante la creazione dello schedule):
			aperiodic_task: funzione da eseguire al rilascio del task;
			wcet: tempo di esecuzione di caso peggiore (in quanti temporali).
		*/
		void set_aperiodic_task(std::function<void()> aperiodic_task, unsigned int wcet);

		/* Lista di task da eseguire in un dato frame (da invocare durante la creazione dello schedule):
			frame: lista degli id corrispondenti ai task da eseguire nel frame, in sequenza
		*/
		void add_frame(std::vector<size_t> frame);

		/* Esegue l'applicazione */
		void run();

		/* Richiede il rilascio del task aperiodico (da invocare durante l'esecuzione).
		*/
		void ap_task_request();

	private:
		enum thread_status {IDLE, PENDING, RUNNING};
		struct task_data
		{
			std::function<void()> function;
			std::condition_variable cond;
			std::thread thread;
			thread_status my_status;

			std::chrono::time_point<std::chrono::high_resolution_clock> start_time;


			unsigned int wcet;
			int index;

			/* ... */
		};
		std::mutex mutex;
		bool release_aperiodic;

		std::vector<task_data> p_tasks;
		task_data ap_task;

		std::vector< std::vector<size_t> > frames;


		const unsigned int frame_length; // lunghezza del frame (in quanti temporali)
		const std::chrono::milliseconds unit_time; // durata dell'unita di tempo (quanto temporale)

		/* ... */

		static void task_function(task_data & task, std::mutex &mtx);

		void exec_function();
};

#endif
