#include <iostream>
#include <vector>
#include <unordered_map>
#include <deque>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <iomanip>
#include <chrono>
#include <thread>
#include <random>

// Constants
const int DEFAULT_DISK_SIZE = 32768;
const int DEFAULT_TLB_SIZE = 64;
const int DEFAULT_VM_SIZE = 65536;

enum CacheLevel {
    L1,
    L2,
    L3,
    RAM,
    DISK
};

enum ReplacementPolicy {
    FIFO,
    LRU,
    RANDOM
};

// Cache block structure
struct CacheBlock {
    int tag;
    bool valid;
    CacheBlock() : tag(-1), valid(false) {}
};

// Forward declarations
std::vector<int> generateSequentialAccess(int startAddress, int endAddress, int step);
std::vector<int> generateRandomAccess(int rangeStart, int rangeEnd, int count);
std::vector<int> generateLoopAccess(int startAddress, int endAddress, int loopCount);

// Function to generate memory addresses based on pattern choice
std::vector<int> generateAddresses(int patternChoice, int startAddress, int endAddress) {
    switch (patternChoice) {
    case 1:
        return generateSequentialAccess(startAddress, endAddress, 10);
    case 2:
        return generateRandomAccess(startAddress, endAddress, 20);
    case 3:
        return generateLoopAccess(startAddress, endAddress, 5);
    default:
        std::cerr << "Invalid pattern choice. Using sequential access by default.\n";
        return generateSequentialAccess(startAddress, endAddress, 10);
    }
}

// Function to generate sequential memory access pattern
std::vector<int> generateSequentialAccess(int startAddress, int endAddress, int step) {
    std::vector<int> addresses;
    for (int addr = startAddress; addr <= endAddress; addr += step) {
        addresses.push_back(addr);
    }
    return addresses;
}

// Function to generate random memory access pattern
std::vector<int> generateRandomAccess(int rangeStart, int rangeEnd, int count) {
    std::vector<int> addresses;
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    for (int i = 0; i < count; ++i) {
        int addr = rangeStart + std::rand() % (rangeEnd - rangeStart + 1);
        addresses.push_back(addr);
    }
    return addresses;
}

// Function to generate loop memory access pattern
std::vector<int> generateLoopAccess(int startAddress, int endAddress, int loopCount) {
    std::vector<int> addresses;
    for (int loop = 0; loop < loopCount; ++loop) {
        for (int addr = startAddress; addr <= endAddress; ++addr) {
            addresses.push_back(addr);
        }
    }
    return addresses;
}

// Cache class
class Cache {
private:
    int size;
    int blockSize;
    int numBlocks;
    ReplacementPolicy policy;
    int accessTime;
    std::vector<CacheBlock> blocks;
    std::deque<int> fifoQueue;
    std::unordered_map<int, int> lruMap;
    std::vector<int> randomIndices;

    int getIndex(int address) {
        return (address / blockSize) % numBlocks;
    }

public:
    int getAccessTime() { return accessTime; }
    Cache(int s = 0, int bs = 1, int at = 0, ReplacementPolicy rp = FIFO)
        : size(s), blockSize(bs), accessTime(at), policy(rp), numBlocks(s / bs) {

        blocks.resize(numBlocks);
        if (policy == RANDOM) {
            randomIndices.resize(numBlocks);
            for (int i = 0; i < randomIndices.size(); ++i) {
                randomIndices[i] = i;
            }
            std::srand(static_cast<unsigned int>(std::time(nullptr)));
        }
    }

