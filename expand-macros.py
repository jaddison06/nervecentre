# Utility to expand macros without expanding #includes - `python expand-macros.py widget.c "\"widget.h\""`

import os, sys

def main():
    fname = sys.argv[1]
    os.system(f'gcc -E {fname} -o expanded_{fname}')
    with open(f'expanded_{fname}', 'rt') as fh:
        i = 0
        lines = fh.readlines()
        for line in lines:
            if line.startswith('#') and line.split(' ')[2] == f'"{fname}"':
                break
            i += 1
    with open(f'expanded_{fname}', 'wt') as fh:
        lines = lines[i:]
        if len(sys.argv) > 2:
            for include in sys.argv[2:]:
                lines.insert(0, f'#include {include}\n')
        fh.write(''.join(lines))

if __name__ == '__main__': main()