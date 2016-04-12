# Functionality related to the C compiler.

import os, shutil, subprocess, tempfile

def uint128_supported(cc, cflags=[]):
    '''
    Test whether the given C compiler supports the `__int128` type.
    '''
    p = subprocess.Popen([cc, '-x', 'c', '-o', os.devnull, '-'] + cflags,
        stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    p.communicate(
        '#include <stdint.h>\n'
        'int main(void) {\n'
        '  unsigned __int128 x;\n'
        '  return 0;\n'
        '}')
    return p.returncode == 0

def get_constant(name, cc, cflags=[]):
    '''
    Get the value of a given C integral constant.
    '''
    tmp = tempfile.mkdtemp()
    try:
        src = os.path.join(tmp, 'main.c')
        with open(src, 'wt') as f:
            f.write(
                '#include <inttypes.h>\n'
                '#include <limits.h>\n'
                '#include <stdint.h>\n'
                '#include <stdio.h>\n'
                'int main(void) {\n'
                '  printf("%%" PRIuMAX, (uintmax_t)(%s));\n'
                '  return 0;\n'
                '}' % name)
        aout = os.path.join(tmp, 'a.out')
        subprocess.check_call([cc, '-x', 'c', '-o', aout, src])
        return int(subprocess.check_output([aout]))
    finally:
        shutil.rmtree(tmp)
