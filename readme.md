# ThreadPoolExecutor
A thread pool for C++ (C++11\17\20), Support both dynamic model and static model.  
Protocol : https://github.com/progschj/ThreadPool
```cpp
/* -------- create as static model -------- */
//pool size: 8
ThreadPoolExecutor threadPool(8); 
/*-------- create as dynamic model -------- */
//max size: 8, initial size: 4, threads keepAlive 1000s
ThreadPoolExecutor threadPool(8, 4, 1000*1000); 

/* ----------- application ----------- */
auto res = threadPool.excute([](int a, int b){return a*b;}, 6, 7); 
auto result = res.get();
threadPool.shutdown(); //optional, wait for all tasks complete, or ~threadPool() will stop all tasks in waiting.
```
