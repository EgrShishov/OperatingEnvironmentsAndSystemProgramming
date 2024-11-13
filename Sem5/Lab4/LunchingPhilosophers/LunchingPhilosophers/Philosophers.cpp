#include <iostream>
#include <vector>
#include <thread>
#include <windows.h>
#include <atomic>
#include <fstream>
#include <random>

int philosophers_count;
int min_thinking_time;
int max_thinking_time;
int min_eating_time;
int max_eating_time;
int working_time = 10000;
int time_out = 100;
int strategy;

struct PhilosopherStat {
    int id;
    int meals = 0;
    double total_thinking_time = 0;
    double total_eating_time = 0;
    double total_waiting_time = 0;
};

static int generate_random_time(int min_time, int max_time) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(min_time, max_time);
    return dist(gen);
}

HANDLE* forks;
HANDLE waiter_semaphore; // Semaphore for the waiter
std::atomic<bool> running(true); // Flag to stop philosophers

void think(int id, PhilosopherStat& stat, int min_thinking_time, int max_thinking_time) {
    std::cout << "Философ " << id << " думает." << std::endl;
    auto thinking_start = GetTickCount();
    Sleep(generate_random_time(min_thinking_time, max_thinking_time));
    auto thinking_end = GetTickCount();
    stat.total_thinking_time += (thinking_end - thinking_start);
}

void eat(int id, PhilosopherStat& stat, int min_eating_time, int max_eating_time) {
    std::cout << "Философ " << id << " начинает есть." << std::endl;
    auto eating_start = GetTickCount();
    Sleep(generate_random_time(min_eating_time, max_eating_time));
    auto eating_end = GetTickCount();
    stat.total_eating_time += (eating_end - eating_start);
    ++stat.meals;
    std::cout << "Философ " << id << " закончил есть." << std::endl;
}

void random_strategy(int id, PhilosopherStat& stat) {
    int left_fork = id;
    int right_fork = (id + 1) % philosophers_count;

    while (running) {
        think(id, stat, min_thinking_time, max_thinking_time);
        std::cout << "Философ " << id << " проголодался." << std::endl;

        auto waiting_start = GetTickCount();
        while (true) {
            if (rand() % 2 == 0) {
                WaitForSingleObject(forks[left_fork], INFINITE);
                if (WaitForSingleObject(forks[right_fork], 0) == WAIT_OBJECT_0) {
                    break;
                }
                ReleaseMutex(forks[left_fork]);
            }
            else {
                WaitForSingleObject(forks[right_fork], INFINITE);
                if (WaitForSingleObject(forks[left_fork], 0) == WAIT_OBJECT_0) {
                    break;
                }
                ReleaseMutex(forks[right_fork]);
            }
            Sleep(time_out);
        }

        auto waiting_end = GetTickCount();
        stat.total_waiting_time += (waiting_end - waiting_start);
        eat(id, stat, min_eating_time, max_eating_time);
        ReleaseMutex(forks[left_fork]);
        ReleaseMutex(forks[right_fork]);
    }
}

void waiter_strategy(int id, PhilosopherStat& stat) {
    int left_fork = id;
    int right_fork = (id + 1) % philosophers_count;

    while (running) {
        think(id, stat, min_thinking_time, max_thinking_time);
        std::cout << "Философ " << id << " проголодался." << std::endl;

        auto waiting_start = GetTickCount();
        WaitForSingleObject(waiter_semaphore, INFINITE);
        WaitForSingleObject(forks[left_fork], INFINITE);
        WaitForSingleObject(forks[right_fork], INFINITE);
        auto waiting_end = GetTickCount();
        stat.total_waiting_time += (waiting_end - waiting_start);

        eat(id, stat, min_eating_time, max_eating_time);
        ReleaseMutex(forks[left_fork]);
        ReleaseMutex(forks[right_fork]);
        ReleaseSemaphore(waiter_semaphore, 1, NULL);
    }
}

void asymmetric_strategy(int id, PhilosopherStat& stat) {
    int left_fork = id;
    int right_fork = (id + 1) % philosophers_count;

    while (running) {
        think(id, stat, min_thinking_time, max_thinking_time);
        std::cout << "Философ " << id << " проголодался." << std::endl;

        auto waiting_start = GetTickCount();
        if (id == philosophers_count - 1) {
            WaitForSingleObject(forks[right_fork], INFINITE);
            WaitForSingleObject(forks[left_fork], INFINITE);
        }
        else {
            WaitForSingleObject(forks[left_fork], INFINITE);
            WaitForSingleObject(forks[right_fork], INFINITE);
        }
        auto waiting_end = GetTickCount();
        stat.total_waiting_time += (waiting_end - waiting_start);

        eat(id, stat, min_eating_time, max_eating_time);

        ReleaseMutex(forks[left_fork]);
        ReleaseMutex(forks[right_fork]);
    }
}

