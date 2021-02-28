#ifndef SRC_SINGLETHREADEDPAGERANKCOMPUTER_HPP_
#define SRC_SINGLETHREADEDPAGERANKCOMPUTER_HPP_

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "immutable/network.hpp"
#include "immutable/pageIdAndRank.hpp"
#include "immutable/pageRankComputer.hpp"

class SingleThreadedPageRankComputer : public PageRankComputer {
public:
    SingleThreadedPageRankComputer() {};

    std::vector<PageIdAndRank> computeForNetwork(Network const& network, double alpha, uint32_t iterations, double tolerance) const
    {
        std::unordered_map<PageId, PageRank, PageIdHash> pageHashMap;
        std::unordered_map<PageId, uint32_t, PageIdHash> numLinks;
        std::unordered_map<PageId, std::vector<PageId>, PageIdHash> edges;
        size_t dangleCount = 0;
        for (auto const& page : network.getPages()) {
            page.generateId(network.getGenerator());
        }

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


        double difference, danglingWeight = 1.0 / network.getSize();

        double oldDangleSum, newDangleSum = danglingWeight * dangleCount;

        for (uint32_t i = 0; i < iterations; ++i) {
            std::unordered_map<PageId, PageRank, PageIdHash> previousPageHashMap = pageHashMap;

            oldDangleSum = newDangleSum * alpha;
            newDangleSum = 0.0;

            difference = 0;
            for (auto& pageMapElem : pageHashMap) {
                PageId pageId = pageMapElem.first;

                double bonus = oldDangleSum * danglingWeight + (1.0 - alpha) / network.getSize();

                if (edges.count(pageId) > 0) {
                    for (auto link : edges[pageId]) {
                        bonus += alpha * previousPageHashMap[link] / numLinks[link];
                    }
                }

                pageMapElem.second = bonus;

                if (numLinks[pageId] == 0) {
                    newDangleSum += bonus;
                }

                difference += std::abs(previousPageHashMap[pageId] - bonus);
            }

            if (difference < tolerance) {
                std::vector<PageIdAndRank> result;
                for (auto iter : pageHashMap) {
                    result.push_back(PageIdAndRank(iter.first, iter.second));
                }

                ASSERT(result.size() == network.getSize(), "Invalid result size=" << result.size() << ", for network" << network);

                return result;
            }
        }

        ASSERT(false, "Not able to find result in iterations=" << iterations);
    }

    std::string getName() const
    {
        return "SingleThreadedPageRankComputer";
    }
};

#endif /* SRC_SINGLETHREADEDPAGERANKCOMPUTER_HPP_ */