    int access(int address) {
        int index = getIndex(address);
        int tag = address / blockSize;
        if (blocks[index].valid && blocks[index].tag == tag) {
            if (policy == LRU) {
                lruMap[tag] = static_cast<int>(std::time(nullptr));
            }
            return accessTime;  // Cache hit, return actual access time
        }
        replace(index, tag);
        return -1;  // Cache miss, return -1
    }

private:
    void replace(int index, int tag) {
        switch (policy) {
        case FIFO:
            if (fifoQueue.size() == numBlocks) {
                int oldIndex = fifoQueue.front();
                fifoQueue.pop_front();
                blocks[oldIndex].tag = tag;
                blocks[oldIndex].valid = true;
                fifoQueue.push_back(index);
            }
            else {
                blocks[index].tag = tag;
                blocks[index].valid = true;
                fifoQueue.push_back(index);
            }
            break;
        case LRU:
            if (lruMap.size() == numBlocks) {
                int lruTag = -1;
                int oldestTime = std::time(nullptr);
                for (auto it = lruMap.begin(); it != lruMap.end(); ++it) {
                    if (it->second < oldestTime) {
                        oldestTime = it->second;
                        lruTag = it->first;
                    }
                }
                lruMap.erase(lruTag);
                for (int i = 0; i < numBlocks; ++i) {
                    if (blocks[i].tag == lruTag) {
                        blocks[i].tag = tag;
                        blocks[i].valid = true;
                        lruMap[tag] = static_cast<int>(std::time(nullptr));
                        return;
                    }
                }
            }
            else {
                blocks[index].tag = tag;
                blocks[index].valid = true;
                lruMap[tag] = static_cast<int>(std::time(nullptr));
            }
            break;
        case RANDOM:
            std::shuffle(randomIndices.begin(), randomIndices.end(), std::default_random_engine(std::time(nullptr)));
            for (int i = 0; i < numBlocks; ++i) {
                if (!blocks[randomIndices[i]].valid) {
                    blocks[randomIndices[i]].tag = tag;
                    blocks[randomIndices[i]].valid = true;
                    return;
                }
            }
            int randomIndex = std::rand() % numBlocks;
            blocks[randomIndex].tag = tag;
            blocks[randomIndex].valid = true;
            break;
        }
    }
};

// TLB class
class TLB {
private:
    int size;
    int numBlocks;
    int accessTime;
    ReplacementPolicy policy;
    std::vector<CacheBlock> blocks;
    std::deque<int> fifoQueue;
    std::unordered_map<int, int> lruMap;
    std::vector<int> randomIndices;

    int getIndex(int page) {
        return page % numBlocks;
    }

public:
    TLB(int s = DEFAULT_TLB_SIZE, int at = 0, ReplacementPolicy rp = FIFO)
        : size(s), accessTime(at), policy(rp), numBlocks(s) {

        blocks.resize(numBlocks);
        if (policy == RANDOM) {
            randomIndices.resize(numBlocks);
            for (int i = 0; i < randomIndices.size(); ++i) {
                randomIndices[i] = i;
            }
            std::srand(static_cast<unsigned int>(std::time(nullptr)));
        }
    }

    int access(int page) {
        int index = getIndex(page);
        if (blocks[index].valid && blocks[index].tag == page) {
            if (policy == LRU) {
                lruMap[page] = static_cast<int>(std::time(nullptr));
            }
            return accessTime;  // TLB hit, return actual access time
        }
        replace(index, page);
        return -1;  // TLB miss, return -1
    }

