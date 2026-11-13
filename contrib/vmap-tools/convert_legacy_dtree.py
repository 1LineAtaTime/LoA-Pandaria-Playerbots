from pathlib import Path
import math
import shutil
import struct

root = Path(__file__).resolve().parents[2]
source = root / 'exports/vmaps/GameObjectModels.dtree'
backup = root / 'out/compat-test/GameObjectModels.dtree.legacy'
data = source.read_bytes()
if data.startswith(b'VMAP_4.3'):
    raise SystemExit('Already in current VMAP format')
if backup.exists():
    raise SystemExit('Refusing to overwrite existing backup')

position = 0
records = []
while position < len(data):
    if position + 8 > len(data):
        raise ValueError('Truncated legacy record header')
    display_id, name_length = struct.unpack_from('<II', data, position)
    position += 8
    if not display_id or not 1 <= name_length < 500 or position + name_length + 24 > len(data):
        raise ValueError('Invalid legacy record')
    name = data[position:position + name_length]
    position += name_length
    vectors = data[position:position + 24]
    position += 24
    if not all(math.isfinite(value) for value in struct.unpack('<6f', vectors)):
        raise ValueError('Invalid model bounds')
    lower = name.lower()
    if lower.endswith(b'.m2'):
        is_wmo = 0
    elif lower.endswith(b'.wmo'):
        is_wmo = 1
    else:
        raise ValueError('Unknown model extension')
    records.append((display_id, is_wmo, name, vectors))

if len(records) < 1000 or position != len(data):
    raise ValueError('Unexpected record count or trailing bytes')

output = bytearray(b'VMAP_4.3')
for display_id, is_wmo, name, vectors in records:
    output += struct.pack('<IBI', display_id, is_wmo, len(name))
    output += name
    output += vectors

# Parse the converted format before touching the staged asset.
position = 8
for display_id, is_wmo, name, vectors in records:
    actual_id, actual_wmo, actual_length = struct.unpack_from('<IBI', output, position)
    position += 9
    if (actual_id, actual_wmo, actual_length) != (display_id, is_wmo, len(name)):
        raise ValueError('Converted header mismatch')
    if output[position:position + len(name)] != name:
        raise ValueError('Converted model name mismatch')
    position += len(name)
    if output[position:position + 24] != vectors:
        raise ValueError('Converted bounds mismatch')
    position += 24
if position != len(output):
    raise ValueError('Converted length mismatch')

shutil.copy2(source, backup)
temporary = source.with_suffix('.dtree.new')
temporary.write_bytes(output)
temporary.replace(source)
print(f'Converted {len(records)} models; original saved at {backup}')
