/*
 * @Author Shi Zhangkun
 * @Date 2022-03-28 19:32:16
 * @LastEditTime 2022-03-29 01:45:24
 * @LastEditors Shi Zhangkun
 * @Description none
 * @FilePath /test/src/main.cpp
 */
#include <iostream>
#include <vector>
#include <chrono>

#include "ThreadPoolExecutor.hpp"
int main()
{
    
    ThreadPoolExecutor pool(4, 4);
    std::vector< std::future<int> > results;

    for(int i = 0; i < 8; ++i) {
        results.emplace_back(
            pool.excute([i] {
                std::cout << "hello " << i << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
                std::cout << "world " << i << std::endl;
                return i*i;
            })
        );
    }

    for(auto && result: results)
        std::cout << result.get() << ' ';
    std::cout << std::endl;
    return 0;
}