void print_results(const std::vector<PhilosopherStat>& philosopher_stats, const std::string& strategy_name) {
    std::cout << "\nРезультаты моделирования (" << strategy_name << "):\n";
    for (const auto& stat : philosopher_stats) {
        std::cout << "Философ: " << stat.id << " съел " << stat.meals << " раз.\n";
        std::cout << "Время размышлений: " << stat.total_thinking_time << " мс\n";
        std::cout << "Время за едой: " << stat.total_eating_time << " мс\n";
        std::cout << "Время ожидания: " << stat.total_waiting_time << " мс\n\n";
    }
}

void save_results_to_csv(const std::vector<PhilosopherStat>& philosopher_stats, const std::string& strategy_name) {
    std::ofstream csv_file("philosophers_results.csv", std::ios::app);
    if (csv_file.is_open()) {
        for (const auto& stat : philosopher_stats) {
            csv_file << stat.id << ","
                << stat.meals << ","
                << stat.total_thinking_time << ","
                << stat.total_eating_time << ","
                << stat.total_waiting_time << ","
                << strategy_name << "\n"; // Add strategy name
        }
        csv_file.close();
    }
}

DWORD WINAPI philosopher_thread(LPVOID lpParam) {
    auto* stat = static_cast<PhilosopherStat*>(lpParam);
    if (strategy == 1) {
        random_strategy(stat->id, *stat);
    }
    else if (strategy == 2) {
        waiter_strategy(stat->id, *stat);
    } else if (strategy == 3) {
        asymmetric_strategy(stat->id, *stat);
    }
    return 0;
}

void run_strategy(int strat) {
    strategy = strat;
    running = true;
    std::vector<PhilosopherStat> philosopher_stats(philosophers_count);
    std::vector<HANDLE> philosopher_threads(philosophers_count);

    for (int i = 0; i < philosophers_count; ++i) {
        philosopher_stats[i].id = i;
        philosopher_threads[i] = CreateThread(NULL, 0, philosopher_thread, &philosopher_stats[i], 0, NULL);
    }

    Sleep(working_time);
    running = false;

    WaitForMultipleObjects(philosophers_count, philosopher_threads.data(), TRUE, INFINITE);
    print_results(philosopher_stats, strat == 1 ? "Случайная стратегия" : (strat == 2 ? "Стратегия с официантом" : "Асимметричная стратегия"));
    save_results_to_csv(philosopher_stats, strat == 1 ? "Случайная стратегия" : (strat == 2 ? "Стратегия с официантом" : "Асимметричная стратегия"));

    for (int i = 0; i < philosophers_count; ++i) {
        CloseHandle(philosopher_threads[i]);
    }
}

int main() {
    setlocale(LC_ALL, "Ru");

    std::cout << "Введите количество философов: ";
    std::cin >> philosophers_count;

    std::cout << "Введите минимальное время размышлений (в мс): ";
    std::cin >> min_thinking_time;

    std::cout << "Введите максимальное время размышлений (в мс): ";
    std::cin >> max_thinking_time;

    std::cout << "Введите минимальное время еды (в мс): ";
    std::cin >> min_eating_time;

    std::cout << "Введите максимальное время еды (в мс): ";
    std::cin >> max_eating_time;

    std::cout << "Введите время работы программы (в мс): ";
    std::cin >> working_time;

    std::cout << "Введите время таймаута (в мс): ";
    std::cin >> time_out;

    forks = new HANDLE[philosophers_count];
    for (int i = 0; i < philosophers_count; ++i) {
        forks[i] = CreateMutex(NULL, FALSE, NULL);
    }
    waiter_semaphore = CreateSemaphore(NULL, philosophers_count - 1, philosophers_count - 1, NULL);

    run_strategy(1);
    run_strategy(2);
    run_strategy(3);

    for (int i = 0; i < philosophers_count; ++i) {
        CloseHandle(forks[i]);
    }
    CloseHandle(waiter_semaphore);
    delete[] forks;

    return 0;
}