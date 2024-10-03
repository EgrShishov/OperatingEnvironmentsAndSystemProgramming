#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <chrono>
#include <Windows.h>

struct PhilosophersStat {
    int id;
    int meals = 0;
    double total_thinking_time = 0;
    double total_eating_time = 0;
    double total_waiting_time = 0;
};

class DinningPhilosophers {

public:

    DinningPhilosophers(int philosophers_amount, int max_philosophers,
                        int min_thinking, int max_thinking, 
                        int min_eating, int max_eating, int strategy, 
                        int timeout_duration) {
        this->max_philosophers = max_philosophers;
        this->philosophers_amount = philosophers_amount;
        this->time_out_size = time_out_size;
        this->strategy = strategy;
        this->max_thinking_time = max_thinking;
        this->max_eating_time = max_eating;
        this->min_thinking_time = min_thinking;
        this->min_eating_time = min_eating; 

        this->philosophers_stat.resize(philosophers_amount);
        this->philosophers_indicies.resize(philosophers_amount);
        this->clean_forks.resize(philosophers_amount);
        this->fork_requests.resize(philosophers_amount);
        this->forks.resize(philosophers_amount);
        this->philosophers.resize(philosophers_amount);
        this->managers.resize(philosophers_amount);

        launch_task(this->strategy);

        this->running = true;
	}

	~DinningPhilosophers() {
        for (int i = 0; i < philosophers_amount; ++i) {
            CloseHandle(philosophers[i]);
            CloseHandle(forks[i]);
            CloseHandle(fork_requests[i]);
            CloseHandle(managers[i]);
        }
	}

    void launch_task(int strategy) {
        if (strategy == 3) {
            init_waiter();
        }

        for (int i = 0; i < philosophers_amount; ++i) {
            philosophers_stat[i].id = i;
            this->forks[i] = CreateMutex(nullptr, false, nullptr);
            this->fork_requests[i] = CreateEvent(nullptr, false, false, nullptr);
            this->clean_forks[i] = true;
            this->philosophers_indicies[i] = i;

            switch (strategy) {
                case 1: {
                    this->philosophers[i] = CreateThread
                    (nullptr,
                        0,
                        assymetric_fork_grub,
                        &philosophers_indicies[i],
                        0,
                        nullptr);
                    this->managers[i] = CreateThread(
                        nullptr,
                        0,
                        manage_forks,
                        &philosophers_indicies[i],
                        0,
                        nullptr);
                    break;
                }
                case 2: {
                    this->philosophers[i] = CreateThread
                    (nullptr,
                        0,
                        randomize_strategy,
                        &philosophers_indicies[i],
                        0,
                        nullptr);
                    break;
                }
                case 3: {
                    this->philosophers[i] = CreateThread
                    (nullptr,
                        0,
                        waiter_strategy,
                        &philosophers_indicies[i],
                        0,
                        nullptr);
                    break;
                }
                default: {
                    std::cout << "Что-то пошло не так в заполнении: несуществующая стратегия" << std::endl;
                    break;
                }

            }
        }
    }

