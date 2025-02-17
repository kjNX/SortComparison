#include <array>
#include <random>
#include <thread>
#include "boost/asio.hpp"

#include <algorithm>
#include <bits/ranges_algo.h>

#include "Commons.hpp"

namespace asio = boost::asio;

std::vector<int> makeDeck()
{
    std::random_device rd{};
    std::seed_seq seq{rd(), rd(), rd(), rd()};
    std::ranlux24 dev{seq};
    std::uniform_int_distribution<> dist{0, Commons::max_cards - 1};
    std::vector<int> deck(Commons::DECK_SIZE);
    for(int& i : deck) i = dist(dev);
    return deck;
}

void quickSort(std::vector<int>& arr, const int& left, int right)
{
    while(left < right)
    {
        int pivot{left};
        int idx{right};
        bool forward{false};
        while(pivot != idx)
        {
            if(arr[forward ? pivot : idx] < arr[forward ? idx : pivot])
            {
                std::swap(arr[pivot], arr[idx]);
                std::swap(pivot, idx);
                forward = !forward;
            }
            forward ? ++idx : --idx;
        }
        quickSort(arr, pivot + 1, right);
        // asio::post(pool, [&]{ quickSort(arr, pivot + 1, right, pool); });
        right = pivot;
    }
}

void merge(std::vector<int>& arr, const int& left, const int& mid, const int& right)
{
    const int& size1{mid - left + 1};
    const int& size2{right - mid};
    int arr1[size1];
    int arr2[size2];
    for(int i{0}; i < size1; ++i)
        arr1[i] = arr[left + i];
    for(int i{0}; i < size2; ++i)
        arr2[i] = arr[mid + 1 + i];

    int newPtr{left}, i{0}, j{0};
    while(i < size1 && j < size2)
    {
        const bool& greater{arr1[i] < arr2[j]};
        arr[newPtr++] = greater ? arr1[i] : arr2[j];
        greater ? ++i : ++j;
    }
    for(; i < size1; ++i) arr[newPtr++] = arr1[i];
    for(; j < size2; ++j) arr[newPtr++] = arr2[j];
}

void mergeSort(std::vector<int>& arr, const int& left, const int& right)
{
    if(left < right)
    {
        const int& mid{(right + left) / 2};
        // std::thread thread1{mergeSort, std::ref(arr), left, mid};
        mergeSort(arr, left, mid);
        mergeSort(arr, mid + 1, right);
        // thread.join();
        // asio::post(pool, [&]{ mergeSort(arr, mid + 1, right, pool); });
        merge(arr, left, mid, right);
    }
}

int main() {
    const unsigned& THREAD_COUNT{std::thread::hardware_concurrency()};
    unsigned threadCount{THREAD_COUNT};
    // unsigned threadCount{8u};
    unsigned mergeLength{Commons::DECK_SIZE / threadCount};
    std::vector<int> sequentialDeck{makeDeck()};
    std::vector<int> parallelDock{sequentialDeck};
    printf("Sequential sort:\n");
    auto startTime{std::chrono::steady_clock::now()};
    mergeSort(sequentialDeck, 0, Commons::DECK_SIZE - 1);
    auto endTime{std::chrono::steady_clock::now()};
    std::chrono::duration<double> sequentialTime{endTime - startTime};
    printf("Finished! Elapsed time: %f seconds.\n", sequentialTime.count());

    printf("Parallel merge-quick sort hybrid:\n");
    startTime = std::chrono::steady_clock::now();
    {
        std::unique_ptr<std::thread> threads[threadCount - 1u]{};
        for(unsigned i{0u}; i < threadCount - 1u; ++i)
            threads[i] = std::make_unique<std::thread>(&quickSort,
                std::ref(parallelDock), i * mergeLength, (i + 1u) * mergeLength - 1u);
        quickSort(parallelDock, (threadCount - 1u) * mergeLength, Commons::DECK_SIZE - 1u);
        for(const std::unique_ptr<std::thread>& i : threads) i -> join();
    }
    while(threadCount > 3)
    {
        threadCount >>= 1;
        mergeLength <<= 1;
        unsigned mid{mergeLength >> 1};
        // if(threadCount % 2)
        // {

        // }
        std::unique_ptr<std::thread> threads[threadCount - 1u]{};
        for(unsigned i{0u}; i < threadCount - 1u; ++i)
            threads[i] = std::make_unique<std::thread>(&merge,
                std::ref(parallelDock), i * mergeLength, i * mergeLength + mid - 1u, (i + 1u) * mergeLength - 1u);
        merge(parallelDock, (threadCount - 1u) * mergeLength, (threadCount - 1u) * mergeLength + mid - 1u, Commons::DECK_SIZE - 1u);
        for(const std::unique_ptr<std::thread>& i : threads) i -> join();
    }

    merge(parallelDock, 0, mergeLength - 1u, 2 * mergeLength - 1u);
    merge(parallelDock, 0, 2u * mergeLength - 1u, Commons::DECK_SIZE - 1u);

    endTime = std::chrono::steady_clock::now();
    std::chrono::duration<double> parallelTime{endTime - startTime};
    printf("Finished! Elapsed time: %f seconds.\n", parallelTime.count());
    float speedup{static_cast<float>(sequentialTime.count() / parallelTime.count())};
    float efficiency{speedup / THREAD_COUNT * 100};
    printf("Number of cores: %d\nSpeedup: %f\nEfficiency: %f%%", THREAD_COUNT, speedup, efficiency);
    return 0;
}