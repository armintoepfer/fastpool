#include <fstream>

#include <FastPool.h>

int main(int argc, char* argv[])
{
    auto Worker = [](int i) { return i + 1; };
    std::ofstream out("out");
    auto Consumer = [&out](int i) { out << i << "\n"; };

    // Keep order (sorted)
    XLR::FastPoolSPMCSO<int, int> tp(8, Worker, Consumer);

    // Unsorted version
    // XLR::FastPoolSPMCUS<int, int> tp(8, Worker, Consumer);

    for (int i = 0; i < 1000000; ++i)
        tp.Add(i);
}