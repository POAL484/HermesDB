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

Тело запроса — JSON-объект с числовым `op`, строковым `key` и (для записи или добавления в список) строковым `value`:

```json
{"op":1,"key":"greeting","value":"hello"}
```

Значения `op`:

| `op` | Операция | Дополнительные поля | Результат |
|---:|---|---|---|
| `0` | `GET` — получить строковое значение | `key` | Значение; для отсутствующего ключа — пустая строка |
| `1` | `SET` — установить строковое значение | `key`, `value` | `OK!` |
| `2` | `DELETE` — удалить ключ | `key` | `OK!` |
| `3` | `LPUSH` — добавить значение в начало списка | `key`, `value` | Новая длина списка |
| `4` | `RPUSH` — добавить значение в конец списка | `key`, `value` | Новая длина списка |
| `5` | `LPOP` — извлечь значение из начала списка | `key` | Извлечённое значение |
| `6` | `RPOP` — извлечь значение из конца списка | `key` | Извлечённое значение |

Все операции передаются через `POST` или `PUT`; отдельные HTTP-маршруты `GET` и `DELETE` для ключей не предусмотрены. Ответ операции — текстовое тело с результатом (код `200`).

При первой операции со списком строковое значение ключа становится первым элементом списка. Для нового ключа этим значением будет пустая строка.

Примеры с `curl`:

```sh
# Создать хранилище и скопировать uuid из ответа
curl http://localhost/new

# Записать строку
curl -X POST 'http://localhost/?uuid=<uuid>' \
  -H 'Content-Type: application/json' \
  -d '{"op":1,"key":"greeting","value":"hello"}'

# Прочитать строку
curl -X POST 'http://localhost/?uuid=<uuid>' \
  -H 'Content-Type: application/json' \
  -d '{"op":0,"key":"greeting"}'

# Значение ключа становится первым элементом списка; добавить второй и извлечь первый
curl -X POST 'http://localhost/?uuid=<uuid>' \
  -H 'Content-Type: application/json' \
  -d '{"op":1,"key":"queue","value":"first"}'
curl -X PUT 'http://localhost/?uuid=<uuid>' \
  -H 'Content-Type: application/json' \
  -d '{"op":4,"key":"queue","value":"second"}'
curl -X PUT 'http://localhost/?uuid=<uuid>' \
  -H 'Content-Type: application/json' \
  -d '{"op":5,"key":"queue"}'
```

## Кеширование и сохранение

Каждому UUID соответствует отдельное хранилище. Политика вытеснения задаётся в `custom_config.cache_policy`: `LRU` (по умолчанию) или `LFU`.

Персистентность также настраивается в `custom_config` файла `config.json`:

```json
{
  "custom_config": {
    "cache_policy": "LRU",
    "persistent": true,
    "persistent_path": "/home/user/.hermes/"
  }
}
```

При `persistent: true` операции записываются в файл для соответствующего UUID, а файлы из `persistent_path` загружаются при запуске. Каталог должен существовать до старта сервиса. При `persistent: false` данные хранятся только в памяти.

## Тесты

```sh
cmake --build build
ctest --test-dir build --output-on-failure
```
