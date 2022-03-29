/*
 * @Author Shi Zhangkun
 * @Date 2022-03-28 21:52:21
 * @LastEditTime 2022-03-29 20:36:36
 * @LastEditors Shi Zhangkun
 * @Description none
 * @FilePath /test/ThreadPoolExecutor.hpp
 */
#include <thread>
#include <vector>
#include <list>
#include <string>
#include <queue>
#include <atomic>
#include <memory>
#include <condition_variable>
#include <functional>
#include <future>
#include <exception>

#if defined(_MSC_VER )
#define CPP_VERSION _MSVC_LANG
#elif defined(__clang__) || defined(__GNUC__)
#define CPP_VERSION __cplusplus 
#endif

class ThreadPoolExecutor {
private:
  long keepAliveTime;
  std::condition_variable conditionVar;
  std::mutex workerMut; // protect : workers, inactiveWorkers
  std::vector<std::thread> workers;
  std::queue<std::size_t> inactiveWorkers;
  std::mutex tasksMut; // protect : idleWorlers, tasks, running
  std::size_t idleWorkers;
  std::queue<std::function<void()>> tasks;
  bool running = true;

  bool __activeWoker(std::size_t index);
  ThreadPoolExecutor& operator=(const ThreadPoolExecutor&) = delete;
  ThreadPoolExecutor(const ThreadPoolExecutor&) = delete;
public:
  ThreadPoolExecutor(std::size_t maxPoolSize, std::size_t corePoolSize = std::string::npos, long keepAliveTime = -1);
  ~ThreadPoolExecutor();
  void shutdown();
  void shutdownNow();

#if CPP_VERSION >= 201703L
  template <class F, class... Args>
  auto excute(F&& f, Args&&... args)->std::future<typename std::invoke_result<F,Args...>::type>;
#else
  template <class F, class... Args>
  auto excute(F&& f, Args&&... args)->std::future<typename std::result_of<F(Args...)>::type>;
#endif
};

/**
 * @brief  active a worker (thread unsafe function)
 * @note  you must check the worker lock before call this function (workerMut)
 * @param {size_t} index
 * @retval none
 */
bool ThreadPoolExecutor::__activeWoker(std::size_t index)
{
  if (index >= workers.size()) return false;
   /* worker function */
  auto workerFunc = [this](std::size_t workerIndex){
    while (true)
    {
      std::function<void()> task;
      {
        using namespace std::chrono_literals;
        std::unique_lock<std::mutex> lock(tasksMut);
        idleWorkers++;
        auto pred = [this](){return (!running || !tasks.empty());};
        if (keepAliveTime > 0)
          conditionVar.wait_for(lock, keepAliveTime * 1ms, pred);
        else
          conditionVar.wait(lock, pred);
        idleWorkers--;
        if (tasks.empty() && (running == false || keepAliveTime > 0))
        {
          if (running != false)
          {
            std::unique_lock<std::mutex> lock(workerMut);
            inactiveWorkers.push(workerIndex);
          }
          return;
        }
        task = std::move(tasks.front());
        tasks.pop();
      }
      task();
    }
  };
  if (workers[index].joinable()) return false;
  workers[index] = std::thread(workerFunc, index);
  return true;
}

/**
 * @brief  
 * @note  
 * @param {size_t} maxPoolSize : max threads in this pool
 * @param {size_t} corePoolSize : initial pool size 
 * @param {long} keepAliveTime : unit (ms)
 * @retval none
 */
ThreadPoolExecutor::ThreadPoolExecutor(std::size_t maxPoolSize, std::size_t corePoolSize, long keepAliveTime)
:workers(maxPoolSize), keepAliveTime(keepAliveTime)
{
  idleWorkers = 0;
  if(corePoolSize > maxPoolSize) corePoolSize = maxPoolSize;
  for (int i = 0; i < maxPoolSize; i++)
  {
    std::unique_lock<std::mutex> lock(workerMut);
    if (i < corePoolSize)
      __activeWoker(i);
    else
      inactiveWorkers.push(i);
  }
}

/**
 * @brief  
 * @note  
 * @param {*}
 * @retval none
 */
ThreadPoolExecutor::~ThreadPoolExecutor()
{
  shutdownNow();
}

/**
 * @brief  
 * @note  
 * @param {*}
 * @retval none
 */
void ThreadPoolExecutor::shutdown()
{
  {
    std::unique_lock<std::mutex> lock(tasksMut);
    running = false;
  }
  conditionVar.notify_all();
  for (auto& worker : workers)
  {
    std::unique_lock<std::mutex> lock(workerMut);
    if (worker.joinable())
      worker.join();
  }
}

/**
 * @brief  
 * @note  
 * @param {*}
 * @retval none
 */
void ThreadPoolExecutor::shutdownNow()
{
  {
    std::unique_lock<std::mutex> lock(tasksMut);
    while(!tasks.empty()) tasks.pop();
    running = false;
  }
  conditionVar.notify_all();
  for (auto& worker : workers)
  {
    std::unique_lock<std::mutex> lock(workerMut);
    if (worker.joinable())
      worker.join();
  }
}


/**
 * @brief  
 * @note  
 * @param {F} &&f: Callable object
 * @param {Args} &&...args: input parameter
 * @retval std::future
 */
#if CPP_VERSION >= 201703L
template <class F, class... Args>
auto ThreadPoolExecutor::excute(F&& f, Args&&...args)
  ->std::future<typename std::invoke_result<F,Args...>::type>
#else
template <class F, class... Args>
auto ThreadPoolExecutor::excute(F&& f, Args&&...args)
  ->std::future<typename std::result_of<F(Args...)>::type>
#endif
{
#if CPP_VERSION >= 201703L
  using return_type = typename std::invoke_result<F,Args...>::type;
#else
  using return_type = typename std::result_of<F(Args...)>::type;
#endif
  auto task_pack = std::make_shared<std::packaged_task<return_type()>> (
                      std::bind(std::forward<F>(f), std::forward<Args>(args)...)
                  );
  {
    std::unique_lock<std::mutex> lock(tasksMut);
    if (!running) throw std::runtime_error("excute task on a stopped ThreadPool.");
    if (idleWorkers == 0) //if no idle worker, then add a worker. 
    {
      std::unique_lock<std::mutex> lock(workerMut);
      if (!inactiveWorkers.empty())
      {
        __activeWoker(inactiveWorkers.front());
        inactiveWorkers.pop();
      }
    }
    tasks.emplace([task_pack](){(*task_pack)();});
  }
  conditionVar.notify_one();
  return task_pack->get_future();
}
