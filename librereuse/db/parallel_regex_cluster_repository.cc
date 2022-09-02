//
// Created by charlie on 12/21/21.
//

#include "parallel_regex_cluster_repository.h"

#include "cluster.h"
#include <future>
#include <queue>
#include <shared_mutex>

/*----------HELPER CLASSES------------*/
/**
 * Concurrent queue for tasks to pull from
 */
class ConcurrentTaskQueue {
public:
    using Task = std::packaged_task<std::unordered_set<std::string>()>;

    explicit ConcurrentTaskQueue() = default;

    void push(const std::shared_ptr<rereuse::query::BaseClusterQuery> &query,
              const std::shared_ptr<rereuse::db::Cluster> &cluster) {
        Task task([query, cluster] { return query->query(cluster, nullptr); });
        this->push(std::move(task));
    }

    void push(Task &&task) {
        std::lock_guard lock(this->mutex); // Lock the resources
        this->tasks.push(std::move(task)); // Push the element
        // We are done, lock pops
    }

    Task pop() {
        std::lock_guard lock(this->mutex);
        // Take the front
        auto task = std::move(this->tasks.front());
        this->tasks.pop();
        return task;
    }

    bool empty() {
        std::lock_guard lock(this->mutex);
        return this->tasks.empty();
    }

private:
    std::queue<Task> tasks;
    std::shared_mutex mutex;
};

/**
 * Used for collecting a group of patterns across threads
 */
class PatternCollector {
public:
    PatternCollector() = default;

    void append_patterns(const std::unordered_set<std::string> &patterns) {
        std::lock_guard lock(this->mutex);
        std::copy(patterns.begin(),  patterns.end(), std::inserter(this->all_patterns, this->all_patterns.begin()));
    }

    const std::unordered_set<std::string> &patterns() const {
        return all_patterns;
    }

private:
    std::unordered_set<std::string> all_patterns;
    std::shared_mutex mutex;
};

/**
 * Class that performs tasks
 */
class TaskWorker {
public:
    TaskWorker(const std::shared_ptr<ConcurrentTaskQueue> &task_queue, std::shared_ptr<PatternCollector> &pattern_collector) {
        this->manager = std::thread([task_queue, pattern_collector] {
            // Repeat while there is work to do
            while (!task_queue->empty()) {
                // Get the next task to be computed
                auto task = std::move(task_queue->pop());
                // Get the tasks future result
                auto task_result = task.get_future();
                // Run the task
                task();
                // Wait on the result of the task
                task_result.wait(); // TODO probably want to time this out, although it should guaranteed finish
                // Get the result of the task
                auto resulting_patterns = task_result.get();
                // Add them all to the pattern collector
                pattern_collector->append_patterns(resulting_patterns);
            }
        });
    }

    void join() {
        if (this->manager.joinable()) {
            this->manager.join();
        }
    }

private:
    std::thread manager;
};

/*----------END HELPER CLASSES------------*/

rereuse::db::ParallelRegexClusterRepository::ParallelRegexClusterRepository(unsigned int processors)
: RegexClusterRepository()
, processors(processors) {
}

rereuse::db::ParallelRegexClusterRepository::ParallelRegexClusterRepository(unsigned int processors, int maxClusterSize, const std::string &path)
: RegexClusterRepository(maxClusterSize, path)
, processors(processors) {
}

std::unordered_set<std::string>
rereuse::db::ParallelRegexClusterRepository::query(const std::shared_ptr<rereuse::query::BaseClusterQuery> &query) const {
    // Create a queue of tasks for the thread pool to execute
    auto task_queue = std::make_shared<ConcurrentTaskQueue>();
    for (const auto &cluster : this->clusters) {
        task_queue->push(query, cluster);
    }

    // Make a pattern collector to collect everything
    auto pattern_collector = std::make_shared<PatternCollector>();

    // Make a collection of workers for as many processors as we have
    std::vector<TaskWorker> workers;
    workers.reserve(this->processors); // Reserve all the space we will need
    for (int i = 0; i < this->processors; i++) {
        // Start the task workers
        workers.emplace_back(task_queue, pattern_collector);
    }

    // Join all the workers in the worker pool
    for (auto &worker : workers) {
        worker.join();
    }

    // Return all the patterns
    return pattern_collector->patterns();
}
