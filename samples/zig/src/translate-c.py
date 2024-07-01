#!/usr/bin/env python3

import re
import subprocess

include = '/usr/include/'
lines: list[str]

# Zig's C-translator does not like bit fields (for now)
with open('../../../pure-data/src/m_pd.h', 'r') as f:
	lines = f.read().splitlines()
	for i in range(len(lines)):
		if (lines[i].startswith('    unsigned int te_type:2;')):
			lines[i] = '    unsigned char te_type;'

with open('m_pd.h', 'w') as f:
	f.write('\n'.join(lines))

# translate to zig
out = subprocess.check_output([
	'zig', 'translate-c',
	'-I.', '-isystem', include,
	'../../../libpd_wrapper/z_libpd.h'
], encoding='utf-8')


types = {
	'atomtype': 't_atomtype',
	'binbuf': 'pd.BinBuf',
	'floatarg': 'pd.Float',
	'garray': 'pd.GArray',
	'glist': 'pd.GList',
	'gobj': 'pd.GObj',
	'gpointer': 'pd.GPointer',
	'newmethod': 'pd.NewMethod',
	'perfroutine': 'pd.PerfRoutine',

	'libpd_printhook': 'lpd.PrintHook',
	'libpd_banghook': 'lpd.BangHook',
	'libpd_floathook': 'lpd.FloatHook',
	'libpd_doublehook': 'lpd.DoubleHook',
	'libpd_symbolhook': 'lpd.SymbolHook',
	'libpd_listhook': 'lpd.ListHook',
	'libpd_messagehook': 'lpd.MessageHook',
	'libpd_noteonhook': 'lpd.NoteOnHook',
	'libpd_controlchangehook': 'lpd.ControlChangeHook',
	'libpd_programchangehook': 'lpd.ProgramChangeHook',
	'libpd_pitchbendhook': 'lpd.PitchBendHook',
	'libpd_aftertouchhook': 'lpd.AftertouchHook',
	'libpd_polyaftertouchhook': 'lpd.PolyAftertouchHook',
	'libpd_midibytehook': 'lpd.MidiByteHook',
	'libpd_freehook': 'lpd.FreeHook',
}

vec_names = ['argv', 'av', 'vec']
buf_names = ['inBuffer', 'outBuffer']

# Types should be TitleCase and referential to our tailored zig definitions
r_type = r'([^\w])(?:struct__|t_|union_)(\w+)'
def re_type(m):
	name = m.group(2)
	name = types[name] if name in types else ('pd.' + name.capitalize())
	return m.group(1) + name

# Assume pointers aren't intended to be optional except in special cases,
# and assume char pointers are supposed to be null-terminated strings
r_param = r'(\w+): (?:\[\*c\]|\?\*)(const )?([\w\.]+)'
def re_param(m):
	name = m.group(1)
	typ = m.group(3)
	p = '[*]' if name in vec_names \
		else '?[*]' if name in buf_names \
		else '[*:0]' if typ == 'u8' \
		else '*'
	return m.group(1) + ': ' + p + (m.group(2) or '') + typ

# Double pointer usually means it's either an array pointer or a symbol pointer
r_dblptr = r'\[\*c\]\[\*c\](const )?([\w\.]+)'
def re_dblptr(m):
	ptr = '**' if m.group(2) == 'pd.Symbol' else '*[*]'
	return ptr + (m.group(1) or '') + m.group(2)

lines = out.splitlines()
for i in range(len(lines)):
	if re.match(r'(pub extern fn|pub const \w+ = \?\*const fn)', lines[i]):
		m = re.match(r'(.*)\((?!\.)(.*)\)(.*)', lines[i])
		if m:
			ret = re.sub(r_type, re_type, m.group(3))
			args = re.sub(r_type, re_type, m.group(2))
			args = re.sub(r_param, re_param, args)
			args = re.sub(r_dblptr, re_dblptr, args)
			lines[i] = m.group(1) + '(' + args + ')' + ret
	elif re.match(r'pub const t_(\w+) = struct__(\w+)', lines[i]):
		lines[i] = re.sub(r'([^\w])(?:struct__|union_)(\w+)', re_type, lines[i])

with open('m_pd.zig', 'w') as f:
	f.write('const pd = @import("pd.zig");\nconst lpd = @import("libpd.zig");\n' \
		+ '\n'.join(lines))
