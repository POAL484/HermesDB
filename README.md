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

Сервис загружает конфигурацию из `../config.json` относительно текущего каталога и по умолчанию слушает порт `80` на всех сетевых интерфейсах.

## HTTP API

### Создать хранилище пользователя

```http
GET /new
```

В ответе `200 OK` возвращается JSON с UUID созданного хранилища:

```json
{"uuid":"<uuid>"}
```

Передавайте этот UUID в параметре запроса для операций с хранилищем.

### Выполнить операцию

Операционный маршрут принимает запросы `POST` и `PUT`:

```http
POST /?uuid=<uuid>
Content-Type: application/json
```

Тело запроса — JSON-объект со строковыми `op` и `key`, а также полем `value`, которое должно быть JSON-массивом или JSON-объектом:

```json
{"op":"SET","key":"greeting","value":["hello"]}
```

Элементы массива `value` могут быть строками или числами JSON. Строковое значение `SET` создаёт строковый ключ, а числовое — целочисленный. Для строковых операций `value` обычно содержит один элемент; для списков и множеств — несколько значений. Для `HSET` можно передать объект, где имена свойств — поля хэша, а значения — сохраняемые значения. Также поддерживается массив с чередующимися именами полей и значениями. Скалярное значение `value` приводит к ответу `400 Bad Request`.

Операции и результаты:

| `op` | Операция | Дополнительные поля | Результат |
|---:|---|---|---|
| `GET` | Получить значение ключа | `key` | Для строки — строка, для целого числа — JSON-число, для списка — массив, для множества — массив, для хэша — JSON-объект |
| `SET` | Установить строковое или целочисленное значение | `key`, `value` | `OK!`/сохранённое значение |
| `INC` | Увеличить целочисленное значение | `key`, `value` — одно или несколько целых чисел | Итоговое значение после увеличения |
| `DEC` | Уменьшить целочисленное значение | `key`, `value` — одно или несколько целых чисел | Итоговое значение после уменьшения |
| `DELETE` | Удалить ключ | `key` | `OK!` |
| `LPUSH` | Добавить значение в начало списка | `key`, `value` | Новая длина списка |
| `RPUSH` | Добавить значение в конец списка | `key`, `value` | Новая длина списка |
| `LPOP` | Извлечь значение из начала списка | `key` | Извлечённое значение |
| `RPOP` | Извлечь значение из конца списка | `key` | Извлечённое значение |
| `SADD` | Добавить элемент в множество | `key`, `value` | Новый размер множества |
| `SREM` | Удалить элемент из множества | `key`, `value` | Новый размер множества |
| `ISMEMBER` | Проверить принадлежность элемента множеству | `key`, `value` | `true`/`false` |
| `GETITEM` | Получить элемент хэша по ключу | `key`, `value` | JSON-объект с выбранными полями |
| `HSET` | Записать поля и значения в хэш | `key`, `value` — объект `поле: значение` или массив чередующихся полей и значений | Размер хэша |
| `HDEL` | Удалить поля из хэша | `key`, `value` | JSON-объект с удалёнными значениями |

Все операции передаются через `POST` или `PUT`. Ответ операции — JSON-объект в поле `data` со статусом `200`.

Примеры с `curl`:

```sh
# Создать хранилище и скопировать uuid из ответа
curl http://localhost/new

# Записать строку
curl -X POST 'http://localhost/?uuid=<uuid>' \
  -H 'Content-Type: application/json' \
  -d '{"op":"SET","key":"greeting","value":["hello"]}'

# Прочитать строку
curl -X POST 'http://localhost/?uuid=<uuid>' \
  -H 'Content-Type: application/json' \
  -d '{"op":"GET","key":"greeting","value":[]}'

# Создать целочисленный ключ, увеличить его и прочитать результат
curl -X PUT 'http://localhost/?uuid=<uuid>' \
  -H 'Content-Type: application/json' \
  -d '{"op":"SET","key":"visits","value":[10]}'
curl -X PUT 'http://localhost/?uuid=<uuid>' \
  -H 'Content-Type: application/json' \
  -d '{"op":"INC","key":"visits","value":[3]}'
curl -X PUT 'http://localhost/?uuid=<uuid>' \
  -H 'Content-Type: application/json' \
  -d '{"op":"DEC","key":"visits","value":[2]}'
curl -X POST 'http://localhost/?uuid=<uuid>' \
  -H 'Content-Type: application/json' \
  -d '{"op":"GET","key":"visits","value":[]}'

# Значение ключа становится первым элементом списка; добавить второй и извлечь первый
curl -X POST 'http://localhost/?uuid=<uuid>' \
  -H 'Content-Type: application/json' \
  -d '{"op":"SET","key":"queue","value":["first"]}'
curl -X PUT 'http://localhost/?uuid=<uuid>' \
  -H 'Content-Type: application/json' \
  -d '{"op":"RPUSH","key":"queue","value":["second"]}'
curl -X PUT 'http://localhost/?uuid=<uuid>' \
  -H 'Content-Type: application/json' \
  -d '{"op":"LPOP","key":"queue","value":[]}'

# Добавить элементы в множество
curl -X PUT 'http://localhost/?uuid=<uuid>' \
  -H 'Content-Type: application/json' \
  -d '{"op":"SADD","key":"users","value":["john","alice","john"]}'

# Добавить пары в хэш
curl -X PUT 'http://localhost/?uuid=<uuid>' \
  -H 'Content-Type: application/json' \
  -d '{"op":"HSET","key":"profile","value":{"name":"Hermes","role":"gateway"}}'
```

## Кеширование и сохранение

Каждому UUID соответствует отдельное хранилище. Политика вытеснения задаётся в `custom_config.cache_policy`: `LRU` (по умолчанию) или `LFU`.
Алгоритм сжатия записей задаётся в `custom_config.compression_method`: `Snappy` (по умолчанию), `GZIP` или `LZ4`.

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

При `persistent: true` операции сериализуются, сжимаются выбранным алгоритмом, сохраняются в файл для соответствующего UUID и затем загружаются при запуске. Алгоритм записывается для каждой записи отдельно, поэтому сохранённые данные останутся читаемыми и при смене настройки; записи прежнего формата Snappy также поддерживаются. Каталог должен существовать до старта сервиса. При `persistent: false` данные хранятся только в памяти.

## Тесты

```sh
cmake --build build
ctest --test-dir build --output-on-failure
```
