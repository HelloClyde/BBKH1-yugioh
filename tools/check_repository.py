"""Check source closure, documentation links, dependency pin and public inventory."""
import re
from pathlib import Path
from common import ROOT, SDK_COMMIT, run, sdk_commit


def main():
    sdk_commit()
    if 'https://github.com/MrDefinition1999/bbk-h1-bda-sdk.git' not in (ROOT / '.gitmodules').read_text(encoding='utf-8'):
        raise ValueError('Unexpected SDK submodule origin')
    # No compiler/firmware/generated proprietary resource inputs are tracked.
    files = run(['git', '-c', f'safe.directory={ROOT.as_posix()}', '-C', ROOT, 'ls-files']).splitlines()
    prohibited = {'.exe', '.bda', '.elf', '.raw', '.sav', '.pak', '.bin', '.zip', '.ttf', '.ttc', '.otf'}
    for name in files:
        path = Path(name)
        if path.suffix.lower() in prohibited or name.startswith(('.local-assets/', '.tools/', 'build/', 'dist/')):
            raise ValueError('Private or generated data tracked in source repository: ' + name)
    public = [*(ROOT / 'src').glob('*'), *(ROOT / 'data').glob('*'), *(ROOT / 'tests').rglob('*')]
    search = [ROOT / x for x in ('src', 'data', 'tests', 'tests/oracles', 'sdk/sdk/include')]
    generated = {'duel_attack_assets.h'}
    for path in public:
        if not path.is_file():
            continue
        source = path.read_text(encoding='utf-8')
        for include in re.findall(r'^\s*#\s*include\s*"([^"]+)"', source, re.M):
            if include not in generated and not any((folder / include).is_file() for folder in search):
                raise FileNotFoundError(f'{path.name}: missing include {include}')
        if re.search(r'[A-Za-z]:[\\/](?:Users|ProgramData|Windows|msys64)', source):
            raise ValueError('Developer-machine path in source: ' + path.name)
    readme = (ROOT / 'README.md').read_text(encoding='utf-8')
    for section in ('快速开始', '截图', '依赖', '感谢'):
        if section not in readme:
            raise ValueError('README missing section: ' + section)
    for link in re.findall(r'!\[[^\]]*\]\(([^)]+)\)', readme):
        if link.startswith(('http:', 'https:')) or not (ROOT / link).is_file():
            raise ValueError('Screenshot must be present at a repository-relative path: ' + link)
    if (ROOT / '.git').exists():
        gitlink = run(['git', '-c', f'safe.directory={ROOT.as_posix()}', '-C', ROOT, 'ls-files', '--stage', 'sdk'])
        if gitlink and ('160000 ' + SDK_COMMIT) not in gitlink:
            raise ValueError('SDK index gitlink differs from the verified commit')
    print('Source closure, README screenshots, SDK pin and tracked inventory passed')


if __name__ == '__main__':
    main()
