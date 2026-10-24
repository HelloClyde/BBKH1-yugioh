"""Run the same source-only verification and packaging steps locally and in CI."""
import argparse
import sys
from pathlib import Path
from common import ROOT, read_json, run, sha, write_json


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--cc', help='Host GCC executable')
    p.add_argument('--toolchain', type=Path)
    a = p.parse_args()
    def call(name, *args):
        output = run([sys.executable, ROOT / 'tools' / name, *args])
        if output:
            print(output, flush=True)
    tc = ['--toolchain', a.toolchain.resolve()] if a.toolchain else []
    call('check_repository.py')
    call('test_resource_paths.py', *(['--cc', a.cc] if a.cc else []))
    call('test_host.py', *(['--cc', a.cc] if a.cc else []))
    call('test_mips.py', *tc)
    call('build.py', *tc, '--out', ROOT / 'build/native')
    call('build.py', *tc, '--out', ROOT / 'build/repeat')
    one, two = ROOT / 'build/native', ROOT / 'build/repeat'
    for name in ('JOEY.bda', 'JOEY.elf'):
        if sha(one / name) != sha(two / name):
            raise ValueError('Repeated build differs: ' + name)
    call('package.py')
    call('verify_dist.py')
    write_json(ROOT / 'build/ci-verification.json', {
        'passed': True, 'host': read_json(ROOT / 'build/host/tests.json')['suites'],
        'mips_profile_checks': read_json(ROOT / 'build/mips-tests/profile/tests.json')['checks'],
        'resource_path_checks': read_json(ROOT / 'build/path-tests/tests.json')['checks'],
        'bda_sha256': sha(one / 'JOEY.bda'), 'elf_sha256': sha(one / 'JOEY.elf'),
        'repeat_bda_and_elf_identical': True, 'package_hashes_verified': True,
        'sdk_commit': read_json(one / 'build.json')['sdk_commit'],
        'firmware_run_in_ci': False, 'physical_h1_tested': False})
    print('Source-only CI checks passed; firmware verification is separately documented')


if __name__ == '__main__':
    main()
