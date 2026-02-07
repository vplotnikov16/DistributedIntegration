# DistributedIntegration
Клиент-серверная система, в которой сервер формирует задачу по численному интегрированию для функции 1/ln(x) с заданием от пользователя: нижнего предела, верхнего предела и шага интегрирования. Клиенты подключаются к серверу по сети. Сервер делит общую задачу на всех подключенных клиентов равномерно по количеству ядер CPU на каждом клиенте. Клиенты одновременно запускают свои полученные задачи на своих вычислительных ядрах и передают результаты вычислений на сервер. Сервер после получения всех промежуточных результатов суммирует их и выводит на экран.

## Сборка

## Требования
- CMAKE >= 3.30
- C++17 совместимый компилятор (MSVC, GCC, Clang)
- Git

## Зависимости
В проекте используются следующие библиотеки:
- Boost - подмодуль git для сетевого взаимодействия и юнит тестов
- Cereal - подмодуль git для сериализации данных
- spdlog - подмодуль git для логирования
- Threads - системная библиотека для многопоточности

## Сборка проекта

### 1. Клонирование репозитория

```bash
git clone --recurse-submodules https://github.com/vplotnikov16/DistributedIntegration.git
cd DistributedIntegration
```
Для инициализации подмодулей в уже склонированном репозитории без опции `--recurse-submodules` нужно выполнить

```bash
git submodule update --init --recursive
```

### 2. Подготовка Boost

```bash
# 1)
git submodule update --init src/third_party/boost
cd src/third_party/boost
# 2)
bootstrap.bat  # Если Windows
./bootstrap.sh  # Если Linux или macOS
# 3)
b2 headers     # Если Windows
./b2 headers   # Если Linux или macOS
# 4)
cd ../../..
```

### 3. Сборка проекта через CMake

#### Windows

```bash
# Debug
cmake --preset x64-debug
cmake --build out/build/x64-debug

# Release
cmake --preset x64-release
cmake --build out/build/x64-release
```

#### Linux

```bash
# Debug
cmake --preset linux-debug
cmake --build out/build/linux-debug

# Release
cmake --preset linux-release
cmake --build out/build/linux-release
```

#### macOS

```bash
# Debug
cmake --preset macos-debug
cmake --build out/build/macos-debug

# Release
cmake --preset macos-release
cmake --build out/build/macos-release
```

## Запуск приложения

```bash
# Сервер
./out/build/<preset>/bin/server

# Клиент
./out/build/<preset>/bin/client
```

## Инструкция по работе с приложением

Сначала необходимо запустить сервер.

```bash
server
```

Далее запустить клиент, передав первым аргументом командной строки ip-адрес сервера, затем порт 5555:

```bash
# Пример в случае запуска сервера и клиента на одной машине
client 127.0.0.1 5555
```

Если сервер больше не ожидает подключений, написать в консоль сервера "START" для прекращения ожидания новых клиентов, подготовки задач для подключенных клиентов и отправки им задач.
Клиенты получат свои задачи, начнут распределенное интегрирование методом Симпсона (выбор захардкожен). Разработан также метод трапеций, но из интерфейса консоли поменять выбор нельзя (не реализовано).
Как только все клиенты отправят результат серверу, сервер сформирует итоговый ответ и завершит работу, предварительно отправив клиентам команду о завершении работы. 

## Тестирование

Проект включает модульные тесты для проверки корректности методов численного интегрирования.

### Подготовка к запуску тестов

Тесты используют Boost.Test в header-only режиме, поэтому дополнительная сборка библиотек не требуется. Убедитесь, что вы выполнили шаг "Подготовка Boost" из раздела "Сборка проекта".

### Сборка с тестами

Для включения сборки тестов добавьте опцию `-DBUILD_TESTS=ON` при конфигурации проекта:

#### Windows

```bash
# Debug
cmake --preset x64-debug -DBUILD_TESTS=ON
cmake --build out/build/x64-debug --config Debug

# Release
cmake --preset x64-release -DBUILD_TESTS=ON
cmake --build out/build/x64-release --config Release
```

#### Linux

```bash
# Debug
cmake --preset linux-debug -DBUILD_TESTS=ON
cmake --build out/build/linux-debug

# Release
cmake --preset linux-release -DBUILD_TESTS=ON
cmake --build out/build/linux-release
```


#### macOS

```bash
cmake --preset macos-debug -DBUILD_TESTS=ON
cmake --build out/build/macos-debug

# Release
cmake --preset macos-release -DBUILD_TESTS=ON
cmake --build out/build/macos-release
```

### Запус тестов

```bash
# Все тесты
ctest --test-dir out/build/<preset> -C Debug --output-on-failure

# С подробным выводом
ctest --test-dir out/build/<preset> -C Debug --output-on-failure --verbose
```