"""Repository-relative utilities shared by build and verification tools."""
from __future__ import annotations

import hashlib
import json
import os
import shutil
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SDK_COMMIT = '067fe072477861dfc8949d7b1a55279fb92d2548'
ASSET_NAMES = ('FONT.BIN', 'H1UI.PAK', 'H1ART.PAK', 'H1SND.PAK', 'H1SCN.PAK')
FLAGS = ['-march=mips32', '-mabi=32', '-EL', '-msoft-float', '-mno-abicalls',
         '-fno-pic', '-G0', '-ffreestanding', '-fno-builtin', '-fno-stack-protector',
         '-O2', '-std=c11', '-ffunction-sections', '-fdata-sections']


def sha(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def read_json(path: Path):
    return json.loads(path.read_text(encoding='utf-8'))


def write_json(path: Path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')


def run(args, *, cwd=ROOT, env=None, timeout=None):
    result = subprocess.run([str(x) for x in args], cwd=cwd, env=env,
                            capture_output=True, text=True, encoding='utf-8', errors='replace', timeout=timeout)
    if result.returncode:
        raise RuntimeError(f'Command failed ({result.returncode}): {args[0]}\n{result.stdout}{result.stderr}')
    return result.stdout.strip()


def toolchain(path=None):
    folder = Path(path or os.environ.get('H1_GNU_BIN', ROOT / '.tools/toolchain/bin')).resolve()
    suffix = '.exe' if os.name == 'nt' else ''
    gcc, objcopy = [folder / ('mipsel-none-elf-' + n + suffix) for n in ('gcc', 'objcopy')]
    if not gcc.is_file() or not objcopy.is_file():
        raise FileNotFoundError(f'MIPS tools missing in {folder}; see tools/install_toolchain.py --help')
    version = run([gcc, '--version']).splitlines()[0]
    if '15.2.0' not in version:
        raise ValueError('Reproducible release requires MIPS GCC 15.2.0: ' + version)
    return gcc, objcopy, version


def host_gcc(value=None):
    name = value or os.environ.get('HOST_CC', 'gcc')
    found = shutil.which(name)
    if not found:
        raise FileNotFoundError('Host GCC not found; set HOST_CC or --cc to a GCC executable')
    gcc = Path(found).resolve()
    env = dict(os.environ)
    env['PATH'] = str(gcc.parent) + os.pathsep + env.get('PATH', '')
    return gcc, env


def source_hashes():
    paths = [*(ROOT / 'src').glob('*'), *(ROOT / 'data').glob('*'), *(ROOT / 'tests').rglob('*.c'),
             *(ROOT / 'tests').rglob('*.h'), *(ROOT / 'tests').rglob('*.inc'), *(ROOT / 'tools').glob('*.py')]
    return {p.relative_to(ROOT).as_posix(): sha(p) for p in sorted(paths) if p.is_file()}


def sdk_commit():
    # Explicit per-invocation trust also works in a shared sandbox checkout;
    # never change the user's global safe.directory configuration.
    sdk = ROOT / 'sdk'
    actual = run(['git', '-c', f'safe.directory={sdk.as_posix()}', '-C', sdk, 'rev-parse', 'HEAD'])
    if actual != SDK_COMMIT:
        raise ValueError(f'SDK commit {actual} differs from pinned {SDK_COMMIT}')
    if run(['git', '-c', f'safe.directory={sdk.as_posix()}', '-C', sdk, 'status', '--porcelain']):
        raise ValueError('SDK has local changes; reproducible release requires the clean pinned SDK')
    return actual
