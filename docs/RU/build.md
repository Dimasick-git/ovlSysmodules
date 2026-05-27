# Сборка ovlSysmodules (Ryazhenka)

Три пути сборки:

| Метод | Кому | Что нужно установить |
|-------|------|----------------------|
| `scripts/build.ps1` | Windows, не хочется возиться с devkitPro | Docker Desktop |
| Нативно msys2 | Windows, частая разработка | msys2 + devkitPro pacman |
| Нативно Linux/WSL | Linux/WSL | devkitPro pacman |
| CI | Хочется публиковать релизы | ничего, всё в GitHub Actions |

## Метод 1. Docker на Windows

```powershell
git clone --recursive https://github.com/Dimasick-git/ovlSysmodules.git
cd ovlSysmodules
.\scripts\build.ps1 -Dist
```

- `-Dist` — собрать `.ovl` + упаковать в zip с структурой для SD-карты
  (`switch/.overlays/ovlSysmodules.ovl` + `config/ryazhahand/ovlSysmodules/lang/*.json`).
- `-Clean` — `make clean` перед сборкой.
- `-Tag v1.5.2` — поставить локальный git-тег (для перебивки APP_VERSION
  без коммита, если у тебя свой sysmodules-relative tag схема).

## Метод 2. Нативно msys2 (Windows)

См. подробности в [Mission-Control build.md](https://github.com/Dimasick-git/Mission-Control/blob/main/docs/RU/build.md) — процесс
идентичный (msys2 + dkp-pacman). Зависимости sysmodules:

```bash
pacman -S switch-dev
# libryazhahand build (если не используете submodule):
cd libs/libryazhahand && make && make install
```

Затем в корне проекта:

```bash
make -j$(nproc)
```

Результат: `ovlSysmodules.ovl` в корне.

## Метод 3. Linux / WSL2

```bash
sudo dkp-pacman -S switch-dev
git clone --recursive https://github.com/Dimasick-git/ovlSysmodules.git
cd ovlSysmodules
export DEVKITPRO=/opt/devkitpro
make -j$(nproc)
```

## Метод 4. CI (GitHub Actions)

| Триггер | Workflow | Что делает |
|---------|----------|------------|
| Push в `main` или PR | `build.yml` | Собирает `.ovl` через `devkitpro/devkita64` Docker, заливает артефакт |
| Push тега `v*.*.*` | `release.yml` | Собирает + публикует GitHub Release с zip + SHA-256 |
| Изменение `Makefile` в `main` | `release.yml` | То же что push тега, но тег создаётся автоматически из `APP_VERSION` |
| Ежедневно 03:00 UTC | `sync_upstream.yml` | `git fetch upstream/master`, открывает PR с защитой наших файлов |
| PR | `verify_build.yml` | Smoke-build без публикации |

## Структура релизного zip

```
ovlSysmodules-1.5.1-main-<hash>.zip
├── switch/
│   └── .overlays/
│       └── ovlSysmodules.ovl              ← собственно оверлей
└── config/
    └── ryazhahand/
        └── ovlSysmodules/
            └── lang/
                ├── en.json
                └── ru.json
```

Распаковка в корень SD-карты с перезаписью — установка готова.

## Проблемы

- **`devkitpro/devkita64: not found`** — проверь Docker запущен;
  `docker pull devkitpro/devkita64:latest`.
- **`fatal: not a git repository`** при `make` — Makefile использует
  `APP_VERSION` хардкоженно из себя, git не нужен; но если ошибка
  именно в сабмодуле libryazhahand — `git submodule update --init --recursive`.
- **`tesla.hpp: No such file or directory`** — submodule не подцеплен;
  `git submodule update --init`.
- **`undefined reference to ult::...`** — пересобери libryazhahand
  через его `ryazhahand.mk` (Makefile делает это автоматически).

## Проверка подписи готового .ovl

Последние 4 байта `ovlSysmodules.ovl` должны быть `RYZH` (`0x52 0x59 0x5A 0x48`):

```bash
tail -c 4 ovlSysmodules.ovl | xxd
# 00000000: 5259 5a48                                RYZH
```

Если видишь `5554 4c52` (ULTR) — собралось с неправильным Makefile;
проверь что `printf 'RYZH' >> $@` в правиле `$(OUTPUT).ovl`.
