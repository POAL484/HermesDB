# HermesDB

NoSQL-хранилище с HTTP API на C++. Смотрите также [MnemosyneDB](https://github.com/POAL484/MnemosyneDB/).

> **Гермес** — в древнегреческой мифологии бог-вестник и посланник Олимпа. Так же этот проект стремится быть быстрым и надёжным посредником между хранилищем и приложением.

## Запуск

Проект собирается с помощью CMake и библиотеки [Drogon](https://github.com/drogonframework/drogon):

```sh
cmake -S . -B build
cmake --build build
cd build
./cache_service
```

Сервис загружает конфигурацию из json файла, путь до которого задан в переменной окружения `HERMES_CONFIG`.

### Запуск в Docker

Соберите образ и запустите сервис из корня проекта:

```sh
docker compose up --build -d
```

HTTP API будет доступно на `http://localhost:80` и извне Docker-хоста на его порту `80`. Compose сохраняет данные между перезапусками в томе `hermesdb-data`. Чтобы остановить сервис, выполните `docker compose down`; для удаления также и сохранённых данных — `docker compose down -v`.

## HTTP API

### Создать хранилище пользователя

```http
GET /new
```

В ответе `200 OK` возвращается JSON с UUID созданного хранилища:

```json
{"uuid":"<uuid>"}
```

Передавайте этот UUID в HTTP-заголовке `user` для операций с хранилищем.

### Выполнить операцию

Операционный маршрут принимает запросы `POST` и `PUT`:

```http
POST /
user: <uuid>
Content-Type: application/json
```

Тело запроса — JSON-объект со строковыми `op` и `key`. Тип и поле значения зависят от операции:

```json
{"op":"SET","key":"greeting","value":"hello"}
```

Строковое значение `SET` создаёт строковый ключ, числовое — целочисленный ключ, массив — список, а объект — хэш. Для создания множества передайте объект `{"$set":["один","два"]}`. Для `HSET` поле `value` должно быть объектом с парами «поле: значение». Операции `GETITEM` принимают массив индексов или строковых ключей в поле `item`; `HDEL` принимает массив строковых ключей в поле `mapKeys`. Для `INC` и `DEC` поле `value` должно быть числом.

Операции и результаты:

| `op` | Операция | Дополнительные поля | Результат |
|---:|---|---|---|
| `GET` | Получить значение ключа | `key` | Для строки — строка, для целого числа — JSON-число, для списка — массив, для множества — массив, для хэша — JSON-объект |
| `SET` | Установить строковое, числовое, списочное, множественное или хэш-значение | `key`, `value` | `{"OK":"OK!"}` |
| `INC` | Увеличить целочисленное значение | `key`, `value` — число | Итоговое значение в поле `value` |
| `DEC` | Уменьшить целочисленное значение | `key`, `value` — число | Итоговое значение в поле `value` |
| `DELETE` / `DEL` | Удалить ключ | `key` | Удалённое значение в поле `value` |
| `SIZE` | Получить размер строки, списка, множества или хэша | `key` | Размер в поле `size` |
| `LPUSH` | Добавить значение в начало списка | `key`, `value` | Новая длина списка |
| `RPUSH` | Добавить значение в конец списка | `key`, `value` | Новая длина списка |
| `LPOP` | Извлечь значение из начала списка | `key` | Извлечённое значение |
| `RPOP` | Извлечь значение из конца списка | `key` | Извлечённое значение |
| `SADD` | Добавить элемент в множество | `key`, `value` | Новый размер множества |
| `SREM` | Удалить элемент из множества | `key`, `value` | Новый размер множества |
| `ISMEMBER` | Проверить принадлежность элемента множеству | `key`, `value` | `true`/`false` |
| `GETITEM` | Получить элементы списка или поля хэша | `key`, `item` — массив строковых ключей или целочисленных индексов | JSON-объект в поле `value` |
| `HSET` | Записать поля и значения в хэш | `key`, `value` — объект `поле: значение` | Размер хэша в поле `size` |
| `HDEL` | Удалить поля из хэша | `key`, `mapKeys` — массив строковых ключей | Удалённые поля в поле `deleted` |

Все операции передаются через `POST` или `PUT`. Успешный ответ содержит HTTP-статус `200`, поле `status: 200` и данные операции в поле `data`. Если UUID неизвестен, сервис отвечает HTTP `403` и JSON вида `{"status":403,"errCode":0,"err":"Unknown user uuid"}`. Ошибки синтаксиса, семантики и выполнения операции возвращают соответствующий HTTP-статус и JSON вида `{"status":400,"errCode":<код>,"err":"<сообщение>"}`.

Примеры с `curl`:

```sh
# Создать хранилище и скопировать uuid из ответа
curl http://localhost/new

# Записать строку
curl -X POST 'http://localhost/' \
  -H 'user: <uuid>' \
  -H 'Content-Type: application/json' \
  -d '{"op":"SET","key":"greeting","value":"hello"}'

# Прочитать строку
curl -X POST 'http://localhost/' \
  -H 'user: <uuid>' \
  -H 'Content-Type: application/json' \
  -d '{"op":"GET","key":"greeting"}'

# Создать целочисленный ключ, увеличить его и прочитать результат
curl -X PUT 'http://localhost/' \
  -H 'user: <uuid>' \
  -H 'Content-Type: application/json' \
  -d '{"op":"SET","key":"visits","value":10}'
curl -X PUT 'http://localhost/' \
  -H 'user: <uuid>' \
  -H 'Content-Type: application/json' \
  -d '{"op":"INC","key":"visits","value":3}'
curl -X PUT 'http://localhost/' \
  -H 'user: <uuid>' \
  -H 'Content-Type: application/json' \
  -d '{"op":"DEC","key":"visits","value":2}'
curl -X POST 'http://localhost/' \
  -H 'user: <uuid>' \
  -H 'Content-Type: application/json' \
  -d '{"op":"GET","key":"visits"}'

# Значение ключа становится первым элементом списка; добавить второй и извлечь первый
curl -X POST 'http://localhost/' \
  -H 'user: <uuid>' \
  -H 'Content-Type: application/json' \
  -d '{"op":"SET","key":"queue","value":["first"]}'
curl -X PUT 'http://localhost/' \
  -H 'user: <uuid>' \
  -H 'Content-Type: application/json' \
  -d '{"op":"RPUSH","key":"queue","value":"second"}'
curl -X PUT 'http://localhost/' \
  -H 'user: <uuid>' \
  -H 'Content-Type: application/json' \
  -d '{"op":"LPOP","key":"queue"}'

# Добавить элементы в множество
curl -X PUT 'http://localhost/' \
  -H 'user: <uuid>' \
  -H 'Content-Type: application/json' \
  -d '{"op":"SADD","key":"users","value":"john"}'
curl -X PUT 'http://localhost/' \
  -H 'user: <uuid>' \
  -H 'Content-Type: application/json' \
  -d '{"op":"SADD","key":"users","value":"alice"}'
curl -X POST 'http://localhost/' \
  -H 'user: <uuid>' \
  -H 'Content-Type: application/json' \
  -d '{"op":"ISMEMBER","key":"users","value":"john"}'

# Добавить пары в хэш
curl -X PUT 'http://localhost/' \
  -H 'user: <uuid>' \
  -H 'Content-Type: application/json' \
  -d '{"op":"HSET","key":"profile","value":{"name":"Hermes","role":"gateway"}}'

# Получить поле хэша и размер хэша
curl -X POST 'http://localhost/' \
  -H 'user: <uuid>' \
  -H 'Content-Type: application/json' \
  -d '{"op":"GETITEM","key":"profile","item":["name"]}'
curl -X POST 'http://localhost/' \
  -H 'user: <uuid>' \
  -H 'Content-Type: application/json' \
  -d '{"op":"SIZE","key":"profile"}'
```

## Кеширование и сохранение

Каждому UUID соответствует отдельное хранилище. Политика вытеснения задаётся в `custom_config.cache_policy`: `LRU` (по умолчанию), `LFU` или `TLRU`.
Для политики TLRU TTL передаётся целым числом секунд в поле `ttl` операций записи. Значение `0` означает отсутствие срока жизни. Например:

```json
{"op":"SET","key":"session","value":"active","ttl":60}
```
Алгоритм сжатия записей задаётся в `custom_config.compression_method`: `Snappy` (по умолчанию), `GZIP` или `LZ4`.

Настройки можно переопределить переменными окружения. Если переменная задана, её значение имеет приоритет над соответствующим полем `custom_config`:

| Переменная окружения | Поле `config.json` | Допустимые значения |
| --- | --- | --- |
| `CACHE_ALGORITHM` | `cache_policy` | `LRU`, `LFU`, `TLRU` |
| `DISK_ENABLE` | `persistent` | `true`, `false`, `1`, `0` |
| `DISK_COMPRESSION_ALGORITHM` | `compression_method` | `Snappy`, `GZIP`, `LZ4` |

Например, для запуска с LFU, включённой персистентностью и LZ4:

```sh
CACHE_ALGORITHM=LFU DISK_ENABLE=true DISK_COMPRESSION_ALGORITHM=LZ4 docker compose up --build -d
```

Персистентность также настраивается в `custom_config` файла `config.json`:

```json
{
  "custom_config": {
    "cache_policy": "LRU",
    "compression_method": "Snappy",
    "persistent": true,
    "persistent_path": "/home/user/.hermes/"
  }
}
```

При `persistent: true` операции сериализуются, сжимаются выбранным алгоритмом, сохраняются в файл для соответствующего UUID и затем загружаются при запуске. Для TLRU в журнал записывается абсолютный срок истечения, поэтому время остановки программы учитывается: уже истёкшие ключи не загружаются, а для остальных сохраняется оставшийся TTL. Алгоритм сжатия записывается для каждой записи отдельно, поэтому данные остаются читаемыми при смене настройки алгоритма. Каталог должен существовать до старта сервиса. При `persistent: false` данные хранятся только в памяти.

## Тесты

```sh
cmake --build build
ctest --test-dir build --output-on-failure
```
