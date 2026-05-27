# Contributing — ovlSysmodules (Ryazhenka)

**EN:** Downstream fork tracking [ppkantorski/ovl-sysmodules](https://github.com/ppkantorski/ovl-sysmodules) daily via CI. Bugs in upstream sysmodule-toggle logic → file upstream. Bugs in our Ryazhenka-specific patches (i18n, libryazhahand swap, CI workflows, Russian docs) → file here.

---

## Что присылать сюда

- Опечатки/неточности в `lang/ru.json` (или другом языковом файле).
- Запросы на новые языки (`lang/<code>.json` + кейс в `source/main.cpp`).
- Баги в `source/ovls_lang.{hpp,cpp}` (например, не подгружается JSON).
- Проблемы со сборкой `scripts/build.ps1` или CI workflows.
- Опечатки в README/CHANGELOG/docs/RU.
- Совместимость с другими репо Ряженки (libryazhahand, nx-ovlloader, и т.д.).

## Что присылать upstream

- Баги в самой логике переключения sysmodules.
- Баги в graceful-shutdown контракте.
- Изменения в Memory widget, scan logic, sort order, TID display.
- Всё что касается общей логики оверлея, не специфичной для Ryazhenka.

Их issue tracker: <https://github.com/ppkantorski/ovl-sysmodules/issues>.

## Как добавить новый язык

1. Скопировать `lang/en.json` → `lang/<2-letter-code>.json`.
2. Перевести **значения**. Ключи и `\uXXXX` Unicode escape sequences
   (Switch glyphs) **не трогать**.
3. Открыть `source/main.cpp`, добавить case в switch:
   ```cpp
   case SetLanguage_Spanish: ovls::loadLanguage("es"); break;
   ```
4. Открыть PR. `verify_build.yml` подтвердит сборку.

## Как открыть PR

1. Форкнуть [Dimasick-git/ovlSysmodules](https://github.com/Dimasick-git/ovlSysmodules).
2. Создать ветку от `main` (не `master` — `master` зеркалит upstream).
3. Закоммитить (стиль свободный, RU/EN на выбор).
4. PR в `main`. На PR прогонится `verify_build.yml`.
5. Если меняешь файлы из [`.github/sync-protected-paths.txt`](.github/sync-protected-paths.txt) — объясни в описании PR.

## Стиль кода

C++: 4 пробела, namespace `ovls::` для всех наших добавок (не загрязняем
`tsl::`/`ult::`). Файлы с LF (см. `.gitattributes`).

Документация: README/CHANGELOG/CONTRIBUTING — формат "1 абзац EN → `---`
→ полно RU", как у всех репо Ряженки.
