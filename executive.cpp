#include <cassert>
#include <algorithm>
#include "executive.h"

#define VERBOSE

#define IDLE 0
#define PENDING 1
#define RUNNING 2
#define MISSED 3

Executive::Executive(size_t num_tasks, unsigned int frame_length, unsigned int unit_duration)
	: p_tasks(num_tasks), frame_length(frame_length), unit_time(unit_duration)
{
}

void Executive::set_periodic_task(size_t task_id, std::function<void()> periodic_task, unsigned int /* wcet */)
{
	assert(task_id < p_tasks.size());
	p_tasks[task_id].function = periodic_task;
	p_tasks[task_id].was_missed = false;
}

void Executive::add_frame(std::vector<size_t> frame)
{
	for (auto& id : frame)
		assert(id < p_tasks.size());

	frames.push_back(frame);
}

void Executive::start()
{
	rt::priority prio(rt::priority::rt_max);
	rt::affinity aff("1");

	for (size_t i = 0; i < p_tasks.size(); ++i) {
		assert(p_tasks[i].function);

		p_tasks[i].thread = std::thread(&Executive::task_function, std::ref(p_tasks[i]), std::ref(mutex));
		p_tasks[i].status = IDLE;
		p_tasks[i].index = i;

		rt::set_affinity(p_tasks[i].thread, aff);

		p_tasks[i].priority = --prio;

		try {
			rt::set_priority(p_tasks[i].thread, p_tasks[i].priority);
		} catch (rt::permission_error& e) {
			std::cout << "Failed to set priority" << e.what() << std::endl;
		}
	}

	exec_thread = std::thread(&Executive::exec_function, this);

	rt::set_affinity(exec_thread, aff);

	try {
		rt::set_priority(exec_thread, rt::priority::rt_max);
	} catch (rt::permission_error& e) {
		std::cout << "Failed to set priority" << e.what() << std::endl;
	}

	if (stats_observer) {
		stats_thread = std::thread(&Executive::stats_function, this);
		rt::set_affinity(stats_thread, aff);
		try {
			rt::set_priority(stats_thread, rt::priority::not_rt);
		} catch (rt::permission_error& e) {
			std::cout << "Failed to set priority" << e.what() << std::endl;
		}
	}
}

void Executive::wait()
{
	if (stats_thread.joinable())
		stats_thread.join();

	exec_thread.join();

	for (auto& pt : p_tasks)
		pt.thread.join();
}

void Executive::task_function(Executive::task_data& task, std::mutex& mutex)
{
	while (true) {
		{
			std::unique_lock<std::mutex> l(mutex);
			while (task.status == IDLE) {
				task.cond.wait(l);
			}
			task.status = RUNNING;
		}
		auto start = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> release(start - task.start_time);

		task.function();

		auto stop = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> elapsed(stop - start);

		{
			std::unique_lock<std::mutex> l(mutex);
			task.status = IDLE;
			if (task.was_missed == false) {
				if (task.stats.exec_count > 0) {

					task.stats.avg_exec_time = (task.stats.avg_exec_time * task.stats.exec_count + elapsed.count()) / (task.stats.exec_count + 1);
				} else {
					task.stats.avg_exec_time = elapsed.count();
				}

				if (task.stats.max_exec_time < elapsed.count()) {
					task.stats.max_exec_time = elapsed.count();
				}
#ifdef VERBOSE
				std::cout << "- Task " << task.index << " "
					<< "Elapsed [ms]: " << elapsed.count() << std::endl;
#endif
			}
		}
#ifdef VERBOSE
		std::cout << "- Task " << task.index << " e ho terminato la mia esecuzione" << std::endl;
#endif
	}
}

