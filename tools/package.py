"""Make a reproducible, explicitly enumerated code-only release package."""
import argparse
import hashlib
import re
import shutil
import zipfile
from pathlib import Path
from common import ROOT, read_json, sha, write_json

INSTALL = '''游戏王 H1 原生版 — 源码构建包

适用：步步高 H1 / Y100 V1.41，480×272。
本包只含程序，不含原游戏图像、汉化文本、音乐或字体。
需要自行提供兼容的 FONT.BIN、H1UI.PAK、H1ART.PAK、H1SND.PAK、H1SCN.PAK。
将这五个文件放入 B:\\JOEY\\，并将本包的 JOEY.bda 复制到
A:\\应用\\程序\\游戏王.bda，然后在游戏分类中启动“游戏王”。
没有 B: 时资源也可放 A:\\JOEY\\。请保留原有 JNAT0.SAV / JNAT1.SAV。
方向键移动，确认选择，ESC 返回；决斗中触摸左侧阶段图标推进阶段/结束回合。
SHIFT+ESC 正常退出；默认游戏音量 10%。
完整说明与资源来源见 README.md / ASSET_SOURCES.md。
'''


def add(z, name, data):
    info = zipfile.ZipInfo(name, date_time=(2026, 1, 1, 0, 0, 0))
    info.compress_type = zipfile.ZIP_DEFLATED
    info.external_attr = 0o100644 << 16
    z.writestr(info, data)


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--build', type=Path, default=ROOT / 'build/native')
    p.add_argument('--out', type=Path, default=ROOT / 'dist')
    a = p.parse_args()
    report = read_json(a.build / 'build.json')
    if not report.get('passed') or report['visuals'] != 'open' or report['external_resources_included']:
        raise ValueError('Public release packaging requires an open-visuals, code-only build')
    if sha(a.build / 'JOEY.bda') != report['bda_sha256']:
        raise ValueError('BDA identity differs from successful build')
    version = (ROOT / 'VERSION').read_text(encoding='utf-8').strip()
    if not re.fullmatch(r'\d+\.\d+\.\d+', version) or version != report['version']:
        raise ValueError('Version mismatch')
    a.out.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(a.build / 'JOEY.bda', a.out / 'JOEY.bda')
    # Keep absolute paths, compiler timings and private inputs out of packages.
    manifest = {k: report[k] for k in ('version', 'title', 'sdk_commit', 'compiler', 'bda_sha256', 'bda_bytes',
                'visuals', 'external_resources_included', 'native_execution', 'entry', 'source_hashes')}
    write_json(a.out / 'build-manifest.json', manifest)
    notes = ('游戏王 H1 原生版 v' + version + '\n\n'
             '原生 MIPS C；默认音量10%；完整卡牌效果实现。\n'
             '此预发布只分发程序，运行所需的第三方资源包需由使用者自行提供。\n'
             'CI 运行主机完整效果回归、MIPS 存档回归、BDA 静态校验及两次构建一致性检查；'
             'CI 不运行完整 H1 固件，历史完整固件记录的适用哈希见 docs/VALIDATION.md。\n'
             '程序 SHA-256：' + report['bda_sha256'] + '\n')
    (a.out / 'RELEASE_NOTES.md').write_text(notes, encoding='utf-8')
    archive = a.out / ('BBKH1-yugioh-v' + version + '-code-only.zip')
    with zipfile.ZipFile(archive, 'w', compression=zipfile.ZIP_DEFLATED, compresslevel=9) as z:
        add(z, 'JOEY/JOEY.bda', (a.build / 'JOEY.bda').read_bytes())
        add(z, '安装说明.txt', INSTALL.encode('utf-8'))
        for name in ('LICENSE', 'NOTICE', 'README.md', 'ASSET_SOURCES.md'):
            add(z, name, (ROOT / name).read_bytes())
        add(z, 'build-manifest.json', (a.out / 'build-manifest.json').read_bytes())
    names = ('JOEY.bda', archive.name, 'build-manifest.json', 'RELEASE_NOTES.md')
    (a.out / 'SHA256SUMS.txt').write_text(''.join(sha(a.out / n) + '  ' + n + '\n' for n in names), encoding='utf-8')
    print('Prepared code-only release: ' + archive.name)


if __name__ == '__main__':
    main()
