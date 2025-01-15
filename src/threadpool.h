#pragma once

#include <condition_variable>
#include <cstddef>
#include <cstdio>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace AiCo
{
    class threadpool
    {
        std::vector<std::thread> threads;
        std::mutex poolMutex;
        std::queue<std::function<void()>> jobQueue;
        std::condition_variable mutexCond;
        bool shouldTerminate = false;

        std::condition_variable idleCond;
        unsigned int activeThreads = 0;

        void loop()
        {
            while(true)
            {
                std::function<void()> job;

                if(empty())
                {
                    {
                        std::unique_lock<std::mutex> lock(poolMutex);
                        mutexCond.wait(lock, [this](){return !jobQueue.empty() || shouldTerminate;});
                        if (shouldTerminate)
                            return;   
                    }
                }
                {
                    std::unique_lock<std::mutex> lock(poolMutex);
                    job = jobQueue.front();
                    jobQueue.pop();
                    activeThreads++;
                }

                job();
                
                {
                    std::unique_lock<std::mutex> lock(poolMutex);
                    activeThreads--;
                }
                if(all_idle())
                    idleCond.notify_all();
            }
        }
        bool all_idle()
        {
            unsigned int threads, jobs;
            bool ret = false;
            {
                std::lock_guard<std::mutex> idleLock(poolMutex);
                threads = activeThreads; jobs = jobQueue.size();
                ret = threads == 0 && jobs == 0;
            }

            return ret;
        }
    public:
        size_t count(){return threads.size();}
        threadpool(unsigned int count = std::thread::hardware_concurrency()) 
        {
            threads.reserve(count);
            for(size_t i = 0; i < count; ++i)
                threads.emplace_back(std::thread(&threadpool::loop, this));
        }
        void enqueue_job(const std::function<void()>& job)
        {
            {
                std::unique_lock<std::mutex> lock (poolMutex);
                jobQueue.push(job);
            }
            mutexCond.notify_one();
        }
        bool empty(){std::unique_lock<std::mutex> lock (poolMutex); return jobQueue.empty();}
        
        //this WILL destroy the object!
        void stop(){
            {
                std::unique_lock<std::mutex> lock (poolMutex);
                shouldTerminate = true;
            }
            mutexCond.notify_all();
            for (std::thread& activeThread : threads)
                activeThread.join();
            threads.clear();
        }

        void wait_till_done()
        {

            if(all_idle())
                return;
            else
            {
                std::unique_lock<std::mutex> lock (poolMutex);
                idleCond.wait(lock, [this](){return activeThreads == 0 && jobQueue.empty();});
            }
        }

        ~threadpool(){stop();}
    };
}