    int getAccessTime() { return accessTime; }
    int getSize() { return size; }

private:
    void replace(int index, int page) {
        switch (policy) {
        case FIFO:
            if (fifoQueue.size() == numBlocks) {
                int oldIndex = fifoQueue.front();
                fifoQueue.pop_front();
                blocks[oldIndex].tag = page;
                blocks[oldIndex].valid = true;
                fifoQueue.push_back(index);
            }
            else {
                blocks[index].tag = page;
                blocks[index].valid = true;
                fifoQueue.push_back(index);
            }
            break;
        case LRU:
            if (lruMap.size() == numBlocks) {
                int lruPage = -1;
                int oldestTime = std::time(nullptr);
                for (auto it = lruMap.begin(); it != lruMap.end(); ++it) {
                    if (it->second < oldestTime) {
                        oldestTime = it->second;
                        lruPage = it->first;
                    }
                }
                lruMap.erase(lruPage);
                for (int i = 0; i < numBlocks; ++i) {
                    if (blocks[i].tag == lruPage) {
                        blocks[i].tag = page;
                        blocks[i].valid = true;
                        lruMap[page] = static_cast<int>(std::time(nullptr));
                        return;
                    }
                }
            }
            else {
                blocks[index].tag = page;
                blocks[index].valid = true;
                lruMap[page] = static_cast<int>(std::time(nullptr));
            }
            break;
        case RANDOM:
            std::shuffle(randomIndices.begin(), randomIndices.end(), std::default_random_engine(std::time(nullptr)));
            for (int i = 0; i < numBlocks; ++i) {
                if (!blocks[randomIndices[i]].valid) {
                    blocks[randomIndices[i]].tag = page;
                    blocks[randomIndices[i]].valid = true;
                    return;
                }
            }
            int randomIndex = std::rand() % numBlocks;
            blocks[randomIndex].tag = page;
            blocks[randomIndex].valid = true;
            break;
        }
    }
};

// Performance analyzer class
class PerformanceAnalyzer {
private:
    int totalAccesses;
    int hits;
    int misses;
    std::vector<int> cacheHits;
    std::vector<int> cacheMisses;

public:
    PerformanceAnalyzer(int numCaches) : totalAccesses(0), hits(0), misses(0) {
        cacheHits.resize(numCaches, 0);
        cacheMisses.resize(numCaches, 0);
    }

    void logAccess(bool hit, int cacheLevel) {
        totalAccesses++;
        if (hit) {
            hits++;
            if (cacheLevel < cacheHits.size()) {
                cacheHits[cacheLevel]++;
                for (int i = (cacheLevel + 1); i < cacheHits.size(); i++)
                    cacheHits[i]++;
            }
        }
        else {
            misses++;
            if (cacheLevel < cacheMisses.size()) {
                cacheMisses[cacheLevel]++;
            }
        }
    }

    void report() {
        std::cout << "\nPerformance Report:\n";
        std::cout << "Total Accesses: " << totalAccesses << "\n";
        std::cout << "Total Hits: " << hits << "\n";
        std::cout << "Total Misses: " << misses << "\n";
        std::cout << "Overall Hit Rate: " << std::fixed << std::setprecision(2)
            << static_cast<double>(hits) / totalAccesses * 100 << "%\n";
        std::cout << "Overall Miss Rate: " << std::fixed << std::setprecision(2)
            << static_cast<double>(misses) / totalAccesses * 100 << "%\n";

        for (size_t i = 0; i < 3; ++i) {
            std::cout << "L" << i + 1 << " Cache Hit Rate: " << std::fixed << std::setprecision(2)
                << static_cast<double>(cacheHits[i]) / totalAccesses * 100 << "%\n";
            double missrate = 1 - static_cast<double>(cacheHits[i]) / totalAccesses;
            std::cout << "L" << i + 1 << " Cache Miss Rate: " << std::fixed << std::setprecision(2)
                << missrate * 100 << "%\n";
        }
    }
};

// Memory hierarchy class
class MemoryHierarchy {
private:
    std::vector<Cache> caches;
    TLB tlb;
    Cache ram;
    int diskAccessTime;
    PerformanceAnalyzer analyzer;

public:
    MemoryHierarchy(const std::vector<int>& cacheSizes, const std::vector<int>& blockSizes,
        const std::vector<int>& accessTimes, const std::vector<ReplacementPolicy>& policies,
        int ramSize, int ramBlockSize, int ramAt, ReplacementPolicy ramPolicy,
        int diskSize, int diskAt, int tlbSize, int tlbAt, ReplacementPolicy tlbPolicy)
        : ram(ramSize, ramBlockSize, ramAt, ramPolicy), diskAccessTime(diskAt), analyzer(cacheSizes.size() + 1) {

        // Initialize caches
        for (size_t i = 0; i < cacheSizes.size(); ++i) {
            caches.emplace_back(cacheSizes[i], blockSizes[i], accessTimes[i], policies[i]);
        }

        // Initialize TLB
        tlb = TLB(tlbSize, tlbAt, tlbPolicy);
    }

