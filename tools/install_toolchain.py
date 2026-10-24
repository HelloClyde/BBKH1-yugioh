"""Install the exact Windows MIPS GCC release used to verify this application."""
import argparse
import hashlib
import os
import shutil
import tempfile
import urllib.request
import zipfile
from pathlib import Path, PurePosixPath
from common import ROOT, run

URL = 'https://static.grumpycoder.net/pixel/mips/g++-mipsel-none-elf-15.2.0.zip'
SHA256 = '8ba866e25c9826ee04ab4310365d264e3e73769e3738bb58ae38fd6740b7ee8d'


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--archive', type=Path, help='Use an existing downloaded archive, still checked by SHA256')
    p.add_argument('--out', type=Path, default=ROOT / '.tools/toolchain')
    a = p.parse_args()
    if os.name != 'nt':
        raise SystemExit('The pinned binary toolchain is for Windows; native MIPS GCC 15.2.0 may be supplied separately on other hosts')
    archive = a.archive or ROOT / '.tools/g++-mipsel-none-elf-15.2.0.zip'
    if a.archive and not archive.is_file():
        raise FileNotFoundError('--archive must name an existing downloaded toolchain ZIP')
    if not a.archive:
        archive.parent.mkdir(parents=True, exist_ok=True)
    if not archive.is_file():
        temporary = archive.with_suffix('.download')
        version = (ROOT / 'VERSION').read_text(encoding='utf-8').strip()
        # The upstream distributor rejects urllib's generic Python user agent.
        # Identify this project while keeping the exact URL and digest pinned.
        request = urllib.request.Request(URL, headers={
            'User-Agent': f'BBKH1-yugioh/{version} (+https://github.com/HelloClyde/BBKH1-yugioh)',
            'Accept': 'application/octet-stream',
        })
        with urllib.request.urlopen(request, timeout=60) as response, temporary.open('wb') as f:
            shutil.copyfileobj(response, f)
        if hashlib.sha256(temporary.read_bytes()).hexdigest() != SHA256:
            raise ValueError('Downloaded toolchain checksum mismatch; untrusted archive was not extracted')
        temporary.replace(archive)
    if hashlib.sha256(archive.read_bytes()).hexdigest() != SHA256:
        raise ValueError('Toolchain archive checksum mismatch')
    a.out.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(dir=a.out.parent, prefix='h1-toolchain-') as folder:
        stage = Path(folder)
        with zipfile.ZipFile(archive) as z:
            for member in z.infolist():
                path = PurePosixPath(member.filename.replace('\\', '/'))
                if path.is_absolute() or '..' in path.parts or ':' in member.filename:
                    raise ValueError('Unsafe toolchain ZIP member: ' + member.filename)
                if (member.external_attr >> 16) & 0o170000 == 0o120000:
                    raise ValueError('Toolchain ZIP must not contain symlinks')
            z.extractall(stage)
        candidates = list(stage.rglob('bin/mipsel-none-elf-gcc.exe'))
        if len(candidates) != 1:
            raise ValueError('Unexpected archive layout')
        source = candidates[0].parent.parent
        version = run([candidates[0], '--version']).splitlines()[0]
        if '15.2.0' not in version:
            raise ValueError('Unexpected GCC version: ' + version)
        shutil.copytree(source, a.out, dirs_exist_ok=True)
    print('Verified MIPS GCC 15.2.0 installed in ' + str(a.out.resolve()))


if __name__ == '__main__':
    main()
