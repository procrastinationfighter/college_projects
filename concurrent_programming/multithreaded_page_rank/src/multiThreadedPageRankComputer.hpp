#ifndef SRC_MULTITHREADEDPAGERANKCOMPUTER_HPP_
#define SRC_MULTITHREADEDPAGERANKCOMPUTER_HPP_

#include <atomic>
#include <mutex>
#include <thread>
#include <condition_variable>

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "immutable/network.hpp"
#include "immutable/pageIdAndRank.hpp"
#include "immutable/pageRankComputer.hpp"

class MultiThreadedPageRankComputer : public PageRankComputer {
private:
    using PageMap = std::unordered_map<PageId, PageRank, PageIdHash>;
    uint32_t numThreads;

    // Those can be mutable, because their existence is related only to computeForNetwork.
    mutable std::mutex mutexComputing;
    mutable std::condition_variable conditionMainThread;
    mutable std::condition_variable conditionSideThread;
    mutable std::condition_variable conditionDontSkip;

    mutable int currIteration;
    mutable uint32_t workingThreads;
    mutable int stoppedIteration;
    mutable bool isEnd;
    mutable bool hasStarted;
    mutable bool startWorking;

    mutable double currDifference;
    mutable double oldDangleSum;
    mutable double newDangleSum;

    mutable PageMap pageHashMap;
    mutable PageMap previousPageHashMap;

    mutable std::unordered_map<PageId, uint32_t, PageIdHash> numLinks;
    mutable std::unordered_map<PageId, std::vector<PageId>, PageIdHash> edges;

    void joinThreads(std::vector<std::thread> &threads) const
    {
        for (auto &thread : threads) {
            thread.join();
        }
    }

    void initializeMyPart(const Network &network, int threadNo) const
    {
        int mod = network.getSize() % numThreads;
        int div = network.getSize() / numThreads;
        auto first = network.getPages().begin();
        std::advance(first, threadNo * div + std::min(mod, threadNo));
        auto second = first;
        std::advance(second, div + (threadNo < mod));

        if (first == network.getPages().end()) {
            return;
        }

        while (first != second) {
            first->generateId(network.getGenerator());
            first = std::next(first);
        }
    }

    void initializeContainers(Network const& network) const
    {
        pageHashMap = std::unordered_map<PageId, PageRank, PageIdHash>();
        numLinks = std::unordered_map<PageId, uint32_t, PageIdHash>();
        edges = std::unordered_map<PageId, std::vector<PageId>, PageIdHash>();

        std::vector<std::thread> threads;
        threads.reserve(numThreads);
        for (uint32_t i = 0; i < numThreads; i++) {
            threads.emplace_back([this, &network, i]() {this->initializeMyPart(network, i);});
        }

        joinThreads(threads);

        size_t dangleCount = 0;
        for (auto const& page : network.getPages()) {
            pageHashMap[page.getId()] = 1.0 / network.getSize();
            numLinks[page.getId()] = page.getLinks().size();

            if (page.getLinks().size() == 0) {
                dangleCount++;
            }

            for (auto &link : page.getLinks()) {
                edges[link].push_back(page.getId());
            }
        }

        workingThreads = numThreads;
        startWorking = isEnd = hasStarted = false;
        currIteration = -1;
        stoppedIteration = 0;
        newDangleSum = (1.0 / network.getSize()) * dangleCount;
    }

    void waitForWork(std::unique_lock<std::mutex> &myLock, int iterationNumber) const
    {
        if (stoppedIteration < iterationNumber) {
            conditionDontSkip.wait(myLock);
        }
        workingThreads--;

        if (workingThreads == 0) {
            conditionMainThread.notify_all();
        }

        while (iterationNumber == currIteration) {
            conditionSideThread.wait(myLock);
        }
        startWorking = false;
        workingThreads++;
        if (workingThreads == numThreads) {
            stoppedIteration++;
            conditionDontSkip.notify_all();
        }
    }

