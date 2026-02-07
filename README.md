# DistributedIntegration
Клиент-серверная система, в которой сервер формирует задачу по численному интегрированию для функции 1/ln(x) с заданием от пользователя: нижнего предела, верхнего предела и шага интегрирования. Клиенты подключаются к серверу по сети. Сервер делит общую задачу на всех подключенных клиентов равномерно по количеству ядер CPU на каждом клиенте. Клиенты одновременно запускают свои полученные задачи на своих вычислительных ядрах и передают результаты вычислений на сервер. Сервер после получения всех промежуточных результатов суммирует их и выводит на экран.

# Сборка

## Требования
- CMAKE >= 3.30
- C++17 совместимый компилятор (MSVC, GCC, Clang)
- Git

## Инициализация подмодулей

```bash
# 1)
git submodule update --init src/third_party/boost
cd src/third_party/boost
# 2)
bootstrap.bat  # Если Windows
./boostrap.sh  # Если Linux или macOS
# 3)
b2 headers     # Если Windows
./b2 headers   # Если Linux или macOS
# 4)
cd ../../..
```

## Сборка проекта

### Windows

```bash
# Debug
cmake --preset x64-debug
cmake --build out/build/x64-debug

# Release
cmake --preset x64-release
cmake --build out/build/x64-release
```

### Linux

```bash
# Debug
cmake --preset linux-debug
cmake --build out/build/linux-debug

# Release
cmake --preset linux-release
cmake --build out/build/linux-release
```

### macOS

```bash
# Debug
cmake --preset macos-debug
cmake --build out/build/macos-debug

# Release
cmake --preset macos-release
cmake --build out/build/macos-release
```

## Запуск

```bash
# Сервер
./out/build/<preset>/bin/server

# Клиент
./out/build/<preset>/bin/client
```