void Executive::exec_function()
{
	unsigned int frame_id = 0;

	auto frame_n = 0;
	auto hyperperiod_n = 0;

	auto point = std::chrono::steady_clock::now();
	auto start = std::chrono::high_resolution_clock::now();
	for (auto& pt : p_tasks) {
		pt.start_time = std::chrono::high_resolution_clock::now();
	}

	while (true) {
		std::vector<task_stats> singleStats;
#ifdef VERBOSE
		std::cout << "*** Frame n." << frame_id << (frame_id == 0 ? " ******" : "") << std::endl;
#endif
		for (int i = 0; i < p_tasks.size(); i++) {
			try {
				{
					std::unique_lock<std::mutex> lock(mutex);
					if (std::count(frames[frame_id].begin(), frames[frame_id].end(), i)) {
						if (p_tasks[i].status == IDLE) {
							rt::set_priority(p_tasks[i].thread, p_tasks[i].priority);
							p_tasks[i].status = PENDING;
#ifdef VERBOSE
							std::cout << "Schedulato (contenuto nel Frame) - ID, Stato: " << i << " " << p_tasks[i].status << std::endl;
#endif
							p_tasks[i].cond.notify_one();
						} else if (p_tasks[i].status == MISSED) {
							p_tasks[i].stats.canc_count++;
						}
					}

					if (p_tasks[i].status == MISSED) {
						rt::set_priority(p_tasks[i].thread, rt::priority::rt_min);
#ifdef VERBOSE
						std::cout << "WAS MISSED - ID, Stato: " << i << " " << p_tasks[i].status << std::endl;
#endif
						p_tasks[i].cond.notify_one();
					}
					p_tasks[i].stats.task_id = i;
					p_tasks[i].stats.cycle_id = hyperperiod_n;
				}
			} catch (rt::permission_error& e) {
				std::cout << "Failed to set priority" << e.what() << std::endl;
			}
		}

		point += std::chrono::milliseconds(frame_length * unit_time);
		std::this_thread::sleep_until(point);

		for (int i = 0; i < p_tasks.size(); i++) {
			{
				std::unique_lock<std::mutex> l(mutex);
				if (p_tasks[i].status == RUNNING) {
					std::cout << "Il TASK va in Deadline: " << i << std::endl;
					try {
						rt::set_priority(p_tasks[i].thread, rt::priority::not_rt);
					} catch (rt::permission_error& e) {
						std::cout << "Failed to set priority" << e.what() << std::endl;
					}

					p_tasks[i].status = MISSED;
					p_tasks[i].was_missed = true;
					p_tasks[i].stats.miss_count++;
				} else if (p_tasks[i].status == PENDING) {
					if (p_tasks[i].was_missed) {
#ifdef VERBOSE
						std::cout << "DEADLINE MISS: missed -> pending -> missed.  TASK: " << i << std::endl;
#endif
						p_tasks[i].status = MISSED;
						p_tasks[i].stats.miss_count++;
					} else {
#ifdef VERBOSE
						std::cout << "DEADLINE MISS: idle -> pending -> idle.  TASK: " << i << std::endl;
#endif
						p_tasks[i].stats.canc_count++;
						p_tasks[i].status = IDLE;
					}
				} else if (p_tasks[i].status == IDLE) {
					if (p_tasks[i].was_missed) {
#ifdef VERBOSE
						std::cout << "Task uscito dalla deadline. TASK: " << i << std::endl;
#endif
						p_tasks[i].was_missed = false;
					} else if (std::count(frames[frame_id].begin(), frames[frame_id].end(), i)) {
						p_tasks[i].stats.exec_count++;
					}

				}
				singleStats.push_back(p_tasks[i].stats);
			}

		}

		{
			std::unique_lock<std::mutex> l(mutex);
			buffer.push_back(singleStats);
			cond_buffer.notify_one();
		}

		auto next = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> elapsed(next - start);
		start = next;

#ifdef VERBOSE
		std::cout << "--- Frame duration: " << elapsed.count() << "ms" << " ---" << std::endl;
#endif

		if (++frame_id == frames.size()) {
			frame_id = 0;
			frame_n++;
			hyperperiod_n++;


			for (int i = 0; i < singleStats.size(); i++) {
				{
					std::unique_lock<std::mutex> lock(mutex);
					global_statistic.canc_count += p_tasks[i].stats.canc_count;
					global_statistic.miss_count += p_tasks[i].stats.miss_count;
					global_statistic.exec_count += p_tasks[i].stats.exec_count;
					global_statistic.cycle_count = hyperperiod_n;

				}
			}
			singleStats.clear();



		}
	}
}

void Executive::set_stats_observer(std::function<void(task_stats const&)> obs)
{
	stats_observer = obs;
}

global_stats Executive::get_global_stats()
{
	return global_statistic;
}

void Executive::stats_function()
{
	while (true) {
		std::vector<task_stats> actualTask;
		{
			std::unique_lock<std::mutex> l(mutex);
			while (buffer.empty()) {
				cond_buffer.wait(l);
			}
			actualTask = buffer.front(); // Consumer
			buffer.pop_front();
		}

		for (size_t i = 0; i < actualTask.size(); i++) {
			stats_observer(actualTask[i]);
		}
	}
}
