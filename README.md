<p align="left">
<a href="https://github.com/Dimasick-git/ovlSysmodules/blob/main/LICENSE"><img alt="License" src="https://img.shields.io/badge/license-GPLv2-blue.svg"></a>
<a href="https://github.com/Dimasick-git/ovlSysmodules/releases/latest"><img alt="Latest release" src="https://img.shields.io/github/v/release/Dimasick-git/ovlSysmodules?include_prereleases"></a>
<a href="https://github.com/Dimasick-git/ovlSysmodules/actions/workflows/build.yml"><img alt="Build" src="https://github.com/Dimasick-git/ovlSysmodules/actions/workflows/build.yml/badge.svg"></a>
<a href="https://github.com/Dimasick-git/ovlSysmodules/actions/workflows/sync_upstream.yml"><img alt="Upstream sync" src="https://github.com/Dimasick-git/ovlSysmodules/actions/workflows/sync_upstream.yml/badge.svg"></a>
</p>

# ovlSysmodules (Ryazhenka edition)

**EN:** Ryazhenka fork of [ppkantorski/ovl-sysmodules](https://github.com/ppkantorski/ovl-sysmodules) — the Tesla overlay that lets you toggle Atmosphère sysmodules on/off and flip their `boot2.flag` auto-start without rebooting (for dynamic modules) or with a reboot (for static ones). Three changes vs. upstream: (1) the `libultrahand` submodule is swapped for [`Dimasick-git/libryazhahand`](https://github.com/Dimasick-git/libryazhahand) so it integrates with the rest of the Ryazha overlay stack (`/config/ryazhahand/` namespace, `Ryazhahand-Overlay`, `RCU`, etc.); (2) the trailing 4-byte overlay magic is changed from `ULTR` (Ultrahand) to `RYZH` (Ryazhahand) so the Ryazha overlay-managers recognise this as a Ryazha-native overlay; (3) the UI is fully localised — Russian via `lang/ru.json` if the system language is set to Russian, otherwise English defaults (with Switch system-font glyphs preserved verbatim).

---

## Что это

Это форк [ppkantorski/ovl-sysmodules](https://github.com/ppkantorski/ovl-sysmodules) — Tesla-оверлей для Switch homebrew (Atmosphère CFW), который позволяет:

- включать/выключать дочерние sysmodules (overclock, gamepad fixes, audio overrides и т.д.) на лету (модули без `requires_reboot`);
- перезапускать с graceful-shutdown через IPC-контракт (модули с `shutdown_service`/`shutdown_cmd` в их `toolbox.json` — например наш `RCU`);
- ставить/снимать флаг автозапуска `boot2.flag` для любого модуля;
- видеть свободную RAM в шапке оверлея;
- показывать TID контроллеров (по нажатию `MINUS`).

Форк сделан под экосистему Ряженка ([Atmosphere](https://github.com/Dimasick-git/Atmosphere), [Ryazhahand-Overlay](https://github.com/Dimasick-git/Ryazhahand-Overlay), [RCU](https://github.com/Dimasick-git/RCU), [libryazhahand](https://github.com/Dimasick-git/libryazhahand), [nx-ovlloader](https://github.com/Dimasick-git/nx-ovlloader)) и тестируется только в её связке.

## Главные отличия от upstream

| Что | Где живёт | Зачем |
|------|-----------|-------|
| **Submodule `libultrahand` → `libryazhahand`** | `.gitmodules`, `Makefile:50` | Канонический submodule всей Ряженки. Source-совместим с upstream (тот же `tsl::`/`ult::` namespace, тот же API `ryazhahand.mk` ≡ `ultrahand.mk`). Конфиг-путь становится `/config/ryazhahand/...` вместо `/config/ultrahand/...`. |
| **Подпись `ULTR` → `RYZH`** | `Makefile:222`, `printf 'RYZH' >> $@` | Последние 4 байта `.ovl` файла — overlay magic. Upstream писал `ULTR` (Ultrahand). Мы пишем `RYZH` (Ryazhahand) чтобы наш overlay узнавался Ryazhahand-Overlay и другими visualisers Ряженки как "свой". `nx-ovlloader` сам сигнатуру не валидирует, так что загрузка не ломается. |
| **Локализация EN/RU** | `source/ovls_lang.{hpp,cpp}` + `lang/{en,ru}.json` | Все UI-строки (заголовок, секции "Dynamic"/"Static", хинты, `On`/`Off` значения) переводятся при старте оверлея на основе `setGetSystemLanguage()`. Switch system-font глифы (кнопочные иконки, разделители, info-иконка) сохранены в обоих языках. |
| **Брендинг** | `Makefile`, `APP_TITLE = "Sysmodules (Ryazhenka)"` | Видно в DBI/Tinfoil чтобы отличать от ванильной сборки. TID и `/atmosphere/contents/<TID>/` path не трогаются. |
| **Защищённый sync с upstream** | `.github/workflows/sync_upstream.yml` + `.github/sync-protected-paths.txt` | Каждый день в 03:00 UTC бот тянет `ppkantorski/ovl-sysmodules:master` и открывает PR. Наши файлы (i18n, README, CI, submodule URL) автоматически оставляются нашими версиями при конфликтах. |
| **Auto-release при изменении Makefile или push тега** | `.github/workflows/release.yml` | Push тега `v*.*.*` либо изменение `Makefile` на `main` → CI собирает `.ovl` через `devkitpro/devkita64` Docker и публикует GitHub Release с SHA-256. |

## Установка

1. Скачать свежий релиз: [Releases →](https://github.com/Dimasick-git/ovlSysmodules/releases/latest) (файл `ovlSysmodules-1.5.1-*.zip`).
2. Распаковать в корень SD-карты с перезаписью. Содержимое лягет в:
   - `/switch/.overlays/ovlSysmodules.ovl`
   - `/config/ryazhahand/ovlSysmodules/lang/{en,ru}.json`
3. Перезагрузить консоль (или просто открыть Tesla оверлей через `L+DPAD_DOWN+RStick`).

**Требования**: Atmosphère последней версии, [nx-ovlloader (Ryazhenka)](https://github.com/Dimasick-git/nx-ovlloader) загружен.

## Использование

| Действие | Кнопка |
|----------|--------|
| Включить/выключить выделенный модуль (для динамических и graceful-shutdown) | `A` |
| Поставить/снять `boot2.flag` (автозапуск при загрузке) | `Y` |
| Переключить отображение TID контроллеров | `MINUS` |
| Закрыть оверлей | `B` (стандартная Tesla биндинг) |

Модули группируются в две секции:

- **Динамические / Автозапуск / Переключить** — модули без `requires_reboot`, либо со встроенным graceful-shutdown контрактом. Их можно гасить и поднимать прямо из оверлея.
- **Статические / Автозапуск** — модули с `requires_reboot: true` без graceful-shutdown. Для них доступен только тоггл `boot2.flag`, изменения применяются после перезагрузки.

## Локализация

Добавить новый язык:

1. Скопировать `lang/en.json` в `lang/<code>.json` (например `lang/de.json`).
2. Перевести значения, сохраняя ключи неизменными и UTF-8 escape-последовательности (Switch glyphs) в `DYNAMIC_HEADER`/`STATIC_HEADER`/`HINT`.
3. Добавить `case SetLanguage_German: ovls::loadLanguage("de"); break;` в `source/main.cpp::initServices`.
4. Открыть PR.

Полный список ключей и текущих переводов — [`docs/RU/i18n.md`](docs/RU/i18n.md).

## Сборка

### Через Docker (Windows-friendly)

```powershell
.\scripts\build.ps1 -Dist
```

Скрипт сам подтянет `devkitpro/devkita64`, прокинет директорию и выдаст `dist/ovlSysmodules-1.5.1-*.zip` с SHA-256.

### Нативно

```bash
git clone --recursive https://github.com/Dimasick-git/ovlSysmodules.git
cd ovlSysmodules
make -j$(nproc)
```

`make` положит `ovlSysmodules.ovl` в корень. Подробнее — [`docs/RU/build.md`](docs/RU/build.md).

## Лицензия

GPL-2.0, наследуется от ppkantorski/ovl-sysmodules → WerWolv/ovl-sysmodules.

## Кредиты

- **WerWolv** — оригинальный ovl-sysmodules и `libtesla`.
- **ppkantorski** — `libultrahand`, форк ovl-sysmodules, поддержание актуального состояния.
- **Atmosphère NX team** (SciresM, TuxSH, hexkyz, fincs) — CFW.
- **devkitPro** — toolchain.
- **Dimasick-git (Ryazhenka)** — этот форк, libryazhahand swap, i18n, CI/CD.