    void simulateAccess(int address) {
        bool hit = false;
        int totalTime = 0;
        std::cout << "\n\nAddress: " << address << std::endl;
        std::cout << "Getting Physical address...\n";
        // Access TLB
        int page = address / tlb.getSize();
        int time = tlb.access(page);
        if (time != -1) {  // TLB hit
            totalTime += time;
            std::cout << "TLB Hit (Access time: " << totalTime << "ms)\n";
            analyzer.logAccess(true, 0);

            // Access caches
            for (size_t i = 0; i < caches.size(); ++i) {
                time = caches[i].access(address);
                if (time != -1) {  // If cache hit
                    totalTime += time;
                    std::cout << "Hit in L" << i + 1 << " Cache (Access time: " << totalTime << "ms)\n";
                    hit = true;
                    analyzer.logAccess(true, i + 1);
                    return; // Stop further accesses
                }
                else {
                    std::cout << "Miss in L" << i + 1 << " Cache\n";
                    totalTime += caches[i].getAccessTime();
                    analyzer.logAccess(false, i + 1);
                }
            }

            // If all caches miss, access RAM
            time = ram.access(address);
            if (time != -1) {
                totalTime += time;
                std::cout << "Hit in RAM (Access time: " << totalTime << "ms)\n";
                analyzer.logAccess(true, caches.size());
                return;
            }
            else {
                std::cout << "Miss in RAM\n";
                analyzer.logAccess(false, caches.size());
                totalTime += ram.getAccessTime();
            }

            // Access disk
            totalTime += diskAccessTime;
            std::cout << "Wait...\n";
            std::cout.flush();
            std::this_thread::sleep_for(std::chrono::milliseconds(diskAccessTime));
            std::cout << "Accessing Disk (Total access time: " << totalTime << "ms)\n";
            analyzer.logAccess(false, caches.size() + 1);

            // Assume Disk hit for this example
            std::cout << "Hit in Disk (Access time: " << totalTime << "ms)\n";
            analyzer.logAccess(true, caches.size() + 1);
        }
        else { // TLB miss, proceed to main memory
            totalTime += tlb.getAccessTime();
            std::cout << "TLB Miss, Accessing RAM to get Physical Address (Access time: " << totalTime << "ms)\n";
            analyzer.logAccess(false, 0);

            // Access RAM directly to get physical address
            time = ram.access(address);
            if (time != -1) {
                totalTime += time;
                std::cout << "Hit in RAM (Access time: " << totalTime << "ms)\n";
                analyzer.logAccess(true, caches.size());

                // Access caches
                for (size_t i = 0; i < caches.size(); ++i) {
                    time = caches[i].access(address);
                    if (time != -1) {  // If cache hit
                        totalTime += time;
                        std::cout << "Hit in L" << i + 1 << " Cache (Access time: " << totalTime << "ms)\n";
                        hit = true;
                        analyzer.logAccess(true, i + 1);
                        return; // Stop further accesses
                    }
                    else {
                        std::cout << "Miss in L" << i + 1 << " Cache\n";
                        analyzer.logAccess(false, i + 1);
                        totalTime += caches[i].getAccessTime();
                    }
                }

                // If all caches miss, access RAM again
                time = ram.access(address);
                if (time != -1) {
                    totalTime += time;
                    std::cout << "Hit in RAM (Access time: " << totalTime << "ms)\n";
                    analyzer.logAccess(true, caches.size());
                    return;
                }
                else {
                    std::cout << "Miss in RAM\n";
                    analyzer.logAccess(false, caches.size());
                    totalTime += ram.getAccessTime();
                }

                // Access disk
                totalTime += diskAccessTime;
                std::cout << "Wait...\n";
                std::cout.flush();
                std::this_thread::sleep_for(std::chrono::milliseconds(diskAccessTime));
                std::cout << "Accessing Disk (Total access time: " << totalTime << "ms)\n";
                analyzer.logAccess(false, caches.size() + 1);

                // Assume Disk hit for this example
                std::cout << "Hit in Disk (Access time: " << totalTime << "ms)\n";
                analyzer.logAccess(true, caches.size() + 1);
                return;

            }
            else {
                std::cout << "Miss in RAM\n";
                analyzer.logAccess(false, caches.size());
                totalTime += ram.getAccessTime();
                totalTime += diskAccessTime;
                std::cout << "Wait...\n";
                std::cout.flush();
                std::this_thread::sleep_for(std::chrono::milliseconds(diskAccessTime));
                std::cout << "Accessing Disk (Total access time: " << totalTime << "ms)\n";
                analyzer.logAccess(false, caches.size() + 1);
            }

            // Access caches
            for (size_t i = 0; i < caches.size(); ++i) {
                time = caches[i].access(address);
                if (time != -1) {  // If cache hit
                    totalTime += time;
                    std::cout << "Hit in L" << i + 1 << " Cache (Access time: " << totalTime << "ms)\n";
                    hit = true;
                    analyzer.logAccess(true, i + 1);
                    return; // Stop further accesses
                }
                else {
                    std::cout << "Miss in L" << i + 1 << " Cache\n";
                    analyzer.logAccess(false, i + 1);
                    totalTime += caches[i].getAccessTime();
                }
            }

            // If all caches miss, access RAM again
            time = ram.access(address);
            if (time != -1) {
                totalTime += time;
                std::cout << "Hit in RAM (Access time: " << totalTime << "ms)\n";
                analyzer.logAccess(true, caches.size());
                return;
            }
            else {
                std::cout << "Miss in RAM\n";
                analyzer.logAccess(false, caches.size());
                totalTime += ram.getAccessTime();
            }

            // Access disk
            totalTime += diskAccessTime;
            std::cout << "Wait...\n";
            std::cout.flush();
            std::this_thread::sleep_for(std::chrono::milliseconds(diskAccessTime));
            std::cout << "Accessing Disk (Total access time: " << totalTime << "ms)\n";
            analyzer.logAccess(false, caches.size() + 1);

            // Assume Disk hit for this example
            std::cout << "Hit in Disk (Access time: " << totalTime << "ms)\n";
            analyzer.logAccess(true, caches.size() + 1);
        }
    }

