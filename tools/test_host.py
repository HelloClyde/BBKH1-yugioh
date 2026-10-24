"""Compile the production C rules and offline numeric fixtures for host GCC."""
import argparse
import json
import os
import time
from pathlib import Path
from common import ROOT, host_gcc, run, source_hashes, write_json


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--cc', help='Host GCC executable, or HOST_CC')
    p.add_argument('--out', type=Path, default=ROOT / 'build/host')
    p.add_argument('--sanitize', action='store_true', help='Enable undefined-behavior sanitizer')
    a = p.parse_args()
    gcc, env = host_gcc(a.cc)
    a.out.mkdir(parents=True, exist_ok=True)
    report_path = a.out / 'tests.json'
    write_json(report_path, {'passed': False, 'reason': 'Host build/test in progress'})
    results = {}
    for suite in ('duel', 'profile'):
        symbol = f'native_{suite}_test'
        harness = a.out / (suite + '-main.c')
        harness.write_text('#include <stdio.h>\n#include <stdint.h>\n'
            f'extern int {symbol}_run(void);\nextern volatile uint32_t {symbol}_result[];\n'
            f'int main(void){{int result={symbol}_run();printf("{{\\\"checks\\\":%u,\\\"failures\\\":%u}}\\n",{symbol}_result[0],{symbol}_result[1]);return result?1:0;}}\n', encoding='utf-8')
        sources = [ROOT / 'src/duel.c', ROOT / 'src/duel_opening.c']
        if suite == 'profile':
            sources.append(ROOT / 'src/duel_profile.c')
        sources.extend((ROOT / 'tests' / ('native_' + suite + '_test.c'), harness))
        exe = a.out / (suite + ('-tests.exe' if os.name == 'nt' else '-tests'))
        flags = ['-pipe', '-O2', '-std=c11', '-fno-strict-aliasing']
        if a.sanitize:
            flags.extend(('-fsanitize=undefined', '-fno-sanitize-recover=all'))
        includes = ['-I' + str(ROOT / n) for n in ('src', 'data', 'tests', 'tests/oracles')]
        start = time.perf_counter()
        run([gcc, *flags, *includes, *sources, '-o', exe], env=env)
        compiled = time.perf_counter() - start
        start = time.perf_counter()
        result = json.loads(run([exe.resolve()], env=env, timeout=60))
        result.update(compile_seconds=compiled, execution_seconds=time.perf_counter() - start)
        results[suite] = result
        if result['failures'] or result['checks'] == 0:
            raise ValueError(f'{suite} failed: {result}')
        print(f"{suite}: {result['checks']} checks, {result['failures']} failures, {result['execution_seconds']:.3f}s", flush=True)
    if results['duel']['checks'] != 1454892 or results['profile']['checks'] != 4691:
        raise ValueError('Full suite check count differs; investigate before updating the expected inventory')
    write_json(report_path, {'passed': True, 'full_suite': True, 'suites': results, 'compiler': run([gcc, '--version'], env=env).splitlines()[0],
                            'execution': 'Host GCC executing the production native C and offline numeric fixtures',
                            'original_executable_loaded': False, 'mips_tested': False, 'physical_h1_tested': False,
                            'source_hashes': source_hashes(), 'undefined_behavior_sanitizer': a.sanitize})


if __name__ == '__main__':
    main()
