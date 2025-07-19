#ifndef BLOCKING_QUEUE_H
#define BLOCKING_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>

namespace tektask::queue
{
    /**
     * @class BlockingQueue
     * @brief Thread-safe blocking queue with support for graceful shutdown.
     *
     * Provides a classical FIFO queue with blocking semantics.
     *
     * waitPush() blocks if needed while acquiring the lock;
     * waitPop() blocks until data is available or shutdown is triggered.
     *
     * @tparam T Type of the elements stored in the queue.
     */
    template <typename T>
    class BlockingQueue
    {
    public:
        BlockingQueue() = default;
        ~BlockingQueue() = default;
        BlockingQueue(const BlockingQueue&) = delete;
        BlockingQueue& operator=(const BlockingQueue&) = delete;

        BlockingQueue(BlockingQueue&& other) noexcept : m_queue(std::move(other.m_queue)),
                                                        m_mutex(std::move(other.m_mutex)), m_cv(std::move(other.m_cv)),
                                                        m_stopped(std::move(other.m_stopped))
        {
        }

        BlockingQueue& operator=(BlockingQueue&& other)
        {
            if (this == &other)
            {
                return *this;
            }
            m_queue = std::move(other.m_queue);
            m_mutex = std::move(other.m_mutex);
            m_cv = std::move(other.m_cv);
            m_stopped = std::move(other.m_stopped);
            return *this;
        }

        using value_type = T;

        /**
         * @brief Pushes a copy of the item into the queue.
         *
         * Notifies one waiting consumer thread.
         *
         * @param item The item to be pushed.
         */
        void waitPush(const T& item)
        {
            {
                std::lock_guard lock(m_mutex);
                m_queue.push(item);
            }
            m_cv.notify_one();
        }

        /**
         * @brief Pushes a moved item into the queue.
         *
         * Notifies one waiting consumer thread.
         *
         * @param item The item to be moved.
         */
        void waitPush(T&& item)
        {
            {
                std::lock_guard lock(m_mutex);
                m_queue.push(std::move(item));
            }
            m_cv.notify_one();
        }

        /**
         * @brief Pops an item from the queue.
         *
         * Waits until an item is available or shutdown was triggered.
         *
         * @param out Reference to store the dequeued item.
         * @return true if an item was popped, false if shutdown and the queue was empty.
         */
        bool waitPop(T& out)
        {
            std::unique_lock lock(m_mutex);
            m_cv.wait(lock, [&]
            {
                return !m_queue.empty() || m_stopped;
            });

            if (m_stopped && m_queue.empty())
            {
                return false;
            }

            out = std::move(m_queue.front());
            m_queue.pop();
            return true;
        }

        /**
         * @brief Signals all waiting threads to stop.
         *
         * After calling shutdown, all waiting waitPop() calls will return false.
         */
        void shutdown()
        {
            {
                std::lock_guard lock(m_mutex);
                m_stopped = true;
            }
            m_cv.notify_all();
        }

    private:
        std::queue<T> m_queue;
        std::mutex m_mutex;
        std::condition_variable m_cv;
        std::atomic_bool m_stopped{false};
    };
}

#endif //BLOCKING_QUEUE_H
