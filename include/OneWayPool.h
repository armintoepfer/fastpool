// Author: Armin Töpfer

#pragma once

#include <set>
#include <thread>
#include <vector>

#include <blockingconcurrentqueue.h>

namespace XLR {

template <typename I>
class OneWayPool
{
public:
    OneWayPool(const size_t numThreads, const std::function<void(I&)> wrkFct,
               const size_t inputQUpperBound = 100)
        : inputQueue({inputQUpperBound, numThreads, numThreads})
    {
        for (size_t i = 0; i < numThreads; ++i) {
            this->threads_.emplace_back(std::thread([this, wrkFct]() {
                typename moodycamel::BlockingConcurrentQueue<I>::consumer_token_t tok(inputQueue);
                bool foundLast = true;
                while (this->keepPulling_ || (!this->keepPulling_ && foundLast)) {
                    I input;
                    if (this->keepPulling_) {
                        if (inputQueue.wait_dequeue_timed(tok, input, std::chrono::milliseconds(1)))
                            wrkFct(input);
                    } else {
                        foundLast = inputQueue.try_dequeue(tok, input);
                        if (foundLast) wrkFct(input);
                    }
                }
            }));
        }
    }

    ~OneWayPool()
    {
        this->keepPulling_ = false;
        for (auto& t : this->threads_)
            t.join();
    }

    void Add(I input)
    {
        while (!inputQueue.try_enqueue(std::move(input))) {
            std::this_thread::sleep_for(std::chrono::nanoseconds(5));
        }
    }

private:
    moodycamel::BlockingConcurrentQueue<I> inputQueue;
    std::vector<std::thread> threads_;

    volatile bool keepPulling_{true};
};
}  // namespace XLR