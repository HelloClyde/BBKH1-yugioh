"""Check production C path encoding and A/B fallback against an independent GBK reference."""
import argparse
import json
from pathlib import Path
from common import ASSET_NAMES, ROOT, host_gcc, run, write_json


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--cc')
    p.add_argument('--out', type=Path, default=ROOT / 'build/path-tests')
    a = p.parse_args()
    a.out.mkdir(parents=True, exist_ok=True)
    gcc, env = host_gcc(a.cc)
    source = a.out / 'path-test.c'
    source.write_text(r'''
#include <stdio.h>
#include <stdlib.h>
typedef struct {unsigned dummy;} h1_file;
static h1_file fake_file;
static unsigned available;
static h1_file *h1_fopen(const char *path,const char *mode){
    printf("{\"open_hex\":\"");
    for(const unsigned char *p=(const unsigned char *)path;*p;++p)printf("%02x",*p);
    printf("\",\"read_mode\":%u}\n",mode[0]=='r'&&mode[1]=='b'&&!mode[2]);
    return available&(path[0]=='A'?1u:path[0]=='B'?2u:0u)?&fake_file:0;
}
#include "duel_paths.h"
int main(int argc,char **argv){
    available=argc>1?atoi(argv[1]):0;
    h1_file *files[5];unsigned drives[5]={99,99,99,99,99};
    files[0]=JOEY_OPEN_RESOURCE("FONT.BIN",&drives[0]);
    files[1]=JOEY_OPEN_RESOURCE("H1UI.PAK",&drives[1]);
    files[2]=JOEY_OPEN_RESOURCE("H1ART.PAK",&drives[2]);
    files[3]=JOEY_OPEN_RESOURCE("H1SND.PAK",&drives[3]);
    files[4]=JOEY_OPEN_RESOURCE("H1SCN.PAK",&drives[4]);
    for(unsigned i=0;i<5;++i)printf("{\"found\":%u,\"drive\":%u}\n",files[i]!=0,drives[i]);
    return 0;
}
''', encoding='utf-8')
    executable = a.out / 'resource-path-tests.exe'
    run([gcc, '-std=c11', '-O2', '-I', ROOT / 'src', source, '-o', executable], env=env)
    checks = 0
    for mask in (1, 2, 3, 0):
        output = [json.loads(line) for line in run([executable.resolve(), str(mask)], env=env).splitlines()]
        calls = [item for item in output if 'open_hex' in item]
        results = [item for item in output if 'found' in item]
        expected = []
        for name in ASSET_NAMES:
            expected.append(('A:\\应用\\数据\\yugioh\\' + name).encode('gbk').hex())
            if not mask & 1:
                expected.append(('B:\\应用\\数据\\yugioh\\' + name).encode('gbk').hex())
        assert [item['open_hex'] for item in calls] == expected, mask
        assert all(item['read_mode'] for item in calls)
        assert len(results) == 5
        assert all(item == {'found': int(bool(mask)), 'drive': 0 if mask & 1 else 1 if mask & 2 else 99} for item in results), mask
        checks += len(calls) + len(results)
    report = {'passed': True, 'checks': checks, 'resource_files': list(ASSET_NAMES),
              'gbk_encoded_paths_verified': True, 'a_only': True, 'b_only': True,
              'a_preferred_when_both_present': True, 'both_absent': True,
              'execution': 'Production C resource opener on host; filesystem presence is mocked.',
              'physical_h1_tested': False}
    write_json(a.out / 'tests.json', report)
    print(f'Resource paths: {checks} checks passed (GBK, A, B, A priority, missing files)')


if __name__ == '__main__':
    main()