	static DWORD WINAPI assymetric_fork_grub(LPVOID lpParametr) {
        int id = *reinterpret_cast<int*>(lpParametr);
		int left_fork = id;
		int right_fork = (id + 1) % philosophers_amount;
        bool has_left_fork = false;
        bool has_right_fork = false;

		while (running) {
            std::cout << "Философ " << id << " думает" << std::endl;
            auto thinking_start = std::chrono::high_resolution_clock::now();
            Sleep(generate_random_time(min_thinking_time, max_thinking_time));
            auto thinking_end = std::chrono::high_resolution_clock::now();

            philosophers_stat[id].total_thinking_time += std::chrono::duration_cast<std::chrono::milliseconds>(thinking_end - thinking_start).count();

            std::cout << "Философ " << id << " проголадался" << std::endl;
            auto waiting_start = std::chrono::high_resolution_clock::now();

            WaitForSingleObject(forks[left_fork], time_out_size);
            if (!clean_forks[left_fork]) {
                std::cout << "Философ " << id << " просит левую филку" << std::endl;
                SetEvent(fork_requests[left_fork]);
            }
            ReleaseMutex(forks[left_fork]);

            WaitForSingleObject(forks[right_fork], time_out_size);
            if (!clean_forks[right_fork]) {
                std::cout << "Философ " << id << " просит правую вилку" << std::endl;
                SetEvent(fork_requests[right_fork]);
            }
            ReleaseMutex(forks[right_fork]);

            while (!(has_left_fork && has_right_fork)) {
                WaitForSingleObject(forks[left_fork], time_out_size);
                if (clean_forks[left_fork]) {
                    has_left_fork = true;
                }
                ReleaseMutex(forks[left_fork]);

                WaitForSingleObject(forks[right_fork], time_out_size);
                if (clean_forks[right_fork]) {
                    has_right_fork = true;
                }
                ReleaseMutex(forks[right_fork]);
            }

            auto waiting_end = std::chrono::high_resolution_clock::now();
            philosophers_stat[id].total_waiting_time += 
                std::chrono::duration_cast<std::chrono::milliseconds>(waiting_end - waiting_start).count();

            std::cout << "Философ " << id << " начал трапезу" << std::endl;
            auto eating_start = std::chrono::high_resolution_clock::now();
            Sleep(generate_random_time(min_eating_time, max_eating_time));

            WaitForSingleObject(forks[left_fork], INFINITE);
            clean_forks[left_fork] = false;
            ReleaseMutex(forks[left_fork]);

            WaitForSingleObject(forks[right_fork], INFINITE);
            clean_forks[right_fork] = false;
            ReleaseMutex(forks[right_fork]);
            auto eating_end = std::chrono::high_resolution_clock::now();

            philosophers_stat[id].total_eating_time += 
                std::chrono::duration_cast<std::chrono::milliseconds>(eating_end - eating_start).count();
            ++philosophers_stat[id].meals;

            std::cout << "Филосов " << id << " завершил трапезу" << std::endl;
		}

        return 0;
	}

    static DWORD WINAPI randomize_strategy(LPVOID lpParametr) {
        int id = *reinterpret_cast<int*>(lpParametr);
        int left_fork = id;
        int right_fork = (id + 1) % philosophers_amount;

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(0, 1);

        bool has_left_fork = false;
        bool has_right_fork = false;

        while (running) {
            std::cout << "Философ " << id << " думает" << std::endl;
            auto thinking_start = std::chrono::high_resolution_clock::now();
            Sleep(generate_random_time(min_thinking_time, max_thinking_time));
            auto thinking_end = std::chrono::high_resolution_clock::now();

            philosophers_stat[id].total_thinking_time +=
                std::chrono::duration_cast<std::chrono::milliseconds>(thinking_end - thinking_start).count();

            std::cout << "Философ " << id << " проголадался" << std::endl;
            auto waiting_start = std::chrono::high_resolution_clock::now();
            int choice = dist(gen);

            while (!(has_left_fork && has_right_fork)) {
                if (choice) {
                    WaitForSingleObject(forks[left_fork], time_out_size);
                    if (clean_forks[left_fork]) {
                        has_left_fork = true;
                    } else {
                        std::cout << "Философ " << id << " запрашивает чистку левой вилки у соседа" << std::endl;
                        SetEvent(fork_requests[left_fork]);
                    }
                    ReleaseMutex(forks[left_fork]);

                    WaitForSingleObject(forks[right_fork], time_out_size);
                    if (clean_forks[right_fork]) {
                        has_right_fork = true;
                    } else {
                        std::cout << "Философ " << id << " запрашивает чистку правой вилки у соседа" << std::endl;
                        SetEvent(fork_requests[right_fork]);
                    }
                    ReleaseMutex(forks[right_fork]);
                } else {
                    WaitForSingleObject(forks[right_fork], time_out_size);
                    if (clean_forks[right_fork]) {
                        has_right_fork = true;
                    }
                    else {
                        std::cout << "Философ " << id << " запрашивает чистку правой вилки у соседа" << std::endl;
                        SetEvent(fork_requests[right_fork]);
                    }
                    ReleaseMutex(forks[right_fork]);

                    WaitForSingleObject(forks[left_fork], time_out_size);
                    if (clean_forks[left_fork]) {
                        has_left_fork = true;
                    }
                    else {
                        std::cout << "Философ " << id << " запрашивает чистку левой вилки у соседа" << std::endl;
                        SetEvent(fork_requests[left_fork]);
                    }
                    ReleaseMutex(forks[left_fork]);
                }

                if (!has_left_fork || !has_right_fork) {
                    Sleep(time_out_size);
                }
            }
           
            auto waiting_end = std::chrono::high_resolution_clock::now();
            
            philosophers_stat[id].total_waiting_time +=
                std::chrono::duration_cast<std::chrono::milliseconds>(waiting_end - waiting_start).count();

            std::cout << "Философ " << id << " трапезничает " << std::endl;

            auto eating_start = std::chrono::high_resolution_clock::now();
            
            Sleep(generate_random_time(min_eating_time, max_eating_time));
           
            WaitForSingleObject(forks[left_fork], INFINITE);
            clean_forks[left_fork] = false;
            ReleaseMutex(forks[left_fork]);
           
            WaitForSingleObject(forks[right_fork], INFINITE);
            clean_forks[left_fork] = false;
            ReleaseMutex(forks[right_fork]);

            auto eating_end = std::chrono::high_resolution_clock::now();
            
            philosophers_stat[id].total_eating_time +=
                std::chrono::duration_cast<std::chrono::milliseconds>(eating_end - eating_start).count();
            ++philosophers_stat[id].meals;

            std::cout << "Философ " << id << " завершил трапезу" << std::endl;
        }
        return 0;
    }

