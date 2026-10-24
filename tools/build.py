"""Build a native H1 BDA from the pinned SDK and repository-local sources."""
from __future__ import annotations
import argparse
import shutil
import struct
import sys
from pathlib import Path
from elftools.elf.elffile import ELFFile
from common import ASSET_NAMES, FLAGS, ROOT, read_json, run, sdk_commit, sha, source_hashes, toolchain, write_json
from make_open_assets import generate


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--toolchain', type=Path, help='Directory containing mipsel-none-elf-gcc/objcopy; alternatively H1_GNU_BIN')
    p.add_argument('--out', type=Path, default=ROOT / 'build/native')
    p.add_argument('--assets', type=Path, help='Locally imported, private BIN/PAK folder; omitted for a code-only build')
    p.add_argument('--visuals', choices=('open', 'reference'), default='open')
    a = p.parse_args()
    out = a.out.resolve()
    out.mkdir(parents=True, exist_ok=True)
    version = (ROOT / 'VERSION').read_text(encoding='utf-8').strip()
    write_json(out / 'build.json', {'passed': False, 'reason': 'Build in progress'})
    sdk = sdk_commit()
    inputs = source_hashes()
    gcc, objcopy, compiler = toolchain(a.toolchain)
    generated = out / 'generated'
    generate(generated)
    sys.path.insert(0, str(ROOT / 'sdk'))
    from h1_bda.header import HeaderFields, encode_header
    from h1_bda.resources import PAYLOAD_OFFSET, RESOURCE_OFFSET, RESOURCE_SIZES, build_icon_resources
    from h1_bda.validate import validate_bda
    reference = read_json(ROOT / 'assets/reference-manifest.json')
    if a.assets:
        for name in ASSET_NAMES:
            if sha(a.assets / name) != reference['files'][name]['sha256']:
                raise ValueError('Incompatible or changed local resource: ' + name)
    if a.visuals == 'reference':
        if not a.assets:
            raise ValueError('--visuals reference requires locally imported --assets')
        icon_resources = (a.assets / 'icon-resources.bin').read_bytes()
        import hashlib
        if hashlib.sha256(icon_resources).hexdigest() != reference['icon_region']['sha256']:
            raise ValueError('Private reference desktop icon bytes differ')
        shutil.copyfile(a.assets / 'duel_attack_assets.h', generated / 'duel_attack_assets.h')
    else:
        icon_resources = build_icon_resources(generated / 'icon.png')
    names = ('duel.c', 'duel_profile.c', 'duel_opening.c', 'duel_h1.c', 'duel_audio.c', 'duel_ima.c', 'mini.c', 'h1_entry.S')
    includes = [part for folder in (generated, ROOT / 'data', ROOT / 'src', ROOT / 'sdk/sdk/include') for part in ('-I', folder)]
    objects = []
    for name in names:
        obj = out / (Path(name).stem + '.o')
        run([gcc, *FLAGS, *includes, '-c', ROOT / 'src' / name, '-o', obj])
        objects.append(obj)
    elf_path = out / 'JOEY.elf'
    run([gcc, *FLAGS, '-nostdlib', '-Wl,--build-id=none,--gc-sections,--undefined=joey_duel_layout,--undefined=joey_profile_layout',
         '-T', ROOT / 'src/h1.ld', *objects, '-lgcc', '-o', elf_path])
    with elf_path.open('rb') as f:
        elf = ELFFile(f)
        symbols = {s.name: s for s in elf.get_section_by_name('.symtab').iter_symbols() if s.name}
        forbidden = {'joey_x86_run', 'joey_aot_run', 'joey_host_lookup', 'joey_win32_run', 'execute_instruction'} & symbols.keys()
        if forbidden:
            raise ValueError('Unexpected legacy execution symbols: ' + str(forbidden))
        end = symbols['__bss_end']['st_value']
        if end > 0x83e00000 or elf.header['e_entry'] != 0x83c00020:
            raise ValueError('Native entry or application memory range differs')
        code_bytes = sum(s['sh_size'] for s in elf.iter_sections() if s['sh_flags'] & 4)
        for s in elf.iter_sections():
            if s['sh_flags'] & 4:
                for word, in struct.iter_unpack('<I', s.data()):
                    if word >> 26 in (0x11, 0x13, 0x31, 0x35, 0x39, 0x3d):
                        raise ValueError('Hardware floating point instruction linked')
        layouts = {}
        for name in ('joey_duel_layout', 'joey_profile_layout'):
            symbol = symbols[name]
            section = elf.get_section(symbol['st_shndx'])
            at = symbol['st_value'] - section['sh_addr']
            layouts[name] = list(struct.unpack('<' + 'I' * (symbol['st_size'] // 4), section.data()[at:at + symbol['st_size']]))
    raw = out / 'JOEY.bin'
    run([objcopy, '-O', 'binary', elf_path, raw])
    payload = raw.read_bytes()
    padding = (-(PAYLOAD_OFFSET + len(payload))) & 3
    fields = HeaderFields(0x48, PAYLOAD_OFFSET + len(payload) + padding - 4, PAYLOAD_OFFSET, RESOURCE_OFFSET, RESOURCE_SIZES)
    bda = out / 'JOEY.bda'
    bda.write_bytes(encode_header(fields, title='游戏王', description='城之内原生重写') + icon_resources + payload + bytes(padding))
    validation = validate_bda(bda)
    if not validation['ok']:
        raise ValueError(validation)
    validation.pop('path', None)
    write_json(out / 'validation.json', validation)
    if a.visuals == 'reference' and version == '0.1.0' and sha(bda) != reference['bda_sha256']:
        raise ValueError('Reference build is not byte-identical to the verified BDA')
    if source_hashes() != inputs:
        raise ValueError('Source inputs changed during compilation; rebuild from a fixed snapshot')
    install = out / 'install'
    install.mkdir(parents=True, exist_ok=True)
    program = install / '应用/程序/游戏王.bda'
    program.parent.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(bda, program)
    resource_folder = install / '应用/数据/yugioh'
    if a.assets:
        resource_folder.mkdir(parents=True, exist_ok=True)
    for name in ASSET_NAMES:
        target = resource_folder / name
        if a.assets:
            shutil.copyfile(a.assets / name, target)
        elif target.exists():
            target.unlink()
    report = {'passed': True, 'version': version,
              'title': '游戏王', 'sdk_commit': sdk, 'compiler': compiler,
              'bda_sha256': sha(bda), 'bda_bytes': bda.stat().st_size, 'elf_sha256': sha(elf_path),
              'visuals': a.visuals, 'external_resources_included': bool(a.assets),
              'native_execution': True, 'interpreter_linked': False, 'binary_translation': False,
              'original_executable_loaded': False, 'floating_point_instructions': 0,
              'entry': '0x83c00020', 'bss_end': hex(end), 'native_mips_code_bytes': code_bytes,
              'duel_state_layout': layouts['joey_duel_layout'], 'duel_state_bytes': layouts['joey_duel_layout'][0],
              'profile_state_layout': layouts['joey_profile_layout'], 'source_hashes': inputs,
              'resource_search_paths': ['A:/应用/数据/yugioh', 'B:/应用/数据/yugioh'],
              'files': {'JOEY.bda': sha(bda), **({name: sha(resource_folder / name) for name in ASSET_NAMES} if a.assets else {})},
              'reference_firmware_evidence_applies': sha(bda) == reference['bda_sha256'],
              'firmware_tested_in_this_build': False, 'physical_h1_tested': False}
    write_json(out / 'build.json', report)
    print(f"Built {bda.name}: {report['bda_bytes']} bytes, SHA256 {report['bda_sha256']}")


if __name__ == '__main__':
    main()
