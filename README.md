Yet another threadpool, based on a [lock-free concurrent queue](https://github.com/cameron314/concurrentqueue).

In general, it is a single-producer, multi-consumer pool. Call `Add()` only from
a single thread.

Example:

    #include <FastPool.h>
    #include <fstream>

    int main(int argc, char* argv[])
    {
        auto Worker = [](const int& i) { return i + 1; };
        std::ofstream out("out");
        auto Consumer = [&out](const int& i) { out << i << "\n"; };

        // Keep order (sorted)
        // XLR::FastPoolSPMCSO<int, int> tp(8, Worker, Consumer);

        // Unsorted version
        // XLR::FastPoolSPMCUS<int, int> tp(8, Worker, Consumer);

        // Unsorted, stealing version
        XLR::FastPoolStealSPMCUS<int, int> tp(8, Worker, Consumer);

        for (int i = 0; i < 1000000; ++i)
            tp.Add(i);
    }