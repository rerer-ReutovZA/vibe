# CS 1.6 Data Capture DLL

DLL-модуль для захвата игровых данных из Counter-Strike 1.6 (Steam версия) и передачи их через WebSocket.

## Возможности

### Захват данных:
- **Данные игроков**: ники, фраги (K/D/A), деньги, команда, статус жизни
- **Статус бомбы C4**: установлена ли, таймер до взрыва, обезврежена ли
- **Игровые события**: начало раунда, установка бомбы, обезвреживание, взрыв

### Технические особенности:
- Безопасное чтение из памяти игры
- Асинхронная отправка данных через WebSocket
- Детальное логирование для отладки
- Автоматическое переподключение при потере связи
- Минимальное влияние на производительность

## Структура проекта

```
dll/
├── CMakeLists.txt          # Файл сборки CMake
├── README.md               # Документация
├── include/                # Заголовочные файлы
│   ├── game_types.h        # Типы данных игры
│   ├── logger.h            # Система логирования
│   ├── memory_reader.h     # Чтение памяти
│   ├── websocket_client.h  # WebSocket клиент
│   └── game_data_capture.h # Основной класс захвата
├── src/                    # Исходный код
│   ├── dllmain.cpp         # Точка входа DLL
│   ├── logger.cpp
│   ├── memory_reader.cpp
│   ├── websocket_client.cpp
│   └── game_data_capture.cpp
└── build/                  # Папка для сборки
```

## Требования

### Для компиляции:
- Windows 10/11
- Visual Studio 2019 или новее (с C++ компонентами)
- CMake 3.15 или новее
- Windows SDK

### Для запуска:
- Counter-Strike 1.6 (Steam версия)
- WebSocket сервер, слушающий на localhost:8080

## Сборка проекта

### Вариант 1: Используя CMake и Visual Studio

1. Откройте командную строку (Developer Command Prompt for VS)

2. Перейдите в папку dll:
```bash
cd dll
```

3. Создайте папку для сборки:
```bash
mkdir build
cd build
```

4. Сгенерируйте проект:
```bash
cmake ..
```

5. Соберите проект:
```bash
cmake --build . --config Release
```

6. DLL файл будет находиться в `build/bin/Release/cs16_datacapture.dll`

### Вариант 2: Используя Visual Studio напрямую

1. Откройте Visual Studio
2. File → Open → CMake...
3. Выберите `dll/CMakeLists.txt`
4. Build → Build All
5. DLL файл будет в папке `out/build/`

## Использование

### Автоматический запуск (DLL injection):

