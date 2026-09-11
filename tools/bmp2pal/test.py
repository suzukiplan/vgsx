#!/usr/bin/env python3
"""Check indexed-PNG palette extraction independently of pixel bit depth."""
from pathlib import Path
import struct
import subprocess
import tempfile
import zlib

TOOL = Path(__file__).resolve().parent / 'bmp2pal'


def chunk(kind, data):
    return struct.pack('>I', len(data)) + kind + data + struct.pack('>I', zlib.crc32(kind + data))


with tempfile.TemporaryDirectory() as temporary:
    root = Path(temporary)
    for bits in (1, 2, 4, 8, 16):
        image = b'\x89PNG\r\n\x1a\n'
        image += chunk(b'IHDR', struct.pack('>IIBBBBB', 1, 1, bits, 3, 0, 0, 0))
        image += chunk(b'PLTE', bytes.fromhex('123456abcdef'))
        image += chunk(b'IDAT', zlib.compress(b'\x00\x00'))
        image += chunk(b'IEND', b'')
        source = root / f'{bits}.png'
        output = root / f'{bits}.bin'
        source.write_bytes(image)
        result = subprocess.run([str(TOOL), str(source), str(output)], capture_output=True)
        if bits == 16:
            assert result.returncode != 0 and not output.exists()
        else:
            assert result.returncode == 0, result.stderr
            colors = struct.unpack('=256I', output.read_bytes())
            assert colors[:2] == (0x123456, 0xABCDEF) and not any(colors[2:])
    print('OK: indexed PNG palettes at 1/2/4/8 bits and invalid-depth rejection')
