### Тестовое по [вакансии](https://hh.ru/vacancy/110746454).

Синопсис [problem.txt](problem.txt). <br/>
Реализация в [DnsCacheImpl.hpp](net_utils/DnsCacheImpl.hpp). <br/>
Singleton facade для него в [DnsCache.hpp](net_utils/DnsCache.hpp). <br/>
Параметризация сборки в [CMakeLists.txt](CMakeLists.txt). <br/>

Кеш использует идиому [RCU](https://en.wikipedia.org/wiki/Read-copy-update), подразумевая частые обращения на чтение и редкие на изменение. <br/>
[**Lock-free**](https://en.wikipedia.org/wiki/Non-blocking_algorithm) реализация RCU в [RcuStorage.hpp](net_utils/RcuStorage.hpp).

Big O для `DnsCache`. 
* resolve - O(1)
* update - Amortized(1), O(cache_size * log cache_bound)

Результат бенчмарка: [bench_without_spinlock.txt](bench_results/bench_without_spinlock.txt).
(*сравнить с shared_mutex реализацией*).

По умолчанию для всех тестов в [tests](net_utils/tests) создается target с санитайзерами из списка `TESTS_SANITIZE`. <br/>
Запустить все тесты можно командой `ctest`.

Тесты собраны и запущены:
* GNU C++17 (Ubuntu 10.5.0-1ubuntu1~22.04) version 10.5.0 (x86_64-linux-gnu)
* clang -cc1 version 19.1.1 based upon LLVM 19.1.1 default target x86_64-unknown-linux-gnu
* GNU C++17 (GCC) версия 14.2.0 (x86_64-pc-linux-gnu)

Список целей: <br/>![Снимок экрана от 2024-11-22 15-25-27.png](gifs/%D0%A1%D0%BD%D0%B8%D0%BC%D0%BE%D0%BA%20%D1%8D%D0%BA%D1%80%D0%B0%D0%BD%D0%B0%20%D0%BE%D1%82%202024-11-22%2015-25-27.png)

Проект зависит только от libc++ | libstdc++ и сопутствующих библиотек. <br/>
Разрешение зависимостей происходит автоматически с помощью [cmake-CPM](https://github.com/cpm-cmake/CPM.cmake).

Весь код (c++/cmake), представленный в репозитории, так или иначе, написан [мной](https://github.com/conelov). 
Некоторый код мог быть взят из [CustomHelper](https://github.com/conelov/CustomHelper). <br/>
В ином случае имеется аннотация на источник.