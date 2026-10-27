#!/usr/bin/env python3
"""NACP оверлея (.ovl/.nro): показать имя и автора; с --fix переписать их в UTF-8.

На этом ПК make из msys передаёт APP_TITLE в nacptool.exe через кодовую страницу Windows (cp1251): кириллическое
имя ложится в NACP байтами cp1251, и меню Ryzhand его не показывает. --fix перекодирует cp1251 -> UTF-8 на месте во
всех языковых записях (размер файла не меняется).
"""
import os
import struct
import sys

LANGS = 16
ENTRY = 0x300


def nacp_range(data):
    size = struct.unpack_from('<I', data, 0x18)[0]
    if data[size:size + 4] != b'ASET':
        raise ValueError('no asset section')
    off, length = struct.unpack_from('<QQ', data, size + 0x18)
    return size + off, length


def field(buf, start, length):
    return bytes(buf[start:start + length]).split(b'\0')[0]


def main():
    fix = '--fix' in sys.argv
    for path in [a for a in sys.argv[1:] if a != '--fix']:
        data = bytearray(open(path, 'rb').read())
        base, _ = nacp_range(data)
        changed = False
        for lang in range(LANGS):
            for off, length in ((0, 0x200), (0x200, 0x100)):
                start = base + lang * ENTRY + off
                raw = field(data, start, length)
                try:
                    raw.decode('utf-8')
                    continue
                except UnicodeDecodeError:
                    pass
                if not fix:
                    continue
                text = raw.decode('cp1251').encode('utf-8')[:length - 1]
                data[start:start + length] = text + b'\0' * (length - len(text))
                changed = True
        name = field(data, base, 0x200)
        author = field(data, base + 0x200, 0x100)
        try:
            state = 'ok'
            shown = name.decode('utf-8') + ' | ' + author.decode('utf-8')
        except UnicodeDecodeError:
            state = 'NOT UTF-8'
            shown = name.decode('cp1251', 'replace') + ' | ' + author.decode('cp1251', 'replace')
        if changed:
            open(path, 'wb').write(data)
            state = 'fixed'
        print(f'{os.path.basename(path)}: {state}: {shown}')


if __name__ == '__main__':
    main()