    void runSimulation(int patternChoice, int startAddress, int endAddress) {
        std::vector<int> addresses = generateAddresses(patternChoice, startAddress, endAddress);

        for (int address : addresses) {
            simulateAccess(address);
        }

        analyzer.report();
    }
};


// Function to get cache and block sizes from user
void getCacheConfiguration(std::vector<int>& cacheSizes, std::vector<int>& blockSizes, std::vector<int>& accessTimes, std::vector<ReplacementPolicy>& policies) {
    int numLayers;
    std::cout << "Enter the number of cache layers (1-3): ";
    std::cin >> numLayers;
    while (numLayers < 1 || numLayers > 3) {
        std::cout << "Invalid input. Enter the number of cache layers (1-3): ";
        std::cin >> numLayers;
    }

    cacheSizes.resize(numLayers);
    blockSizes.resize(numLayers);
    accessTimes.resize(numLayers);
    policies.resize(numLayers);

    for (int i = 0; i < numLayers; ++i) {
        std::cout << "Enter L" << i + 1 << " cache size: ";
        std::cin >> cacheSizes[i];
        std::cout << "Enter L" << i + 1 << " block size: ";
        std::cin >> blockSizes[i];
        std::cout << "Enter L" << i + 1 << " access time (in ms): ";
        std::cin >> accessTimes[i];

        int policy;
        std::cout << "Select L" << i + 1 << " replacement policy (0 - FIFO, 1 - LRU, 2 - Random): ";
        std::cin >> policy;
        while (policy < 0 || policy > 2) {
            std::cout << "Invalid input. Select replacement policy (0 - FIFO, 1 - LRU, 2 - Random): ";
            std::cin >> policy;
        }
        policies[i] = static_cast<ReplacementPolicy>(policy);
    }
}

