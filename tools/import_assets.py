"""Import an explicitly supplied, locally prepared JOEY folder; never downloads a game."""
import argparse
import hashlib
import shutil
from pathlib import Path
from common import ASSET_NAMES, ROOT, read_json, sha, write_json
from make_open_assets import attack_header


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('source', type=Path, help='Folder containing five BIN/PAK files, optionally the verified JOEY.bda')
    p.add_argument('--out', type=Path, default=ROOT / '.local-assets')
    a = p.parse_args()
    source, out = a.source.resolve(), a.out.resolve()
    reference = read_json(ROOT / 'assets/reference-manifest.json')
    # Validate the entire input set before changing any local file.
    for name in ASSET_NAMES:
        src = source / name
        if not src.is_file() or sha(src) != reference['files'][name]['sha256']:
            raise ValueError(f'{name}: expected the compatible prepared resource pack described in assets/reference-manifest.json')
    bda = source / 'JOEY.bda'
    reference_visuals = bda.is_file() and sha(bda) == reference['bda_sha256']
    out.mkdir(parents=True, exist_ok=True)
    for name in ASSET_NAMES:
        if (source / name) != (out / name):
            shutil.copyfile(source / name, out / name)
    if reference_visuals:
        raw = bda.read_bytes()
        regions = {}
        for name in ('icon_region', 'attack_region'):
            region = reference[name]
            content = raw[region['offset']:region['offset'] + region['bytes']]
            if hashlib.sha256(content).hexdigest() != region['sha256']:
                raise ValueError('Reference embedded asset hash mismatch: ' + name)
            regions[name] = content
        (out / 'icon-resources.bin').write_bytes(regions['icon_region'])
        attack_header(regions['attack_region'], out / 'duel_attack_assets.h')
    write_json(out / 'import.json', {'files': reference['files'], 'reference_visuals': reference_visuals,
                                    'private_local_inputs': True, 'permission_to_redistribute': False})
    print('Imported five local resource packs; reference embedded visuals: ' + str(reference_visuals))


if __name__ == '__main__':
    main()