    // waiters work inefficient
    static DWORD WINAPI waiter_strategy(LPVOID lpParametr) {
        int id = *reinterpret_cast<int*>(lpParametr);
        int left_fork = id;
        int right_fork = (id + 1) % philosophers_amount;

        bool has_left_fork = false;
        bool has_righ_fork = false;

        while (running) {
            std::cout << "Философ " << id << " думает" << std::endl;
            auto thinking_start = std::chrono::high_resolution_clock::now();
            Sleep(generate_random_time(min_thinking_time, max_thinking_time));
            auto thinking_end = std::chrono::high_resolution_clock::now();

            philosophers_stat[id].total_thinking_time +=
                std::chrono::duration_cast<std::chrono::milliseconds>(thinking_end - thinking_start).count();

            std::cout << "Философ " << id << " проголадался" << std::endl;
            auto waiting_start = std::chrono::high_resolution_clock::now();
            WaitForSingleObject(waiter, INFINITE);

            WaitForSingleObject(forks[left_fork], time_out_size);
            WaitForSingleObject(forks[right_fork], time_out_size);

            auto waiting_end = std::chrono::high_resolution_clock::now();
            philosophers_stat[id].total_waiting_time 
                += std::chrono::duration_cast<std::chrono::milliseconds>(waiting_end - waiting_start).count();

            std::cout << "Филосов " << id << " трапезничает" << std::endl;
            auto eating_start = std::chrono::high_resolution_clock::now();
            Sleep(generate_random_time(min_eating_time, max_eating_time));

            ReleaseMutex(forks[left_fork]);
            ReleaseMutex(forks[right_fork]);
            ReleaseSemaphore(waiter, 1, nullptr);
            auto eating_end = std::chrono::high_resolution_clock::now();

            philosophers_stat[id].total_eating_time +=
                std::chrono::duration_cast<std::chrono::milliseconds>(eating_end - eating_start).count();
            ++philosophers_stat[id].meals;
        }

        return 0;
    }

    static DWORD manage_forks(LPVOID lpParametr) {
        int fork_id = *reinterpret_cast<int*>(lpParametr);
        int left_philosopher = fork_id;
        int right_philosopher = (fork_id + 1) % philosophers_amount;

        while (running) {
            WaitForSingleObject(fork_requests[fork_id], INFINITE);
            WaitForSingleObject(forks[fork_id], INFINITE);

            if (!clean_forks[fork_id]) {
                std::cout << "Вилка " << fork_id << " очищена" << std::endl;
                clean_forks[fork_id] = true;
                ReleaseMutex(forks[fork_id]);
            }
            else {
                std::cout << "Вилка " << fork_id << " чистая" << std::endl;
            }

        }

        return 0;
    }

