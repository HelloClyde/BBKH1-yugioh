"""Verify release files against per-file hashes and ZIP payload identity."""
import argparse
import re
import zipfile
from pathlib import Path
from common import ROOT, read_json, sha


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('folder', type=Path, nargs='?', default=ROOT / 'dist')
    a = p.parse_args()
    listed = set()
    for line in (a.folder / 'SHA256SUMS.txt').read_text(encoding='utf-8').splitlines():
        match = re.fullmatch(r'([0-9a-f]{64})  ([A-Za-z0-9._-]+)', line)
        if not match or match[2] in listed:
            raise ValueError('Malformed or duplicated checksum entry')
        expected, name = match.groups()
        if sha(a.folder / name) != expected:
            raise ValueError('Release checksum mismatch: ' + name)
        listed.add(name)
    manifest = read_json(a.folder / 'build-manifest.json')
    expected_names = {'JOEY.bda', 'build-manifest.json', 'RELEASE_NOTES.md', f"BBKH1-yugioh-v{manifest['version']}-code-only.zip"}
    if listed != expected_names or sha(a.folder / 'JOEY.bda') != manifest['bda_sha256']:
        raise ValueError('Release inventory or BDA identity differs')
    if {p.name for p in a.folder.iterdir()} != expected_names | {'SHA256SUMS.txt'}:
        raise ValueError('Release folder contains unlisted files; use a fresh output directory')
    with zipfile.ZipFile(a.folder / f"BBKH1-yugioh-v{manifest['version']}-code-only.zip") as z:
        if set(z.namelist()) != {'应用/程序/游戏王.bda', '安装说明.txt', 'LICENSE', 'NOTICE', 'README.md', 'ASSET_SOURCES.md', 'build-manifest.json'}:
            raise ValueError('Unexpected release ZIP inventory')
        if z.read('应用/程序/游戏王.bda') != (a.folder / 'JOEY.bda').read_bytes():
            raise ValueError('Packaged BDA differs from standalone BDA')
    print('Verified all release hashes, ZIP inventory and BDA identity')


if __name__ == '__main__':
    main()
