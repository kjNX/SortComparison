#include <array>
#include <random>
// #include "boost/asio.hpp"

#include <algorithm>
#include <bits/ranges_algo.h>

#include "Commons.hpp"

// namespace asio = boost::asio;

std::array<int, Commons::DECK_SIZE> makeDeck()
{
    std::random_device rd{};
    std::seed_seq seq{rd(), rd(), rd(), rd()};
    std::ranlux24 dev{seq};
    std::uniform_int_distribution<> dist{0, Commons::max_cards - 1};
    std::array<int, Commons::DECK_SIZE> deck{};
    for(int& i : deck) i = dist(dev);
    return deck;
}

void quickSort(std::array<int, Commons::DECK_SIZE>& arr, const int& left, int right)
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

void mergeSort(std::array<int, Commons::DECK_SIZE>& arr, const int& left, const int& right)
{
    if(left < right)
    {
        int mid{(right + left) / 2};
        mergeSort(arr, left, mid);
        mergeSort(arr, mid + 1, right);
        merge(arr, left, mid, right);
    }
}

int main() {
    std::array<int, Commons::DECK_SIZE> deck{makeDeck()};

    // asio::thread_pool thread_pool(std::thread::hardware_concurrency());
    // asio::post(thread_pool, [&]{ quickSort(deck, 0, Commons::DECK_SIZE - 1, thread_pool); });
    // thread_pool.join();

    quickSort(deck, 0, Commons::DECK_SIZE - 1);
    return 0;
}