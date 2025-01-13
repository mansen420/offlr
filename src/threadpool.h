#pragma once

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace AiCo
{
    class threadpool
    {
        std::vector<std::thread> threads;
        std::mutex queueMutex;
        std::queue<std::function<void()>> jobQueue;
        std::condition_variable mutexCond;
        bool shouldTerminate = false;
    public:
        size_t count(){return threads.size();}
        threadpool() 
        {
            unsigned int count = std::thread::hardware_concurrency();
            threads.reserve(count);
            for(size_t i = 0; i < count; ++i)
                threads.emplace_back(std::thread(&threadpool::loop, this));
        }
        void enqueue_job(const std::function<void()>& job)
        {
            {
                std::unique_lock<std::mutex> lock (queueMutex);
                jobQueue.push(job);
            }
            mutexCond.notify_one();
        }
        bool busy(){std::unique_lock<std::mutex> lock (queueMutex); return !jobQueue.empty();}
        void stop(){
            {
                std::unique_lock<std::mutex> lock (queueMutex);
                shouldTerminate = true;
            }
            mutexCond.notify_all();
            for (std::thread& activeThread : threads)
                activeThread.join();
            threads.clear();
        }
        void loop()
        {
            while(true)
            {
                std::function<void()> job;
                {
                    std::unique_lock<std::mutex> lock (queueMutex);

                    mutexCond.wait(lock, [=, this](){return !jobQueue.empty() || shouldTerminate;});

                    if(shouldTerminate)
                        return;

                    job = jobQueue.front();
                    jobQueue.pop();
                }
                job();
            }
        }
        ~threadpool(){stop();}
    };
}
