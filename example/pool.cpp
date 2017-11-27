#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <FastPool.h>

class Timer
{
public:
    Timer() { Restart(); }

    float ElapsedMilliseconds() const
    {
        auto tock = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(tock - tick).count();
    }
    float ElapsedSeconds() const { return ElapsedMilliseconds() / 1000; }
    std::string ElapsedTime() const
    {
        auto tock = std::chrono::steady_clock::now();
        auto t = std::chrono::duration_cast<std::chrono::nanoseconds>(tock - tick).count();

        auto d = t / 10000 / 1000 / 1000 / 60 / 60 / 24;
        auto h = (t / 1000 / 1000 / 1000 / 60 / 60) % 24;
        auto m = (t / 1000 / 1000 / 1000 / 60) % 60;
        auto s = (t / 1000 / 1000 / 1000) % 60;
        auto ms = (t / 1000 / 1000) % 1000;
        auto us = (t / 1000) % 1000;
        auto ns = t % 1000;
        std::stringstream ss;
        if (d > 0) ss << d << "d ";
        if (h > 0) ss << h << "h ";
        if (m > 0) ss << m << "m ";
        if (s > 0) ss << s << "s ";
        if (ms > 0) ss << ms << "ms ";
        if (us > 0) ss << us << "us ";
        if (ns > 0) ss << ns << "ns ";
        return ss.str();
    }
    void Restart() { tick = std::chrono::steady_clock::now(); }

private:
    std::chrono::time_point<std::chrono::steady_clock> tick;
};

void IntPool()
{
    enum class Type : int
    {
        SORTED = 0,
        UNSORTED,
        STEAL_UNSORTED
    };

    auto RunWithPool = [](Type t) {
        auto Worker = [](const int& i) { return i + 1; };
        std::ofstream out("int-out-" + std::to_string(static_cast<int>(t)));
        auto Consumer = [&out](const int& i) { out << i << "\n"; };

        std::unique_ptr<XLR::FastPoolSPMC<int, int>> fp;
        switch (t) {
            case Type::SORTED:
                fp = std::make_unique<XLR::FastPoolSPMCSO<int, int>>(8, Worker, Consumer);
                break;
            case Type::UNSORTED:
                fp = std::make_unique<XLR::FastPoolSPMCUS<int, int>>(8, Worker, Consumer);
                break;
            case Type::STEAL_UNSORTED:
                fp = std::make_unique<XLR::FastPoolStealSPMCUS<int, int>>(8, Worker, Consumer);
                break;
            default:
                break;
        }
        for (int i = 0; i < 1000000; ++i)
            fp->Add(i);
    };
    Timer t;
    RunWithPool(Type::SORTED);
    std::cerr << "Sorted         : " << t.ElapsedTime() << std::endl;
    t.Restart();
    RunWithPool(Type::UNSORTED);
    std::cerr << "Unsorted       : " << t.ElapsedTime() << std::endl;
    t.Restart();
    RunWithPool(Type::STEAL_UNSORTED);
    std::cerr << "Unsorted Steal : " << t.ElapsedTime() << std::endl;
}

void UniqueStringPool()
{
    enum class Type : int
    {
        SORTED = 0,
        UNSORTED,
        STEAL_UNSORTED
    };
    using IOType = std::unique_ptr<std::string>;
    // using IOType = std::string;

    auto RunWithPool = [](Type t) {
        auto Worker = [](const IOType& i) {
            return std::make_unique<std::string>(*i);
            // return i;
        };
        std::ofstream out("string-out-" + std::to_string(static_cast<int>(t)));
        const auto Consumer = [&out](const IOType& i) -> void { out << *i << "\n"; };
        // const auto Consumer = [&out](const IOType& i) -> void { out << i << "\n"; };

        std::unique_ptr<XLR::FastPoolSPMC<IOType, IOType>> fp;
        switch (t) {
            case Type::SORTED:
                fp = std::make_unique<XLR::FastPoolSPMCSO<IOType, IOType>>(8, Worker, Consumer);
                break;
            case Type::UNSORTED:
                fp = std::make_unique<XLR::FastPoolSPMCUS<IOType, IOType>>(8, Worker, Consumer);
                break;
            case Type::STEAL_UNSORTED:
                fp =
                    std::make_unique<XLR::FastPoolStealSPMCUS<IOType, IOType>>(8, Worker, Consumer);
                break;
            default:
                break;
        }
        for (int i = 0; i < 1000000; ++i) {
            auto a = std::make_unique<std::string>("test");
            // std::string a = "test";
            fp->Add(std::move(a));
        }
    };
    Timer t;
    RunWithPool(Type::SORTED);
    std::cerr << "Sorted         : " << t.ElapsedTime() << std::endl;
    t.Restart();
    RunWithPool(Type::UNSORTED);
    std::cerr << "Unsorted       : " << t.ElapsedTime() << std::endl;
    t.Restart();
    RunWithPool(Type::STEAL_UNSORTED);
    std::cerr << "Unsorted Steal : " << t.ElapsedTime() << std::endl;
}

int main(int argc, char* argv[])
{
    IntPool();
    UniqueStringPool();
}