"""Execute compiled native C on a MIPS32 CPU model (no H1 firmware or x86)."""
import argparse
import struct
import zlib
from pathlib import Path
from elftools.elf.elffile import ELFFile
from unicorn import Uc, UC_ARCH_MIPS, UC_MODE_MIPS32, UC_MODE_LITTLE_ENDIAN
from unicorn import mips_const as m
from common import FLAGS, ROOT, run, sha, source_hashes, toolchain, write_json


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--toolchain', type=Path)
    p.add_argument('--suite', choices=('profile', 'duel'), default='profile')
    p.add_argument('--out', type=Path, default=ROOT / 'build/mips-tests')
    a = p.parse_args()
    gcc, _, compiler = toolchain(a.toolchain)
    folder = a.out / a.suite
    folder.mkdir(parents=True, exist_ok=True)
    report_path = folder / 'tests.json'
    write_json(report_path, {'passed': False, 'reason': 'MIPS test in progress'})
    sources = [ROOT / 'src/duel.c', ROOT / 'src/duel_opening.c', ROOT / 'src/mini.c',
               ROOT / 'tests' / ('native_' + a.suite + '_test.c')]
    if a.suite == 'profile':
        sources.append(ROOT / 'src/duel_profile.c')
    includes = [part for name in ('src', 'data', 'tests', 'tests/oracles') for part in ('-I', ROOT / name)]
    objects = []
    for source in sources:
        obj = folder / (source.stem + '.o')
        run([gcc, *FLAGS, *includes, '-c', source, '-o', obj])
        objects.append(obj)
    entry = 'native_' + a.suite + '_test_run'
    script = folder / 'test.ld'
    script.write_text(f'ENTRY({entry})\nSECTIONS {{ . = 0x81000000; .text : {{ *(.text*) }} .rodata : {{ *(.rodata*) }} .data : {{ *(.data*) *(.sdata*) }} .bss : {{ *(.bss*) *(.sbss*) *(COMMON) }} /DISCARD/ : {{ *(.MIPS.abiflags) *(.reginfo) *(.eh_frame*) *(.comment) *(.note*) *(.pdr) }} }}\n', encoding='utf-8')
    target = folder / 'tests.elf'
    run([gcc, *FLAGS, '-nostdlib', '-Wl,--build-id=none', '-T', script, *objects, '-lgcc', '-o', target])
    cpu = Uc(UC_ARCH_MIPS, UC_MODE_MIPS32 | UC_MODE_LITTLE_ENDIAN)
    cpu.mem_map(0, 64 * 1024 * 1024)
    with target.open('rb') as f:
        elf = ELFFile(f)
        symbols = {s.name: s['st_value'] for s in elf.get_section_by_name('.symtab').iter_symbols() if s.name}
        for section in elf.iter_sections():
            if section['sh_flags'] & 2 and section['sh_type'] != 'SHT_NOBITS':
                cpu.mem_write(section['sh_addr'] & 0x1fffffff, section.data())
    cpu.reg_write(m.UC_MIPS_REG_SP, 0x803ffff0)
    cpu.reg_write(m.UC_MIPS_REG_RA, 0x80001ff0)
    budget = 100_000_000 if a.suite == 'profile' else 40_000_000_000
    print(f'Executing native MIPS {a.suite} suite (budget {budget} instructions)', flush=True)
    cpu.emu_start(symbols[entry], 0x80001ff0, count=budget)
    if cpu.reg_read(m.UC_MIPS_REG_PC) != 0x80001ff0:
        raise ValueError('MIPS suite exhausted its instruction budget')
    checks, failures = struct.unpack('<2I', cpu.mem_read(symbols['native_' + a.suite + '_test_result'] & 0x1fffffff, 8))
    if failures or checks != (4691 if a.suite == 'profile' else 1454892):
        raise ValueError(f'MIPS tests failed or incomplete: {checks} checks, {failures} failures')
    crc_verified = False
    if a.suite == 'profile':
        at = symbols['wire'] & 0x1fffffff
        size = struct.unpack_from('<I', cpu.mem_read(at, 24), 16)[0]
        if size not in (1333, 1345, 1377):
            raise ValueError('Unexpected synthetic save envelope size')
        wire = bytearray(cpu.mem_read(at, size))
        crc = struct.unpack_from('<I', wire, 20)[0]
        wire[20:24] = bytes(4)
        if zlib.crc32(wire) != crc:
            raise ValueError('Native profile CRC differs from independent CRC32')
        crc_verified = True
    write_json(report_path, {'passed': True, 'suite': a.suite, 'checks': checks, 'failures': failures,
                            'elf_sha256': sha(target), 'compiler': compiler, 'source_hashes': source_hashes(),
                            'execution': 'Compiled native C on Unicorn MIPS32; not a firmware test',
                            'original_executable_loaded': False, 'physical_h1_tested': False,
                            'independent_crc32_verified': crc_verified})
    print(f'MIPS {a.suite}: {checks} checks, 0 failures')


if __name__ == '__main__':
    main()
