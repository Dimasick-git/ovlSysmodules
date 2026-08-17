#!/usr/bin/env python3
"""Проверяет ограничение длины русских экранных строк ovlSysmodules."""

from __future__ import annotations

import json
import sys
from pathlib import Path

MAX_EXTRA_CHARACTERS = 3

# Исходные строки актуальной базы ppkantorski. Иконки Switch не учитываются.
SOURCE_ENGLISH = {
    "OVERLAY_TITLE": "Sysmodules",
    "ON_LABEL": "On",
    "OFF_LABEL": "Off",
    "NO_SYSMODULES_FOUND": "No sysmodules found!",
    "SCAN_FAILED": "Scan failed!",
    "DYNAMIC_HEADER": "Dynamic   Auto Start   Toggle",
    "DYNAMIC_HINT": " These sysmodules can be toggled at any time.",
    "STATIC_HEADER": "Static   Auto Start",
    "STATIC_HINT": " These sysmodules need a reboot to work.",
    "SHUTDOWN_IPC_FAILED": "Shutdown IPC has failed.",
    "RAM_LABEL": "System RAM",
    "FREE_LABEL": "free",
}


def visible_length(value: str) -> int:
    """Возвращает длину текста без служебных значков шрифта Switch."""
    return sum(not (0xE000 <= ord(char) <= 0xF8FF) for char in value)


def main() -> int:
    translation_path = Path(__file__).resolve().parents[1] / "lang" / "ru.json"
    translations = json.loads(translation_path.read_text(encoding="utf-8"))
    failures: list[str] = []

    for key, original in SOURCE_ENGLISH.items():
        translated = translations.get(key)
        if not isinstance(translated, str):
            failures.append(f"{key}: отсутствует русский перевод")
            continue

        original_length = visible_length(original)
        translated_length = visible_length(translated)
        allowed_length = original_length + MAX_EXTRA_CHARACTERS
        if translated_length > allowed_length:
            failures.append(
                f"{key}: {translated_length} символов вместо допустимых {allowed_length} "
                f"(исходная строка: {original_length})"
            )
        else:
            print(f"OK  {key}: {translated_length}/{allowed_length}")

    if failures:
        print("Ошибка ограничения длины:", file=sys.stderr)
        for failure in failures:
            print(f"  - {failure}", file=sys.stderr)
        return 1

    print("Все русские строки соответствуют ограничению длины.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
