# ovlSysmodules для Ryazhahand

`ovlSysmodules` — overlay-меню для Nintendo Switch, которое показывает установленные sysmodule и позволяет включать, выключать и настраивать их автозапуск без выхода из игры.

Проект основан на актуальном исходнике `ppkantorski/ovl-sysmodules`, но использует библиотеку [`libryazhahand`](https://github.com/Dimasick-git/libryazhahand). Все пользовательские строки интерфейса переведены на русский язык. Русские формулировки не длиннее соответствующих исходных английских строк более чем на три видимых символа; это автоматически проверяется перед сборкой.

## 1.5.4

Обновлена libryazhahand. Компактный текст и элементы в оверлее. libnx 4.12.0.

## Установка

Скачайте файл `ovlSysmodules.zip` из раздела релизов и распакуйте его в корень SD-карты. В архив уже включены следующие пути:

| Содержимое | Путь на SD-карте |
| --- | --- |
| Overlay | `/switch/.overlays/ovlSysmodules.ovl` |
| Русские строки | `/config/ryazhahand/ovlSysmodules/lang/ru.json` |

## Сборка

Перед локальной сборкой инициализируйте подмодуль, затем выполните `make`:

```sh
git submodule update --init --recursive
make -j"$(nproc)"
```

Готовый файл будет создан как `ovlSysmodules.ovl`. Последние четыре байта файла имеют сигнатуру `RYZH`; журнал сборки выводит строку `Ryazhahand signature has been added.`

## Releases

Pushes build the overlay. Releases are published only for version tags and contain ovlSysmodules.zip.

## Проверка локализации

```sh
python3 scripts/check_translation_lengths.py
```

Команда проверяет, что каждая отображаемая русская строка не превышает длину исходной английской строки более чем на три символа. Служебные значки шрифта Nintendo Switch в расчёт не входят.
