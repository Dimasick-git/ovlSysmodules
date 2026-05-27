# Changelog — ovlSysmodules (Ryazhenka)

Заметные изменения форка относительно [ppkantorski/ovl-sysmodules](https://github.com/ppkantorski/ovl-sysmodules) документируются здесь. Формат — [Keep a Changelog](https://keepachangelog.com/ru/1.1.0/).

## [1.5.1] — 2026-05-27

Первый релиз форка под брендом Ряженка. Синхронизирован с upstream
`v1.5.1` (коммит `ca44b71`, "Update libultrahand").

### Добавлено

- **`libryazhahand` вместо `libultrahand`** как submodule (`libs/libryazhahand/`).
  Source-совместимая замена с конфиг-namespace `/config/ryazhahand/`.
- **Подпись `.ovl` файла `ULTR` → `RYZH`** в `Makefile`. Последние 4 байта
  overlay'я теперь = `0x52 0x59 0x5A 0x48` (Ryazhahand) вместо
  `0x55 0x4C 0x54 0x52` (Ultrahand). `nx-ovlloader` подпись не валидирует
  (подтверждено инспекцией `source/main.c`), поэтому загрузка не ломается;
  overlay-managers Ряженки (Ryazhahand-Overlay) теперь узнают этот файл
  как "свой".
- **Локализация EN/RU.** Новый модуль `source/ovls_lang.{hpp,cpp}` грузит
  `/config/ryazhahand/ovlSysmodules/lang/<code>.json` при старте оверлея.
  Все UI-строки (заголовок, секционные заголовки, хинты, On/Off значения)
  переводятся. Glyph-иконки Switch system-font (``, ``,
  ``, ``, ``, ``) сохранены во всех переводах.
- **`lang/en.json` и `lang/ru.json`** — английский (= upstream defaults) +
  русский. Распаковываются в `/config/ryazhahand/ovlSysmodules/lang/`.
- **`.gitattributes`** — `* text=auto eol=lf` для воспроизводимых сборок
  между Windows-разработкой и Linux CI.
- **CI/CD набор** (см. [`.github/workflows/`](.github/workflows/)):
  - `build.yml` — Docker `devkitpro/devkita64` сборка на push в `main` + PR;
  - `release.yml` — авто-релиз на push тега или изменение `Makefile`;
  - `sync_upstream.yml` — daily 03:00 UTC sync c
    `ppkantorski/ovl-sysmodules:master` через PR, защищая
    `.github/sync-protected-paths.txt`;
  - `verify_build.yml` — PR smoke-тест.
- **`scripts/build.ps1`** — локальная сборка через Docker под Windows
  без необходимости ставить devkitPro/msys2.

### Изменено

- `Makefile`: `APP_TITLE := "Sysmodules (Ryazhenka)"`, добавлен
  `APP_AUTHOR`. `include ... libultrahand/ultrahand.mk` → `libryazhahand/ryazhahand.mk`.
- `source/gui_main.cpp`: все user-visible string literals заменены на
  ссылки в `ovls::` namespace. Поведение функционально идентично
  upstream (только источник строк теперь runtime-loadable).
- `source/main.cpp::initServices`: добавлен вызов `setGetSystemLanguage`
  + `ovls::loadLanguage(...)` с маппингом language enum на 2-буквенный код.

### Не изменилось (наследуется из upstream v1.5.1)

- TID, путь установки `/switch/.overlays/ovlSysmodules.ovl`.
- IPC graceful-shutdown контракт через `toolbox.json`.
- Все upstream-фичи: scan, sort, RAM widget, TID toggle, etc.

[1.5.1]: https://github.com/Dimasick-git/ovlSysmodules/releases/tag/v1.5.1