    static void print_results() {
        std::cout << "\nРезультаты моделирования:\n";
        for (const auto& stat : philosophers_stat) {
            std::cout << "Философ: " << stat.id << " съел " << stat.meals << " раз.\n";
            std::cout << "Время размышлений: " << stat.total_thinking_time << " мс\n";
            std::cout << "Время за едой: " << stat.total_eating_time << " мс\n";
            std::cout << "Время ожидания: " << stat.total_waiting_time << " мс\n\n";
        }
    }

    void init_waiter() {
        waiter = CreateSemaphore(nullptr, max_philosophers - 1, max_philosophers - 1, nullptr);
    }

    static int philosophers_amount;
    static int max_philosophers;
    static int time_out_size;
    static int strategy;
    static int min_thinking_time;
    static int max_thinking_time;
    static int min_eating_time;
    static int max_eating_time;
    static HANDLE waiter;
    static std::vector<HANDLE> forks; 
    static std::vector<bool> clean_forks;
    static std::vector<HANDLE> fork_requests;
    static std::vector<HANDLE> philosophers;
    static std::vector<HANDLE> managers;
    static std::vector<int> philosophers_indicies;
    static std::vector<PhilosophersStat> philosophers_stat;
    static int max_meals;
    static bool running;

private:

    static int generate_random_time(int min_time, int max_time) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(min_time, max_time);
        return dist(gen);
    }
};

int DinningPhilosophers::philosophers_amount = 0;
int DinningPhilosophers::max_philosophers = 0;
int DinningPhilosophers::time_out_size = 0;
bool DinningPhilosophers::running = false;
std::vector<HANDLE> DinningPhilosophers::forks;
std::vector<HANDLE> DinningPhilosophers::fork_requests;
std::vector<HANDLE> DinningPhilosophers::philosophers;
std::vector<int> DinningPhilosophers::philosophers_indicies;
std::vector<bool> DinningPhilosophers::clean_forks;
std::vector<HANDLE> DinningPhilosophers::managers;
int DinningPhilosophers::max_meals = 0;
int DinningPhilosophers::strategy = 0;
int DinningPhilosophers::min_thinking_time = 0;
int DinningPhilosophers::max_thinking_time = 0;
int DinningPhilosophers::min_eating_time = 0;
int DinningPhilosophers::max_eating_time = 0;
std::vector<PhilosophersStat> DinningPhilosophers::philosophers_stat;
HANDLE DinningPhilosophers::waiter;

int main() {
    setlocale(LC_ALL, "Ru");
    int philosophers_amount = 0;
    int max_philosophers = 0;
    int min_thinking_time, max_thinking_time;
    int min_eating_time, max_eating_time;
    int strategy;

    std::cout << "Введите количество философов: ";
    std::cin >> philosophers_amount;

    std::cout << "Введите максимальное количество философов, которые обедают за раз вместе: ";
    std::cin >> max_philosophers;
    
    std::cout << "Выберите стратегию разрешения конфликта:\n";
    std::cout << "1 - Стратегия ассиметричного захвата вилок\n";
    std::cout << "2 - Стратегия случайного захвата вилок\n";
    std::cout << "3 - Стратегия централизованного контроля (наличие оффицианта)\n";
    std::cin >> strategy;

    std::cout << "Введите минимальное время размышления (мс): ";
    std::cin >> min_thinking_time;

    std::cout << "Введите максимальное время размышления (мс): ";
    std::cin >> max_thinking_time;

    std::cout << "Введите минимальное время еды (мс): ";
    std::cin >> min_eating_time;   
    
    std::cout << "Введите максимальное время еды (мс): ";
    std::cin >> max_eating_time;

    int timeout_duration;
    std::cout << "Введите тайм-аут ожидания вилки (мс): ";
    std::cin >> timeout_duration;

    if (timeout_duration == 0)
        timeout_duration = INFINITE;

    DinningPhilosophers dp = DinningPhilosophers(
        philosophers_amount,
        max_philosophers,
        min_thinking_time,
        max_thinking_time,
        min_eating_time,
        max_eating_time,
        strategy,
        timeout_duration);

    /*WaitForMultipleObjects(
        philosophers_amount,
        DinningPhilosophers::philosophers.data(),
        true, 
        INFINITE);*/

    Sleep(10000);

    dp.print_results();
    return 0;
}