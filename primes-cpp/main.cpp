#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

using namespace std;

void sieve(queue<int> &pipe_in, mutex &mtx_prev, condition_variable &cond_var_prev) {
    int divisor = 0;
    {
        unique_lock<mutex> guard(mtx_prev);
        while (pipe_in.empty()) {
            cond_var_prev.wait(guard);
        }
        divisor = pipe_in.front();
        pipe_in.pop();
        cout << "prime " << divisor << "\n" << flush;
    }

    queue<int> pipe_out;
    mutex mtx_next;
    condition_variable cond_var_next;
    thread next_sieve;

    int number = 0;
    while (number != -1) {
        {
            unique_lock<mutex> guard(mtx_prev);
            while (pipe_in.empty()) {
                cond_var_prev.wait(guard);
            }

            number = pipe_in.front();
            pipe_in.pop();
        }

        if (number == -1 && !next_sieve.joinable()) {
            // next_sieve is not created yet
            return;
        }
        if (number % divisor == 0) {
            continue;
        }

        if (!next_sieve.joinable()) {
            next_sieve = thread(sieve, ref(pipe_out), ref(mtx_next), ref(cond_var_next));
        }

        {
            unique_lock<mutex> guard(mtx_next);
            pipe_out.push(number);
        }
        cond_var_next.notify_one();
    }

    next_sieve.join();
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("error: primes expects 1 argument\n");
        exit(1);
    }

    queue<int> pipe;
    mutex mtx;
    condition_variable cond_var;

    auto first_sieve = thread(sieve, ref(pipe), ref(mtx), ref(cond_var));
    for (int num = 2; num < atoi(argv[1]); ++num) {
        {
            unique_lock<mutex> guard(mtx);
            pipe.push(num);
        }
        cond_var.notify_one();
    }
    {
        lock_guard<mutex> guard(mtx);
        pipe.push(-1);
        // end signal
    }
    cond_var.notify_one();

    first_sieve.join();
}