1. Используйте инжектор DLL (например, [Extreme Injector](https://github.com/master131/ExtremeInjector))
2. Запустите Counter-Strike 1.6
3. Инжектируйте `cs16_datacapture.dll` в процесс `hl.exe`
4. DLL автоматически начнет захватывать и отправлять данные

### Ручное управление через экспортируемые функции:

```cpp
// Загрузка DLL
HMODULE hDll = LoadLibrary("cs16_datacapture.dll");

// Получение функций
auto StartCapture = (bool(*)())GetProcAddress(hDll, "StartCapture");
auto StopCapture = (void(*)())GetProcAddress(hDll, "StopCapture");
auto IsCapturing = (bool(*)())GetProcAddress(hDll, "IsCapturing");
auto SetUpdateInterval = (void(*)(int))GetProcAddress(hDll, "SetUpdateInterval");

// Запуск захвата
if (StartCapture()) {
    printf("Capture started\n");
}

// Изменение интервала обновления (в миллисекундах)
SetUpdateInterval(100);  // 100ms = 10 обновлений в секунду

// Остановка захвата
StopCapture();

// Выгрузка DLL
FreeLibrary(hDll);
```

## Формат данных WebSocket

DLL отправляет данные в JSON формате:

```json
{
  "players": [
    {
      "name": "Player1",
      "kills": 10,
      "deaths": 5,
      "assists": 2,
      "money": 3200,
      "team": 2,
      "isAlive": true
    }
  ],
  "bomb": {
    "planted": true,
    "timeRemaining": 35.5,
    "defused": false
  },
  "events": [
    "Bomb Planted"
  ],
  "roundNumber": 5,
  "roundTime": 120.5
}
```

## Конфигурация

Настройки находятся в `src/dllmain.cpp`:

```cpp
static const std::string WS_HOST = "localhost";  // Адрес WebSocket сервера
static const int WS_PORT = 8080;                 // Порт WebSocket сервера
static const std::string LOG_FILE = "cs16_datacapture.log";  // Файл логов
```

Для изменения настроек отредактируйте эти константы и пересоберите проект.

## Логирование

Логи записываются в файл `cs16_datacapture.log` в той же папке, где находится `hl.exe`.

Формат логов:
```
[2024-12-26 15:30:45.123] [INFO] DLL attached to process
[2024-12-26 15:30:45.124] [INFO] Capture system initialized successfully
[2024-12-26 15:30:45.125] [DEBUG] Game state sent successfully
```

Уровни логирования:
- `DEBUG` - Детальная информация для отладки
- `INFO` - Общая информация о работе
- `WARNING` - Предупреждения
- `ERROR` - Ошибки
- `CRITICAL` - Критические ошибки

## Важные замечания

### Безопасность и античит:
⚠️ **ВНИМАНИЕ**: Использование данной DLL может быть расценено античит-системой VAC как вмешательство в игровой процесс. Используйте только на собственный риск и только на серверах без VAC защиты.

### Поиск смещений памяти:
Текущая реализация содержит placeholder-смещения для чтения игровых данных. Для корректной работы необходимо:

1. Использовать инструменты реверс-инжиниринга (Cheat Engine, IDA Pro, Ghidra)
2. Найти актуальные смещения для вашей версии CS 1.6
3. Обновить значения в методе `GameDataCapture::initializeOffsets()`

Смещения могут отличаться в зависимости от:
- Версии игры (Steam / Non-Steam)
- Установленных модов
- Обновлений игры

## Расширение функционала

### Добавление новых данных для захвата:

1. Добавьте поля в структуры в `game_types.h`
2. Обновите методы захвата в `game_data_capture.cpp`
3. Обновите JSON сериализацию в `websocket_client.cpp`

### Изменение протокола передачи:

Текущая реализация использует простой TCP сокет. Для полноценного WebSocket протокола:

1. Подключите библиотеку WebSocket++ или Boost.Beast
2. Обновите `WebSocketClient` для использования библиотеки
3. Реализуйте WebSocket handshake

## Тестирование

### Тестовый WebSocket сервер на Node.js:

Создайте файл `test_server.js`:

```javascript
const WebSocket = require('ws');

const wss = new WebSocket.Server({ port: 8080 });

wss.on('connection', (ws) => {
  console.log('Client connected');
  
  ws.on('message', (message) => {
    console.log('Received:', message.toString());
    try {
      const data = JSON.parse(message);
      console.log('Players:', data.players.length);
      console.log('Bomb planted:', data.bomb.planted);
    } catch (e) {
      console.error('Parse error:', e);
    }
  });

  ws.on('close', () => {
    console.log('Client disconnected');
  });
});

console.log('WebSocket server listening on port 8080');
```

Запустите:
```bash
npm install ws
node test_server.js
```

## Troubleshooting

### DLL не загружается:
- Убедитесь, что DLL собрана для правильной архитектуры (x86 для CS 1.6)
- Проверьте зависимости DLL с помощью Dependency Walker
- Проверьте логи в `cs16_datacapture.log`

### Не подключается к WebSocket:
- Убедитесь, что сервер запущен и слушает на порту 8080
- Проверьте настройки файрвола
- Проверьте логи подключения в файле логов

### Не захватываются данные:
- Смещения памяти нуждаются в обновлении для вашей версии CS 1.6
- Используйте Cheat Engine для поиска актуальных адресов
- Проверьте логи DEBUG уровня для деталей

## Лицензия

MIT License - свободно используйте и модифицируйте.

## Поддержка

При возникновении проблем:
1. Проверьте файл логов `cs16_datacapture.log`
2. Убедитесь, что используете актуальные смещения памяти
3. Проверьте версию Counter-Strike 1.6

## TODO

- [ ] Найти актуальные смещения памяти для CS 1.6 Steam
- [ ] Добавить поддержку конфигурационного файла
- [ ] Реализовать полноценный WebSocket протокол
- [ ] Добавить шифрование передаваемых данных
- [ ] Добавить поддержку нескольких WebSocket клиентов
- [ ] Оптимизация производительности
- [ ] Юнит-тесты
- [ ] CI/CD pipeline
