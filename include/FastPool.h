// Author: Armin Töpfer

#pragma once

#include <set>
#include <thread>
#include <vector>

#include <blockingconcurrentqueue.h>

namespace XLR {

// I is the input type
// O is the output type
template <typename I, typename O>
class FastPoolSPMC
{
public:
    virtual void Add(I input) = 0;
    virtual ~FastPoolSPMC() = default;

protected:
    std::vector<std::thread> threads_;
    std::thread outputThread_;

    volatile bool keepPulling_{true};
    volatile bool keepPoppping_{true};
};

template <typename I, typename O>
class FastPoolSPMCUS : public FastPoolSPMC<I, O>
{
public:
    FastPoolSPMCUS(const size_t numThreads, const std::function<O(I&)> wrkFct,
                   const std::function<void(const O&)> cnsFct, const size_t inputQUpperBound = 100)
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
                            outputQueue.enqueue(std::move(wrkFct(input)));
                    } else {
                        foundLast = inputQueue.try_dequeue(tok, input);
                        if (foundLast) outputQueue.enqueue(std::move(wrkFct(input)));
                    }
                }
            }));
        }
        this->outputThread_ = std::thread([this, cnsFct]() {
            size_t currentIndex = 0;
            bool foundLast = true;
            while (this->keepPoppping_ || (!this->keepPoppping_ && foundLast)) {
                O output;
                if (this->keepPoppping_) {
                    if (outputQueue.wait_dequeue_timed(output, std::chrono::milliseconds(1))) {
                        cnsFct(output);
                        ++currentIndex;
                    }
                } else {
                    foundLast = outputQueue.try_dequeue(output);
                    if (foundLast) {
                        cnsFct(output);
                        ++currentIndex;
                    }
                }
            }
        });
    }

    ~FastPoolSPMCUS()
    {
        this->keepPulling_ = false;
        for (auto& t : this->threads_)
            t.join();
        this->keepPoppping_ = false;
        this->outputThread_.join();
    }

    void Add(I input) override
    {
        while (!inputQueue.try_enqueue(std::move(input))) {
            std::this_thread::sleep_for(std::chrono::nanoseconds(5));
        }
    }

private:
    moodycamel::BlockingConcurrentQueue<I> inputQueue;
    moodycamel::BlockingConcurrentQueue<O> outputQueue;
};

template <typename I, typename O>
class FastPoolSPMCSO : public FastPoolSPMC<I, O>
{
private:
    struct Wrapper
    {
        size_t Index;
        I Input;
        O Output;

        Wrapper() {}
        Wrapper(const size_t index, I input) : Index(index), Input(std::move(input)) {}
        // Move constructor
        Wrapper(Wrapper&&) = default;
        // Copy constructor
        Wrapper(const Wrapper&) = delete;
        // Move assignment operator
        Wrapper& operator=(Wrapper&&) = default;
        // Copy assignment operator
        Wrapper& operator=(const Wrapper&) = delete;

        bool operator<(const Wrapper& rhs) const { return Index < rhs.Index; }
    };

public:
    FastPoolSPMCSO(const size_t numThreads, const std::function<O(I&)> wrkFct,
                   const std::function<void(const O&)> cnsFct, const size_t inputQUpperBound = 100)
        : inputQueue({inputQUpperBound, numThreads, numThreads})
    {
        for (size_t i = 0; i < numThreads - 1; ++i) {
            this->threads_.emplace_back(std::thread([this, wrkFct]() {
                typename moodycamel::BlockingConcurrentQueue<I>::consumer_token_t tok(inputQueue);
                bool foundLast = true;
                while (this->keepPulling_ || (!this->keepPulling_ && foundLast)) {
                    Wrapper wrapper;
                    if (this->keepPulling_) {
                        if (inputQueue.wait_dequeue_timed(tok, wrapper,
                                                          std::chrono::milliseconds(1))) {
                            wrapper.Output = wrkFct(wrapper.Input);
                            outputQueue.enqueue(std::move(wrapper));
                        }
                    } else {
                        foundLast = inputQueue.try_dequeue(tok, wrapper);
                        if (foundLast) {
                            wrapper.Output = wrkFct(wrapper.Input);
                            outputQueue.enqueue(std::move(wrapper));
                        }
                    }
                }
            }));
        }
        this->outputThread_ = std::thread([this, cnsFct]() {
            std::set<Wrapper> sortedOutput;
            size_t currentIndex = 0;
            auto CheckQueue = [&](auto&& item) {
                if (currentIndex == item.Index) {
                    cnsFct(item.Output);
                    ++currentIndex;
                    for (auto it = sortedOutput.begin(); it != sortedOutput.end();) {
                        if (it->Index == currentIndex) {
                            cnsFct(it->Output);
                            it = sortedOutput.erase(it);
                            ++currentIndex;
                        } else {
                            break;
                        }
                    }
                } else {
                    sortedOutput.emplace(std::forward<Wrapper>(item));
                }
            };
            bool foundLast = true;
            while (this->keepPoppping_ || (!this->keepPoppping_ && foundLast)) {
                Wrapper wrapper;
                if (this->keepPoppping_) {
                    if (outputQueue.wait_dequeue_timed(wrapper, std::chrono::milliseconds(1)))
                        CheckQueue(wrapper);
                } else {
                    foundLast = outputQueue.try_dequeue(wrapper);
                    if (foundLast) CheckQueue(wrapper);
                }
            }
        });
    }