    std::pair<PageMap::iterator, PageMap::iterator> getIteratorBounds(int threadNo) const {
        int mod = pageHashMap.size() % numThreads;
        int div = pageHashMap.size() / numThreads;
        PageMap::iterator first = pageHashMap.begin();
        std::advance(first, threadNo * div + std::min(mod, threadNo));
        PageMap::iterator second = first;
        std::advance(second, div + (threadNo < mod));
        return {first, second};
    }

    void computeRanks(double alpha, size_t networkSize, int threadNo) const
    {
        double danglingWeight = 1.0 / networkSize;
        int myIteration = 0;
        auto bounds = getIteratorBounds(threadNo);
        std::unique_lock<std::mutex> myLock(mutexComputing);

        while (!hasStarted) {
            conditionSideThread.wait(myLock);
        }

        while (!isEnd) {
            myLock.unlock();

            // Process work (the most important part).
            double diff = 0.0, myDangleSum = 0.0;
            PageMap::iterator it = bounds.first;
            while (it != bounds.second && it != pageHashMap.end()) {
                PageId pageId = it->first;

                double bonus = oldDangleSum * danglingWeight + (1.0 - alpha) / networkSize;

                if (edges.count(pageId) > 0) {
                    for (auto link : edges[pageId]) {
                        bonus += alpha * previousPageHashMap[link] / numLinks[link];
                    }
                }
                diff += std::abs(previousPageHashMap[pageId] - bonus);

                it->second = bonus;
                if (numLinks[pageId] == 0) {
                    myDangleSum += bonus;
                }

                it = std::next(it);
            }

            // Finish work.
            myLock.lock();
            currDifference += diff;
            newDangleSum += myDangleSum;
            // Take your next work or wait.
            waitForWork(myLock, myIteration);
            myIteration++;
        }
    }

    void resetDangleSum(double alpha) const
    {
        oldDangleSum = newDangleSum * alpha;
        newDangleSum = 0.0;
    }

public:
    MultiThreadedPageRankComputer(uint32_t numThreadsArg)
        : numThreads(numThreadsArg) {};

    std::vector<PageIdAndRank> computeForNetwork(Network const& network, double alpha, uint32_t iterations, double tolerance) const
    {
        initializeContainers(network);

        std::vector<std::thread> threads;
        threads.reserve(numThreads);
        for (uint32_t i = 0; i < numThreads; i++) {
            threads.emplace_back([this, alpha, network, i]() {this->computeRanks(alpha, network.getSize(), i);});
        }

        std::unique_lock<std::mutex> myLock(mutexComputing);

        for (uint32_t i = 0; i < iterations; i++) {
            // Reinitialize variables.
            previousPageHashMap = pageHashMap;
            resetDangleSum(alpha);
            currDifference = 0.0;

            // Wake up sleeping threads and let them work.
            hasStarted = true;
            currIteration++;
            conditionSideThread.notify_all();
            while (workingThreads > 0 || startWorking) {
                conditionMainThread.wait(myLock);
            }
            startWorking = true;

            // If given tolerance was reached, create result vector and free all threads.
            if (currDifference < tolerance) {
                isEnd = true;
                currIteration++;
                conditionSideThread.notify_all();
                myLock.unlock();

                std::vector<PageIdAndRank> result;
                for (auto iter : pageHashMap) {
                    result.push_back(PageIdAndRank(iter.first, iter.second));
                }
                ASSERT(result.size() == network.getSize(), "Invalid result size=" << result.size() << ", for network" << network);

                joinThreads(threads);
                return result;
            }
        }

        ASSERT(false, "Not able to find result in iterations=" << iterations);
    }

    std::string getName() const
    {
        return "MultiThreadedPageRankComputer[" + std::to_string(this->numThreads) + "]";
    }
};

#endif /* SRC_MULTITHREADEDPAGERANKCOMPUTER_HPP_ */
