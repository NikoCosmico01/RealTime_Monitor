#include <cassert>
#include <algorithm>
#include "executive.h"
#define IDLE 0
#define PENDING 1
#define RUNNING 2
#define MISSED 3

Executive::Executive(size_t num_tasks, unsigned int frame_length, unsigned int unit_duration)
	: p_tasks(num_tasks), frame_length(frame_length), unit_time(unit_duration)
{
}

void Executive::set_periodic_task(size_t task_id, std::function<void()> periodic_task, unsigned int wcet)
{
	assert(task_id < p_tasks.size()); // Fallisce in caso di task_id non corretto (fuori range)

	p_tasks[task_id].function = periodic_task;
	p_tasks[task_id].wcet = wcet * unit_time.count();
	p_tasks[task_id].was_missed = false;
}

void Executive::add_frame(std::vector<size_t> frame)
{
	for (auto &id : frame)
		assert(id < p_tasks.size()); // Fallisce in caso di task_id non corretto (fuori range)

	frames.push_back(frame);

	/* ... */
}

void Executive::run()
{
	rt::priority prio(rt::priority::rt_max); // Lo mette a -1 perchè mette l'executive a rt_max
	rt::affinity aff("1");					 // Poichè schedulazione monoprocessore

	for (size_t id = 0; id < p_tasks.size(); ++id)
	{
		assert(p_tasks[id].function); // Fallisce se set_periodic_task() non e' stato invocato per questo id

		p_tasks[id].thread = std::thread(&Executive::task_function, std::ref(p_tasks[id]), std::ref(mutex));
		p_tasks[id].status = IDLE;
		p_tasks[id].index = id;

		rt::set_affinity(p_tasks[id].thread, aff);

		p_tasks[id].priority = --prio;

		try
		{
			rt::set_priority(p_tasks[id].thread, p_tasks[id].priority);
		}
		catch (rt::permission_error &)
		{
			std::cerr << "Warning: RT priorities are not available" << std::endl;
		}
	}

	std::thread exec_thread(&Executive::exec_function, this);

	rt::set_affinity(exec_thread, aff);

	try
	{
		rt::set_priority(exec_thread, rt::priority::rt_max);
	}
	catch (rt::permission_error &)
	{
		std::cerr << "Warning: RT priorities are not available" << std::endl;
	}
	/* ... */

	exec_thread.join();

	for (auto &pt : p_tasks)
		pt.thread.join();
}

void Executive::task_function(Executive::task_data &task, std::mutex &mutex)
{
	while (true)
	{
		{
			std::unique_lock<std::mutex> l(mutex);
			while (task.status == IDLE)
			{ // status 0 sospeso, 1 attivo
				task.cond.wait(l);
			}
			task.status = RUNNING;
		}
		task.function();

		{
			std::unique_lock<std::mutex> l(mutex);
			std::cout << "sono il task e ho terminato la mia esecuzione" << task.index << std::endl;
			task.was_missed = false;
			task.status = IDLE;
		}
	}
}

void Executive::exec_function()
{
	unsigned int frame_id = 0;
	unsigned int slack_time = 0;

	std::cout << std::endl;
	std::cout << "--- Executive, priorità: " << rt::this_thread::get_priority() << std::endl;
	auto frame_n = 0;
	auto wcet_tot = 0;

	auto point = std::chrono::steady_clock::now();
	auto start = std::chrono::high_resolution_clock::now();

	while (true)
	{
		{
			/* Rilascio dei task periodici del frame corrente*/
			std::unique_lock<std::mutex> l(mutex); // mutex acquired here

			wcet_tot = 0;
			slack_time = 0;

			std::cout << "---------- Frame: " << frame_id + 1 << " ----------" << std::endl;

			// Scheduliamo in task che non i task che non sono in DEALINE
			for (int i = 0; i < p_tasks.size(); i++)
			{





		


				if (p_tasks[i].status == IDLE && std::count(frames[frame_id].begin(),frames[frame_id].end(),i))
				{

					try
					{
						rt::set_priority(p_tasks[i].thread, p_tasks[i].priority);
					}
					catch (rt::permission_error &)
					{
						std::cerr << "Warning: RT priorities are not available" << std::endl;
					}
					p_tasks[i].status = PENDING;
					p_tasks[i].cond.notify_one();
					// std::cout<< "No Dead ID, Stato: " << id << " " << p_tasks[id].status<<std::endl;
				}

				if (p_tasks[i].status == MISSED)
				{
					std::cout << "leggo il missed" << std::endl;

					try
					{
						rt::set_priority(p_tasks[i].thread, rt::priority::rt_min);
					}
					catch (rt::permission_error &)
					{
						std::cerr << "Warning: RT priorities are not available" << std::endl;
					}
					p_tasks[i].was_missed = true;
					p_tasks[i].cond.notify_one();
				}
			}
		} // CHIUDO MUTEX

		// attesa fino al prossimo inizio frame
		point += std::chrono::milliseconds(frame_length * unit_time);
		std::this_thread::sleep_until(point);

		/* Controllo delle deadline periodiche... */
		{
			std::unique_lock<std::mutex> l(mutex);
			for (int i = 0; i < frames[frame_id].size(); i++)
			{
				size_t id = frames[frame_id][i];
				if (p_tasks[id].status == RUNNING)
				{
					std::cout << "Prima volta che il task va in deadline. TASK: " << id << std::endl;
					try
					{
						rt::set_priority(p_tasks[id].thread, rt::priority::not_rt);
						std::cout << "ho settato la prio del task: " << id << " a " << rt::priority::not_rt << std::endl;
					}
					catch (rt::permission_error &)
					{
						std::cerr << "Warning: RT priorities are not available" << std::endl;
					}

					p_tasks[id].status = MISSED;
					p_tasks[id].was_missed = true;
				}
				else if (p_tasks[id].status == PENDING)
				{
					std::cout << "DEADLINE MISS: missed -> pending -> missed.  TASK: " << id << std::endl;
					if (p_tasks[id].was_missed == true)
					{
						// missed -> pending -> missed
						p_tasks[id].status = MISSED;
					}
					else
					{
						std::cout << "DEADLINE MISS: idle -> pending -> idle.  TASK: " << id << std::endl;
						// Non eseguire un task in deadline miss che non ha iniziato l'esecuzione: idle -> pending -> idle
						p_tasks[id].status = IDLE;
					}
				}
				else
				{
					if (p_tasks[id].was_missed == true)
					{
						std::cout << "Task USCITO dalla deadline. TASK: " << id << std::endl;
						p_tasks[id].was_missed = false;
					}
				}

				std::cout << "FINE FRAME ID, Status: " << 2 << " " << p_tasks[2].status << " " << p_tasks[2].was_missed << std::endl;
			}

			// Running -> deadline

			auto next = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double, std::milli> elapsed(next - start);
			start = next;

			std::cout << "--- Frame duration: " << elapsed.count() << "ms"
					  << " ---" << std::endl;

			if (++frame_id == frames.size())
			{
				frame_id = 0;
				frame_n++;
			}
		}
	}
}