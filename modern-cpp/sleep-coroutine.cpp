#include <iostream>
#include <chrono>
#include <random>
#include <coroutine>
#include <vector>

// A simple coroutine that sleeps for a random amount of time
// and returns the amount of time slept

class SleepCoroutine {
public:
    SleepCoroutine() = default;

    // The coroutine promise type
    struct promise_type {
        auto get_return_object() {
            return SleepCoroutine{handle_type::from_promise(*this)};
        }
        std::suspend_never initial_suspend() {
            return {};
        }
        std::suspend_always final_suspend() noexcept {
            return {};
        }
        void unhandled_exception() {
            std::terminate();
        }
        void return_void() {}
    };

    // The coroutine handle type
    using handle_type = std::coroutine_handle<promise_type>;

    // The coroutine execution function
    void operator()() {
        // Generate a random sleep time
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distr(1000, 5000);
        int sleep_time = distr(gen);

        // Sleep for the random time
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
    }

private:
    SleepCoroutine(handle_type h) : handle(h) {}
    handle_type handle;
};

// A function that creates and runs four sleep coroutines
void run_sleep_coroutines() {
    // Create a vector to hold the coroutines
    std::vector<SleepCoroutine::handle_type> coroutines;

    // Create and start the coroutines
    for (int i = 0; i < 4; i++) {
        coroutines.push_back(SleepCoroutine{}());
    }

    // Join the coroutines
    for (auto& coroutine : coroutines) {
        coroutine.resume();
        coroutine.destroy();
    }
}

// The main function
int main() {
    run_sleep_coroutines();
    std::cout << "All coroutines joined." << std::endl;
    return 0;
}