// Function to get RAM configuration from user
void getRAMConfiguration(int& ramSize, int& ramBlockSize, int& ramAccessTime, int& ramPolicy) {
    std::cout << "Enter RAM size: ";
    std::cin >> ramSize;
    std::cout << "Enter RAM block size: ";
    std::cin >> ramBlockSize;
    std::cout << "Enter RAM access time (in ms): ";
    std::cin >> ramAccessTime;
    std::cout << "Select RAM replacement policy (0 - FIFO, 1 - LRU, 2 - Random): ";
    std::cin >> ramPolicy;
    while (ramPolicy < 0 || ramPolicy > 2) {
        std::cout << "Invalid input. Select RAM replacement policy (0 - FIFO, 1 - LRU, 2 - Random): ";
        std::cin >> ramPolicy;
    }
}

// Main function
int main() {
    std::vector<int> cacheSizes;
    std::vector<int> blockSizes;
    std::vector<int> accessTimes;
    std::vector<ReplacementPolicy> policies;
    int ramSize, ramBlockSize, ramAccessTime;
    int diskSize, diskAccessTime;
    int tlbSize, tlbAccessTime, tlbPolicy;
    int ramPolicy;
    int patternChoice;
    int startAddress, endAddress;

    // Loop to allow user to configure cache multiple times
    while (true) {
        // Get cache configuration from user
        getCacheConfiguration(cacheSizes, blockSizes, accessTimes, policies);

        // Get RAM configuration from user
        getRAMConfiguration(ramSize, ramBlockSize, ramAccessTime, ramPolicy);

        std::cout << "Enter Disk size: ";
        std::cin >> diskSize;
        std::cout << "Enter Disk access time (in ms): ";
        std::cin >> diskAccessTime;

        std::cout << "Enter TLB size (Note that TLB's block size is equal to cache level 1's block size): ";
        std::cin >> tlbSize;
        std::cout << "Enter TLB access time (in ms): ";
        std::cin >> tlbAccessTime;
        std::cout << "Select TLB replacement policy (0 - FIFO, 1 - LRU, 2 - Random): ";
        std::cin >> tlbPolicy;

        // Select memory access pattern
        std::cout << "\nSelect memory access pattern:\n";
        std::cout << "1. Sequential Access\n";
        std::cout << "2. Random Access\n";
        std::cout << "3. Loop Access\n";
        std::cout << "Enter your choice (1-3): ";
        std::cin >> patternChoice;

        std::cout << "Enter start address: ";
        std::cin >> startAddress;
        std::cout << "Enter end address: ";
        std::cin >> endAddress;

        // Create MemoryHierarchy instance and run simulation
        MemoryHierarchy mh(cacheSizes, blockSizes, accessTimes, policies,
            ramSize, ramBlockSize, ramAccessTime, static_cast<ReplacementPolicy>(ramPolicy),
            diskSize, diskAccessTime, tlbSize, tlbAccessTime, static_cast<ReplacementPolicy>(tlbPolicy));
        mh.runSimulation(patternChoice, startAddress, endAddress);

        // Option to continue or exit
        std::string choice;
        std::cout << "\nDo you want to configure another cache? (yes/no): ";
        std::cin >> choice;
        if (choice != "yes" && choice != "Yes") {
            break; // Exit the loop if user does not want to continue
        }
    }
    return 0;
}