    ~FastPoolSPMCSO()
    {
        this->keepPulling_ = false;
        for (auto& t : this->threads_)
            t.join();
        this->keepPoppping_ = false;
        this->outputThread_.join();
    }

    void Add(I input) override
    {
        typename moodycamel::BlockingConcurrentQueue<I>::producer_token_t ptok(inputQueue);
        Wrapper w(Index, std::move(input));
        ++Index;
        while (!inputQueue.try_enqueue(ptok, std::move(w))) {
            std::this_thread::sleep_for(std::chrono::nanoseconds(5));
        }
    }

private:
    size_t Index = 0;
    moodycamel::BlockingConcurrentQueue<Wrapper> inputQueue;
    moodycamel::BlockingConcurrentQueue<Wrapper> outputQueue;
};

template <typename I, typename O>
class FastPoolStealSPMCUS : public FastPoolSPMC<I, O>
{
public:
    FastPoolStealSPMCUS(const size_t numThreads, const std::function<O(I&)> wrkFct,
                        const std::function<void(const O&)> cnsFct,
                        const size_t inputQUpperBound = 100)
        : numThreads_(numThreads)
    {
        for (size_t i = 0; i < numThreads; ++i) {
            inputQueue.emplace_back(inputQUpperBound, numThreads, numThreads);
        }
        for (size_t i = 0; i < numThreads; ++i) {
            this->threads_.emplace_back(std::thread([this, i, wrkFct]() {
                const int myQueue = i;
                int roundRobin = i + 1;
                typename moodycamel::BlockingConcurrentQueue<I>::consumer_token_t tok(
                    inputQueue.at(myQueue));
                bool foundLast = true;
                while (this->keepPulling_ || (!this->keepPulling_ && foundLast)) {
                    I input;
                    if (this->keepPulling_) {
                        if (inputQueue.at(myQueue).wait_dequeue_timed(
                                input, std::chrono::microseconds(10))) {
                            outputQueue.enqueue(std::move(wrkFct(input)));
                        } else {
                            for (size_t c = roundRobin; c < roundRobin + numThreads_; ++c) {
                                if (inputQueue.at(roundRobin % numThreads_)
                                        .wait_dequeue_timed(input, std::chrono::microseconds(10))) {
                                    outputQueue.enqueue(std::move(wrkFct(input)));
                                    roundRobin = c;
                                    break;
                                }
                            }
                        }
                    } else {
                        foundLast = inputQueue.at(myQueue).try_dequeue(input);
                        if (foundLast) outputQueue.enqueue(std::move(wrkFct(input)));
                    }
                }
            }));
        }
        this->outputThread_ = std::thread([this, cnsFct]() {
            size_t currentIndex = 0;
            bool foundLast = true;
            while (this->keepPoppping_ || (!this->keepPoppping_ && foundLast)) {
                O output;
                if (this->keepPoppping_) {
                    if (outputQueue.wait_dequeue_timed(output, std::chrono::microseconds(10))) {
                        cnsFct(output);
                        ++currentIndex;
                    }
                } else {
                    foundLast = outputQueue.try_dequeue(output);
                    if (foundLast) {
                        cnsFct(output);
                        ++currentIndex;
                    }
                }
            }
        });
    }

    ~FastPoolStealSPMCUS()
    {
        this->keepPulling_ = false;
        for (auto& t : this->threads_)
            t.join();
        this->keepPoppping_ = false;
        this->outputThread_.join();
    }

    void Add(I input) override
    {
        if (roundRobin == numThreads_) roundRobin = 0;
        while (!inputQueue[roundRobin++].try_enqueue(std::move(input))) {
            if (roundRobin == numThreads_) roundRobin = 0;
            std::this_thread::sleep_for(std::chrono::nanoseconds(5));
        }
    }

private:
    const size_t numThreads_;
    size_t roundRobin = 0;
    size_t counter = 0;
    std::vector<moodycamel::BlockingConcurrentQueue<I>> inputQueue;
    moodycamel::BlockingConcurrentQueue<O> outputQueue;
};
}  // ::Fastpool