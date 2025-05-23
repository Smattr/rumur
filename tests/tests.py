"""
Rumur integration test suite
"""

import functools
import multiprocessing
import os
import platform
import re
import shutil
import stat
import subprocess as sp
import sys
import tempfile
import textwrap
from pathlib import Path

import pytest

# we intentionally avoid f-strings to retain compatibility with Python 3.4, so
# disable warnings about those
# pylint: disable=consider-using-f-string

CPUS = multiprocessing.cpu_count()

VERIFIER_RNG = Path(__file__).resolve().parent / "../misc/verifier.rng"
MURPHI2XML_RNG = Path(__file__).resolve().parent / "../misc/murphi2xml.rng"


def enc(s):
    return s.encode("utf-8", "replace")


def dec(s):
    return s.decode("utf-8", "replace")


def run(args, stdin=None):
    """
    run a command and return its result
    """
    if stdin is not None:
        stdin = enc(stdin)
    p = sp.Popen([str(a) for a in args], stdout=sp.PIPE, stderr=sp.PIPE, stdin=sp.PIPE)
    stdout, stderr = p.communicate(stdin)
    return p.returncode, dec(stdout), dec(stderr)


def cc():
    """find the C compiler"""
    return os.environ.get("CC", "cc")


def cxx():
    """find the C++ compiler"""
    return os.environ.get("CXX", "c++")


def has_c_flag(flag):
    """does the C compiler support the given flag?"""
    ret, _, _ = run(
        [cc(), "-x", "c", "-std=c11", flag, "-o", os.devnull, "-"],
        "int main(void) { return 0; }",
    )
    return ret == 0


@functools.lru_cache()
def has_march_native():
    """does the compiler support -march=native?"""
    return has_c_flag("-march=native")


@functools.lru_cache()
def has_mcx16():
    """does the compiler support -mcx16?"""
    return has_c_flag("-mcx16")


@functools.lru_cache()
def c_flags():
    """initial flags to pass to our C compiler"""

    # first, the default flags
    flags = [
        "-x",
        "c",
        "-std=c11",
        "-Werror=format",
        "-Werror=sign-compare",
        "-Werror=type-limits",
    ]

    # test if the C compiler supports -Werror=enum-conversion
    if has_c_flag("-Werror=enum-conversion"):
        flags += ["-Werror=enum-conversion"]

    # test if the C compiler supports -Werror=maybe-uninitialized
    if has_c_flag("-Werror=maybe-uninitialized"):
        flags += ["-Werror=maybe-uninitialized"]

    # test if the C compiler supports -march=native
    if has_march_native():
        flags += ["-march=native"]

    # test if the C compiler supports -mcx16
    if has_mcx16():
        flags += ["-mcx16"]

    return flags


@functools.lru_cache()
def needs_libatomic():
    """does the toolchain need -latomic to support dword CAS?"""

    cflags = ["-x", "c", "-std=c11"]
    if has_march_native():
        cflags += ["-march=native"]
    if has_mcx16():
        cflags += ["-mcx16"]

    # compile a program that operates on a double-word
    ret, _, _ = run(
        [cc()] + cflags + ["-o", os.devnull, "-"],
        textwrap.dedent(
            """\
#include <stdbool.h>
#include <stdint.h>

// replicate what is in ../rumur/resources/header.c

#define THREADS 2

#if __SIZEOF_POINTER__ <= 4
  typedef uint64_t dword_t;
#elif __SIZEOF_POINTER__ <= 8
  typedef unsigned __int128 dword_t;
#else
  #error "unexpected pointer size; what scalar type to use for dword_t?"
#endif

static dword_t atomic_read(dword_t *p) {

  if (THREADS == 1) {
    return *p;
  }

#if defined(__x86_64__) || defined(__i386__) || \\
    (defined(__aarch64__) && defined(__GNUC__) && !defined(__clang__))
  /* x86-64: MOV is not guaranteed to be atomic on 128-bit naturally aligned
   *   memory. The way to work around this is apparently the following
   *   degenerate CMPXCHG16B.
   * i386: __atomic_load_n emits code calling a libatomic function that takes a
   *   lock, making this no longer lock free. Force a CMPXCHG8B by using the
   *   __sync built-in instead.
   * ARM64: __atomic_load_n emits code calling a libatomic function that takes a
   *   lock, making this no longer lock free. Force a CASP by using the __sync
   *   built-in instead.
   *
   * XXX: the obvious (irrelevant) literal to use here is “0” but this triggers
   * a GCC bug on ARM, https://gcc.gnu.org/bugzilla/show_bug.cgi?id=114310.
   */
  return __sync_val_compare_and_swap(p, 1, 1);
#endif

  return __atomic_load_n(p, __ATOMIC_SEQ_CST);
}

static void atomic_write(dword_t *p, dword_t v) {

  if (THREADS == 1) {
    *p = v;
    return;
  }

#if defined(__x86_64__) || defined(__i386__) || \\
    (defined(__aarch64__) && defined(__GNUC__) && !defined(__clang__))
  /* As explained above, we need some extra gymnastics to avoid a call to
   * libatomic on x86-64, i386, and ARM64.
   */
  dword_t expected;
  dword_t old = 0;
  do {
    expected = old;
    old = __sync_val_compare_and_swap(p, expected, v);
  } while (expected != old);
  return;
#endif

  __atomic_store_n(p, v, __ATOMIC_SEQ_CST);
}

static bool atomic_cas(dword_t *p, dword_t expected, dword_t new) {

  if (THREADS == 1) {
    if (*p == expected) {
      *p = new;
      return true;
    }
    return false;
  }

#if defined(__x86_64__) || defined(__i386__) || \\
    (defined(__aarch64__) && defined(__GNUC__) && !defined(__clang__))
  /* Make GCC >= 7.1 emit cmpxchg on x86-64 and i386. See
   * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=80878.
   * Make GCC emit a CASP on ARM64 (undocumented?).
   */
  return __sync_bool_compare_and_swap(p, expected, new);
#endif

  return __atomic_compare_exchange_n(p, &expected, new, false, __ATOMIC_SEQ_CST,
    __ATOMIC_SEQ_CST);
}

static dword_t atomic_cas_val(dword_t *p, dword_t expected, dword_t new) {

  if (THREADS == 1) {
    dword_t old = *p;
    if (old == expected) {
      *p = new;
    }
    return old;
  }

#if defined(__x86_64__) || defined(__i386__) || \\
    (defined(__aarch64__) && defined(__GNUC__) && !defined(__clang__))
  /* Make GCC >= 7.1 emit cmpxchg on x86-64 and i386. See
   * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=80878.
   * Make GCC emit a CASP on ARM64 (undocumented?).
   */
  return __sync_val_compare_and_swap(p, expected, new);
#endif


  (void)__atomic_compare_exchange_n(p, &expected, new, false, __ATOMIC_SEQ_CST,
    __ATOMIC_SEQ_CST);
  return expected;
}

int main(void) {
  dword_t target = 0;

  target = atomic_read(&target);

  atomic_write(&target, 42);

  atomic_cas(&target, 42, 0);

  return (int)atomic_cas_val(&target, 0, 42);
}
"""
        ),
    )
    return ret != 0


@functools.lru_cache()
def has_sandbox():
    """whether the current platform has sandboxing support for the verifier"""

    # assume macOS always has sandboxing support
    if platform.system() == "Darwin":
        return True

    # assume FreeBSD always has sandboxing support
    if platform.system() == "FreeBSD":
        return True

    # assume OpenBSD always has sandboxing support
    if platform.system() == "OpenBSD":
        return True

    # for Linux, we need to check for seccomp
    if platform.system() == "Linux":
        with tempfile.TemporaryDirectory() as t:
            tmp = Path(t)

            # create a sandbox testing program
            test_c = tmp / "test.c"
            with open(str(test_c), "wt", encoding="utf-8") as f:
                f.write(
                    textwrap.dedent(
                        """\
                #include <linux/audit.h>
                #include <linux/filter.h>
                #include <linux/seccomp.h>
                #include <stdio.h>
                #include <stdlib.h>
                #include <sys/prctl.h>
                #include <sys/socket.h>
                #include <sys/syscall.h>

                int main(void) {

                  // disable addition of new privileges
                  int r = prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0);
                  if (r != 0) {
                    perror("prctl(PR_SET_NO_NEW_PRIVS) failed");
                    return EXIT_FAILURE;
                  }

                  // a BPF program that allows everything
                  static struct sock_filter filter[] = {

                    // return allow
                    BPF_STMT(BPF_RET|BPF_K, SECCOMP_RET_ALLOW),

                  };

                  static const struct sock_fprog filter_program = {
                    .len = sizeof(filter) / sizeof(filter[0]),
                    .filter = filter,
                  };

                  // apply the filter to ourselves
                  r = prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &filter_program, 0, 0);
                  if (r != 0) {
                    perror("prctl(PR_SET_SECCOMP) failed");
                    return EXIT_FAILURE;
                  }

                  return EXIT_SUCCESS;
                }
                """
                    )
                )

            # compile the test program
            a_out = tmp / "a.out"
            ret, _, _ = run([cc(), "-std=c11", test_c, "-o", a_out])
            if ret != 0:
                return False

            # execute the test program
            ret, _, _ = run([a_out])
            if ret != 0:
                return False

        return True

    return False


def has_valgrind():
    """is Valgrind available?"""
    return shutil.which("valgrind") is not None


def has_xmllint():
    """is xmllint available?"""
    return shutil.which("xmllint") is not None


@functools.lru_cache()
def smt_bv_args():
    """get bitvector SMT arguments for an available solver"""

    # preference 1: Z3
    if shutil.which("z3") is not None:
        # we leave a blank logic here, as Z3 performs best when not given a logic
        return [
            "--smt-path",
            "z3",
            "--smt-arg=-smt2",
            "--smt-arg=-in",
            "--smt-bitvectors",
            "on",
        ]

    # preference 2: CVC4
    if shutil.which("cvc4") is not None:
        return [
            "--smt-path",
            "cvc4",
            "--smt-arg=--lang=smt2",
            "--smt-arg=--rewrite-divk",
            "--smt-prelude",
            "(set-logic AUFBV)",
            "--smt-bitvectors",
            "on",
        ]

    # otherwise, give up
    return None


@functools.lru_cache()
def smt_args():
    """get SMT arguments for an available solver"""

    # preference 1: Z3
    if shutil.which("z3") is not None:
        # we leave a blank logic here, as Z3 performs best when not given a logic
        return ["--smt-path", "z3", "--smt-arg=-smt2", "--smt-arg=-in"]

    # preference 2: CVC4
    if shutil.which("cvc4") is not None:
        return [
            "--smt-path",
            "cvc4",
            "--smt-arg=--lang=smt2",
            "--smt-arg=--rewrite-divk",
            "--smt-prelude",
            "(set-logic AUFLIA)",
        ]

    # otherwise, give up
    return None


def test_display_info():
    """
    this is not a test case, but just a vehicle for echoing useful things into the CI
    log
    """

    # output a newline to make things more readable in `--capture=no --verbose` mode
    print()
    print("  CC = {}".format(cc()))
    print("  CXX = {}".format(cxx()))
    print("  c_flags() = {}".format(c_flags()))
    print("  has_march_native() = {}".format(has_march_native()))
    print("  has_mcx16() = {}".format(has_mcx16()))
    print("  has_sandbox() = {}".format(has_sandbox()))
    print("  has_valgrind() = {}".format(has_valgrind()))
    print("  has_xmllint() = {}".format(has_xmllint()))
    print("  needs_libatomic() = {}".format(needs_libatomic()))
    print("  smt_bv_args() = {}".format(smt_bv_args()))
    print("  smt_args() = {}".format(smt_args()))


def parse_test_options(
    src, debug=False, multithreaded=False, xml=False
):  # pylint:disable=unused-argument
    """
    extract test tweaks and directives from leading comments in a test input

    We disable `unused-argument` warnings because these are used by strings read from
    test cases that are eval-ed.
    """
    with open(str(src), "rt", encoding="utf-8") as f:
        for line in f:
            # recognise “-- rumur_flags: …” etc lines
            m = re.match(r"\s*--\s*(?P<key>[a-zA-Z_]\w*)\s*:(?P<value>.*)$", line)
            if m is None:
                break
            key = m.group("key")
            value = eval(m.group("value").strip())  # pylint: disable=eval-used
            yield key, value


def test_murphi_format_colon():
    """colon spacing in definitions as well as ternary expressions by murphi-format"""

    # sample Murphi that uses colons in both situations, that should be reflowed
    model = "const y: 0?1:2; var x : boolean;"

    ret, stdout, stderr = run(["murphi-format"], model)

    assert ret == 0, "failed to reflow Murphi snippet"
    assert stderr == "", "murphi-format printed errors/warnings"

    assert "y: 0" in stdout, "incorrect spacing for const definition"
    assert "0 ? 1 : 2" in stdout, "incorrect spacing for ternary expression"
    assert "x: boolean" in stdout, "incorrect spacing for var definition"


def test_murphi_format_arrow_begin1():
    """`==> begin` should not be reflowed with a newline by murphi-format"""

    # sample Murphi that uses something that should be stable
    model = 'rule "foo" ==> begin end'

    ret, stdout, stderr = run(["murphi-format"], model)

    assert ret == 0, "failed to reflow Murphi snippet"
    assert stderr == "", "murphi-format printed errors/warnings"

    assert "==> begin" in stdout, "incorrect spacing for `==> begin`"


def test_murphi_format_arrow_begin2():
    """`==> begin` should not be reflowed with a newline in murphi-format"""

    # sample Murphi that uses something that should be reflowed
    model = 'rule "foo" ==>begin end'

    ret, stdout, stderr = run(["murphi-format"], model)

    assert ret == 0, "failed to reflow Murphi snippet"
    assert stderr == "", "murphi-format printed errors/warnings"

    assert "==> begin" in stdout, "incorrect spacing for `==> begin`"


def test_murphi_format_newline_on_end():
    """`end` should force a newline in murphi-format"""

    # sample Murphi that uses something that should be reflowed
    model = 'rule "foo" begin end rule "bar" begin end'

    ret, stdout, stderr = run(["murphi-format"], model)

    assert ret == 0, "failed to reflow Murphi snippet"
    assert stderr == "", "murphi-format printed errors/warnings"

    assert re.search("\\bbegin\n+end\\b", stdout) is not None, "no newline before end"
    assert re.search("\\bend\n+rule\\b", stdout) is not None, "no newline after end"


def test_murphi_format_unary_in_for():
    """
    a unary operator following a keyword should be spaced correctly by murphi-format
    """

    model = "rule begin for x := 0 to - 2 by - 1 do end; end"

    ret, stdout, stderr = run(["murphi-format"], model)

    assert ret == 0, "failed to reflow Murphi snippet"
    assert stderr == "", "murphi-format printed errors/warnings"

    assert "to -2" in stdout, "incorrect unary minus spacing"
    assert "by -1" in stdout, "incorrect unary minus spacing"


def test_murphi_format_begin_indentation():
    """`begin` should not be erroneously indented by decl blocks in murphi-format"""

    model = "rule var x: boolean; begin end"

    ret, stdout, stderr = run(["murphi-format"], model)

    assert ret == 0, "failed to reflow Murphi snippet"
    assert stderr == "", "murphi-format printed errors/warnings"

    assert (
        re.search(r"^begin\b", stdout, flags=re.MULTILINE) is not None
    ), "incorrect begin indentation"
    assert (
        re.search(r"^\s+begin\b", stdout, flags=re.MULTILINE) is None
    ), "incorrect begin indentation"


def test_murphi_format_multiline_comment():
    """multiline comments should be recognised by murphi-format"""

    model = "rule /* hello\nworld */ begin end"

    ret, stdout, stderr = run(["murphi-format"], model)

    assert ret == 0, "failed to reflow Murphi snippet"
    assert stderr == "", "murphi-format printed errors/warnings"

    assert "/* hello" in stdout, "multiline comments mishandled"
    assert "world */" in stdout, "multiline comments mishandled"


def test_murphi_format_then_indentation():
    """`then` should incur an murphi-format indent"""

    model = "rule begin if 0 = 0 then\nx := 2; end; end"

    ret, stdout, stderr = run(["murphi-format"], model)

    assert ret == 0, "failed to reflow Murphi snippet"
    assert stderr == "", "murphi-format printed errors/warnings"

    assert "0 = 0 then" in stdout, "incorrect `then` indentation"
    assert (
        re.search(r"^    x := 2", stdout, flags=re.MULTILINE) is not None
    ), "incorrect `then` indentation"


def test_murphi_format_newline_comment():
    """is newline followed by a comment preserved by murphi-format?"""

    model = "const N:\n-- a comment\n0;"

    ret, stdout, stderr = run(["murphi-format"], model)

    assert ret == 0, "failed to reflow Murphi snippet"
    assert stderr == "", "murphi-format printed errors/warnings"

    assert (
        re.search(r"^\s*-- a comment$", stdout, flags=re.MULTILINE) is not None
    ), "incorrect newline,comment handling"


def test_murphi_format_brace_ender():
    """'}' should trigger a newline in murphi-format"""

    model = "type x: enum {A, B} y: boolean;"

    ret, stdout, stderr = run(["murphi-format"], model)

    assert ret == 0, "failed to reflow Murphi snippet"
    assert stderr == "", "murphi-format printed errors/warnings"

    assert "}\n" in stdout, "`}` did not trigger newline"


def test_murphi_format_double_paren():
    """does multi-dimensional indexing get spaced correctly by murphi-format?"""

    model = "rule begin x [ 1 ] [ 2 ] := 3; end"

    ret, stdout, stderr = run(["murphi-format"], model)

    assert ret == 0, "failed to reflow Murphi snippet"
    assert stderr == "", "murphi-format printed errors/warnings"

    assert "x[1][2]" in stdout, "spaced incorrectly"


def test_murphi_format_procedure_var():
    """
    does `var` within a function/procedure parameter list cause problems for
    murphi-format?
    """

    model = "procedure foo( var x : boolean); begin end"

    ret, stdout, stderr = run(["murphi-format"], model)

    assert ret == 0, "failed to reflow Murphi snippet"
    assert stderr == "", "murphi-format printed errors/warnings"

    assert "(var x: boolean)" in stdout, "var parameters spaced incorrectly"


def murphi_format_test_trailing_space():
    """
    does something that would normally be followed by a space also incur a space
    when landing at the end-of-file processed by murphi-format?
    """

    model = "invariant x"

    ret, stdout, stderr = run(["murphi-format"], model)

    assert ret == 0, "failed to reflow Murphi snippet"
    assert stderr == "", "murphi-format printed errors/warnings"

    assert (
        re.search(r"\binvariant x$", stdout, flags=re.MULTILINE) is not None
    ), "incorrect spacing around end-of-file"


def test_murphi_format_bad_operator():
    """
    non-operators like `>-` should not be recognised by murphi-format
    """

    model = "rule begin if x >- 1 then end; end"

    ret, stdout, stderr = run(["murphi-format"], model)

    assert ret == 0, "failed to reflow Murphi snippet"
    assert stderr == "", "murphi-format printed errors/warnings"

    assert " >- " not in stdout, "`>-` incorrectly considered an operator"
    assert ">-" not in stdout, "`>-` incorrectly considered an operator"
    assert "> -" in stdout, "incorrect spacing around `>-`"


def test_murphi_format_case1():
    """reformatting should be caseless"""

    model = "cOnSt N: 0;"

    ret, stdout, stderr = run(["murphi-format"], model)

    assert ret == 0, "failed to reflow Murphi snippet"
    assert stderr == "", "murphi-format printed errors/warnings"

    assert "cOnSt\n  N: 0;" in stdout, "incorrect spacing around erratic casing"


def test_murphi_format_no_start_newline():
    """a newline should not be inserted by murphi-format before all content"""

    model = "rule begin end"

    ret, stdout, stderr = run(["murphi-format"], model)

    assert ret == 0, "failed to reflow Murphi snippet"
    assert stderr == "", "murphi-format printed errors/warnings"

    assert stdout.startswith("rule"), "incorrect preceding space inserted"


def test_murphi_format_switch():
    """murphi-format of switch statements"""

    model = "rule begin switch x\nend; end"

    ret, stdout, stderr = run(["murphi-format"], model)

    assert ret == 0, "failed to reflow Murphi snippet"
    assert stderr == "", "murphi-format printed errors/warnings"

    assert (
        "rule begin\n  switch x\n  end;\nend\n" == stdout
    ), "incorrect switch formatting"


def test_murphi_format_multiple_inplace(tmp_path):
    """murphi-format on multiple files in-place should work"""

    # a model with a small amount of text
    short = "const\n  x: 10;\n"

    # a model with a longer amount of text
    long = "var\n  x: 0..10;\n  y: 0..10;\n  z: 0..10;\n"

    short_path = tmp_path / "short.m"
    long_path = tmp_path / "long.m"

    with open(str(short_path), "wt", encoding="utf-8") as f:
        f.write(short)
    with open(str(long_path), "wt", encoding="utf-8") as f:
        f.write(long)

    ret, stdout, stderr = run(["murphi-format", "--in-place", long_path, short_path])

    assert ret == 0, "failed to reflow Murphi snippets"
    assert stdout == "", "murphi-format produced output when asked for in-place"
    assert stderr == "", "murphi-format printed errors/warnings"

    with open(str(short_path), "rb") as f:
        content = f.read()
    assert content == short.encode("utf-8"), "model was reflowed incorrectly"

    with open(str(long_path), "rb") as f:
        content = f.read()
    assert content == long.encode("utf-8"), "model was reflowed incorrectly"


def test_murphi_format_trailing_indentation():
    """murphi-format should not incur a trailing indentation at end of file"""

    model = "const N: 10;"

    ret, stdout, stderr = run(["murphi-format"], model)

    assert ret == 0, "failed to reflow Murphi snippet"
    assert stderr == "", "murphi-format printed errors/warnings"

    assert "const\n  N: 10;\n" == stdout, "incorrect const block formatting"


def test_murphi_format_startstate_no_begin():
    """unnamed startstate should be followed by correct murphi-format indentation"""

    model = "startstate x := y; end"

    ret, stdout, stderr = run(["murphi-format"], model)

    assert ret == 0, "failed to reflow Murphi snippet"
    assert stderr == "", "murphi-format printed errors/warnings"

    assert "startstate\n  x := y;\nend\n" == stdout, "incorrect startstate formatting"


def test_murphi_format_startstate_begin():
    """
    unnamed startstate with `begin` should be followed by correct murphi-format
    indentation
    """

    model = "startstate begin x := y; end"

    ret, stdout, stderr = run(["murphi-format"], model)

    assert ret == 0, "failed to reflow Murphi snippet"
    assert stderr == "", "murphi-format printed errors/warnings"

    assert (
        "startstate begin\n  x := y;\nend\n" == stdout
    ), "incorrect startstate formatting"


def test_murphi_format_else():
    """murphi-format indentation of `else` should be correct"""

    model = "rule begin if x = x then y := z; else y := w; end; end"

    ret, stdout, stderr = run(["murphi-format"], model)

    assert ret == 0, "failed to reflow Murphi snippet"
    assert stderr == "", "murphi-format printed errors/warnings"

    assert (
        "\n  if x = x then\n    y := z;\n  else\n    y := w;\n  end;\n" in stdout
    ), "incorrect else formatting"


def test_murphi_format_elsif():
    """murphi-format indentation of `elsif` should be correct"""

    model = "rule begin if x = x then y := z; elsif y = y then y := w; end; end"

    ret, stdout, stderr = run(["murphi-format"], model)

    assert ret == 0, "failed to reflow Murphi snippet"
    assert stderr == "", "murphi-format printed errors/warnings"

    assert (
        "\n  if x = x then\n    y := z;\n  elsif y = y then\n    y := w;\n  end;\n"
        in stdout
    ), "incorrect elsif formatting"


def test_murphi_format_while_paren():
    """`while` followed by parenthesised expression should be spaced by murphi-format"""

    model = "rule begin while (x = x) do y := z; end; end"

    ret, stdout, stderr = run(["murphi-format"], model)

    assert ret == 0, "failed to reflow Murphi snippet"
    assert stderr == "", "murphi-format printed errors/warnings"

    assert (
        "\n  while (x = x) do\n    y := z;\n  end;\n" in stdout
    ), "incorrect while formatting"


def test_murphi_format_no_format():
    """formatting disabling comments should be respected"""

    model = textwrap.dedent(
        """\
    rule begin -- murphi-format: off
     while (x = x) do y := z; end; -- murphi-format: on
     end"""
    )

    ret, stdout, stderr = run(["murphi-format"], model)

    assert ret == 0, "failed to reflow Murphi snippet"
    assert stderr == "", "murphi-format printed errors/warnings"

    assert (
        "-- murphi-format: off\n while (x = x) do y := z; end; -- murphi-format: on\n"
        in stdout
    ), "format-disabling comments did not work"


def test_murphi_format_isundefined():
    """`isundefined` should be spaced correctly by murphi-format"""

    model = "rule begin x := isundefined (x); end"

    ret, stdout, stderr = run(["murphi-format"], model)

    assert ret == 0, "failed to reflow Murphi snippet"
    assert stderr == "", "murphi-format printed errors/warnings"

    assert ":= isundefined(x)" in stdout, "`isundefined` spaced incorrectly"


def test_murphi_format_line_comment():
    """line comments should not be bumped onto the next line by murphi-format"""

    model = "rule begin x := y; -- line comment\nz := w;\n-- comment on newline\n end"

    ret, stdout, stderr = run(["murphi-format"], model)

    assert ret == 0, "failed to reflow Murphi snippet"
    assert stderr == "", "murphi-format printed errors/warnings"

    assert (
        "rule begin\n  x := y; -- line comment\n  z := w;\n  -- comment on newline\nend\n"
        == stdout
    ), "comments broken incorrectly"


def test_murphi_format_procedure_params():
    """parameter lists in procedures should appear correctly after murphi-format"""

    model = "procedure foo(a: b; c: d) begin end"

    ret, stdout, stderr = run(["murphi-format"], model)

    assert ret == 0, "failed to reflow Murphi snippet"
    assert stderr == "", "murphi-format printed errors/warnings"

    assert "a: b; c: d" in stdout, "procedure parameters formatted incorrectly"


def test_murphi_format_comment_const_interleaved():
    """comments attached to consts should stay where they are across murphi-format"""

    model = "const x: 42;\n\n-- hello world\ny: 42;"

    ret, stdout, stderr = run(["murphi-format"], model)

    assert ret == 0, "failed to reflow Murphi snippet"
    assert stderr == "", "murphi-format printed errors/warnings"

    assert (
        "const\n  x: 42;\n\n  -- hello world\n  y: 42;\n" == stdout
    ), "const comments formatted incorrectly"


def test_murphi_format_scalarset():
    """`scalarset` should be spaced correctly by murphi-format"""

    model = "type x : scalarset (4);"

    ret, stdout, stderr = run(["murphi-format"], model)

    assert ret == 0, "failed to reflow Murphi snippet"
    assert stderr == "", "murphi-format printed errors/warnings"

    assert ": scalarset(4)" in stdout, "`scalarset` spaced incorrectly"


def test_murphi_format_array():
    """`array` should be spaced intuitively by murphi-format"""

    model = "type x : array [ boolean ] of boolean;"

    ret, stdout, stderr = run(["murphi-format"], model)

    assert ret == 0, "failed to reflow Murphi snippet"
    assert stderr == "", "murphi-format printed errors/warnings"

    assert ": array[boolean] of boolean" in stdout, "`array` spaced incorrectly"


def test_murphi_format_not_unicode():
    """murphi-format `¬` spacing should be correct"""

    model = "rule begin x := ¬ y; end"

    ret, stdout, stderr = run(["murphi-format"], model)

    assert ret == 0, "failed to reflow Murphi snippet"
    assert stderr == "", "murphi-format printed errors/warnings"

    assert ":= ¬y" in stdout, "`¬` spaced incorrectly"


def test_muprhi_format_smart_quotes():
    """murphi-format should handle smart quotes (“”) correctly"""

    model = "rule begin assert “foo bar” x; end"

    ret, stdout, stderr = run(["murphi-format"], model)

    assert ret == 0, "failed to reflow Murphi snippet"
    assert stderr == "", "murphi-format printed errors/warnings"

    assert "assert “foo bar” x" in stdout, "smart quotes spaced incorrectly"


def test_murphi_format_named_startstate():
    """startstate with a name should be formatted correctly by murphi-format"""

    model = 'startstate "foo" begin x := y; end'

    ret, stdout, stderr = run(["murphi-format"], model)

    assert ret == 0, "failed to reflow Murphi snippet"
    assert stderr == "", "murphi-format printed errors/warnings"

    assert 'startstate "foo" begin' in stdout, "startstate spaced incorrectly"


def test_murphi_format_case2():
    """murphi-format formatting of cases within switch statements"""

    model = "rule begin switch x case 1: y := x; case 2: z := x; end; end"

    ret, stdout, stderr = run(["murphi-format"], model)

    assert ret == 0, "failed to reflow Murphi snippet"
    assert stderr == "", "murphi-format printed errors/warnings"

    assert (
        re.search("^  switch x", stdout, flags=re.MULTILINE) is not None
    ), "incorrect `switch` indentation"
    assert (
        re.search("^  case 1", stdout, flags=re.MULTILINE) is not None
    ), "incorrect `case` indentation"
    assert (
        re.search("^  case 2", stdout, flags=re.MULTILINE) is not None
    ), "incorrect `case` indentation"
    assert (
        re.search("^    y := x", stdout, flags=re.MULTILINE) is not None
    ), "incorrect `case` indentation"
    assert (
        re.search("^    z := x", stdout, flags=re.MULTILINE) is not None
    ), "incorrect `case` indentation"


def test_murphi_format_no_trailing_space_before_comment():
    """
    in murphi-format, something that would normally incur a following space
    should not have one if the next thing is a breaking line comment
    """

    model = "rule begin alias\n-- hello world\nx:y do x := z; end; end"

    ret, stdout, stderr = run(["murphi-format"], model)

    assert ret == 0, "failed to reflow Murphi snippet"
    assert stderr == "", "murphi-format printed errors/warnings"

    assert (
        re.search(r"\balias$", stdout, flags=re.MULTILINE) is not None
    ), "incorrect trailing space"


def test_murphi_format_unicode_op():
    """
    unicode operators starting with 0xe2 should be handled by murphi-format correctly
    """

    model = "rule x ∨ y ==> begin x := z; end"

    ret, stdout, stderr = run(["murphi-format"], model)

    assert ret == 0, "failed to reflow Murphi snippet"
    assert stderr == "", "murphi-format printed errors/warnings"

    assert " x ∨ y " in stdout, "incorrect unicode operator handling"


def test_murphi_format_hex_literal():
    """murphi-format should handle hexadecimal literals correctly"""

    model = "rule begin x := 0xe2; end"

    ret, stdout, stderr = run(["murphi-format"], model)

    assert ret == 0, "failed to reflow Murphi snippet"
    assert stderr == "", "murphi-format printed errors/warnings"

    assert (
        re.search("^  x := 0xe2;$", stdout, flags=re.MULTILINE) is not None
    ), "incorrect hex spacing"


def test_murphi_format_end_newline():
    """murphi-format should always leave a final newline"""

    model = "invariant x"

    ret, stdout, stderr = run(["murphi-format"], model)

    assert ret == 0, "failed to reflow Murphi snippet"
    assert stderr == "", "murphi-format printed errors/warnings"

    assert stdout.endswith("\n"), "incorrect file ending"


@pytest.mark.parametrize("component", ("list", "set"))
@pytest.mark.skipif(shutil.which("m4") is None, reason="m4 not available")
def test_stdlib(component, tmp_path):
    """test ../share/_<component>"""

    # pre-process the tester with M4
    share = Path(__file__).absolute().parents[1] / "share"
    ret, model_m, stderr = run(
        ["m4", "--include", share, share / "test_{}.m".format(component)]
    )
    assert ret == 0, "M4 failed:\n{}{}".format(model_m, stderr)

    # run the pre-processed output through Rumur
    ret, model_c, stderr = run(["rumur", "--output=/dev/stdout"], model_m)
    assert ret == 0, "Rumur failed:\n{}{}".format(model_c, stderr)

    # build up arguments to call the C compiler
    model_bin = tmp_path / "model.exe"
    args = [cc()] + c_flags() + ["-O3", "-o", model_bin, "-", "-lpthread"]

    if needs_libatomic():
        args += ["-latomic"]

    # call the C compiler
    ret, stdout, stderr = run(args, model_c)
    assert ret == 0, "C compilation failed:\n{}{}".format(stdout, stderr)

    # now run the model itself
    ret, stdout, stderr = run([model_bin])
    assert ret == 0, "Checker failed with exit status {}:\n{}{}".format(
        ret, stdout, stderr
    )


MODELS = sorted([p.name for p in Path(__file__).parent.iterdir() if p.suffix == ".m"])
"""test cases defined as .m files in this directory"""


@pytest.mark.parametrize("model", MODELS)
def test_murphi2c(model):
    """test cases for murphi2c"""

    testcase = Path(__file__).parent / model
    tweaks = dict(parse_test_options(testcase))

    # there is no C equivalent of isundefined, because an implicit assumption in the C
    # representation is that you do not rely on undefined values
    with open(str(testcase), "rt", encoding="utf-8") as f:
        should_fail = re.search(r"\bisundefined\b", f.read()) is not None

    args = ["murphi2c", "--", testcase]
    if has_valgrind():
        args = [
            "valgrind",
            "--leak-check=full",
            "--show-leak-kinds=all",
            "--error-exitcode=42",
            "--",
        ] + args
    ret, stdout, stderr = run(args)
    if has_valgrind():
        assert ret != 42, "Memory leak:\n{}{}".format(stdout, stderr)

    # if rumur was expected to reject this model, we allow murphi2c to fail
    if tweaks.get("rumur_exit_code", 0) == 0 and not should_fail:
        assert ret == 0, "Unexpected murphi2c exit:\n{}{}".format(stdout, stderr)

    if should_fail:
        assert ret != 0, "Unexpected murphi2c exit:\n{}{}".format(stdout, stderr)

    if ret != 0:
        return

    # omit -Werror=maybe-uninitialized which identifies legitimate problems in input
    # models
    cflags = [f for f in c_flags() if f != "-Werror=maybe-uninitialized"]

    # ask the C compiler if this is valid
    args = [cc()] + cflags + ["-c", "-o", os.devnull, "-"]
    ret, out, err = run(args, stdout)
    assert ret == 0, "C compilation failed:\n{}{}\nProgram:\n{}".format(
        out, err, stdout
    )


@pytest.mark.parametrize("model", MODELS)
def test_murphi2c_header(model, tmp_path):
    """test cases for murphi2c --header"""

    testcase = Path(__file__).parent / model
    tweaks = dict(parse_test_options(testcase))

    # there is no C equivalent of isundefined, because an implicit assumption in the C
    # representation is that you do not rely on undefined values
    with open(str(testcase), "rt", encoding="utf-8") as f:
        should_fail = re.search(r"\bisundefined\b", f.read()) is not None

    args = ["murphi2c", "--header", "--", testcase]
    if has_valgrind():
        args = [
            "valgrind",
            "--leak-check=full",
            "--show-leak-kinds=all",
            "--error-exitcode=42",
            "--",
        ] + args
    ret, stdout, stderr = run(args)
    if has_valgrind():
        assert ret != 42, "Memory leak:\n{}{}".format(stdout, stderr)

    # if rumur was expected to reject this model, we allow murphi2c to fail
    if tweaks.get("rumur_exit_code", 0) == 0 and not should_fail:
        assert ret == 0, "Unexpected murphi2c exit:\n{}{}".format(stdout, stderr)

    if should_fail:
        assert ret != 0, "Unexpected murphi2c exit:\n{}{}".format(stdout, stderr)

    if ret != 0:
        return

    # write the header to a temporary file
    header = tmp_path / "header.h"
    with open(str(header), "wt", encoding="utf-8") as f:
        f.write(stdout)

    # ask the C compiler if the header is valid
    main_c = '#include "{}"\nint main(void) {{ return 0; }}\n'.format(header)
    args = [cc()] + c_flags() + ["-o", os.devnull, "-"]
    ret, stdout, stderr = run(args, main_c)
    assert ret == 0, "C compilation failed:\n{}{}".format(stdout, stderr)

    # ask the C++ compiler if it is valid there too
    ret, stdout, stderr = run(
        [
            cxx(),
            "-std=c++11",
            "-o",
            os.devnull,
            "-x",
            "c++",
            "-",
            "-Werror=format",
            "-Werror=sign-compare",
            "-Werror=type-limits",
        ],
        main_c,
    )
    assert ret == 0, "C++ compilation failed:\n{}{}".format(stdout, stderr)


@pytest.mark.parametrize("model", MODELS)
def test_murphi2xml(model):
    """test cases for murphi2xml"""

    testcase = Path(__file__).parent / model
    tweaks = dict(parse_test_options(testcase))

    args = ["murphi2xml", "--", testcase]
    if has_valgrind():
        args = [
            "valgrind",
            "--leak-check=full",
            "--show-leak-kinds=all",
            "--error-exitcode=42",
            "--",
        ] + args
    ret, stdout, stderr = run(args)
    if has_valgrind():
        assert ret != 42, "Memory leak:\n{}{}".format(stdout, stderr)

    # if rumur was expected to reject this model, we allow murphi2xml to fail
    if tweaks.get("rumur_exit_code", 0) == 0:
        assert ret == 0, "Unexpected murphi2xml exit:\n{}{}".format(stdout, stderr)

    if ret != 0:
        return

    # murphi2xml will have written XML to its stdout
    xmlcontent = stdout

    # See if we have xmllint
    if not has_xmllint():
        pytest.skip("xmllint not available for validation")

    # Validate the XML
    ret, stdout, stderr = run(
        ["xmllint", "--relaxng", MURPHI2XML_RNG, "--noout", "-"], xmlcontent
    )
    assert ret == 0, "Failed to validate:\n{}{}".format(stdout, stderr)


@pytest.mark.parametrize("model", MODELS)
def test_murphi2uclid(model, tmp_path):
    """test cases for murphi2uclid"""

    testcase = Path(__file__).parent / model
    tweaks = dict(parse_test_options(testcase))

    # test cases for which murphi2uclid is expected to fail
    murphi2uclid_fail = (
        # contains `<<` or `>>`
        "const-folding5.m",
        "const-folding6.m",
        "lsh-basic.m",
        "rsh-and.m",
        "rsh-basic.m",
        "smt-bv-lsh.m",
        "smt-bv-rsh.m",
        # contains `/`
        "const-folding3.m",
        "division.m",
        "smt-bv-div.m",
        "smt-bv-div2.m",
        "smt-div.m",
        "unicode-div.m",
        "unicode-div2.m",
        # contains `%`
        "const-folding4.m",
        "mod-neg-1.m",
        "mod-neg-1_2.m",
        "put-string-injection.m",
        "smt-bv-mod.m",
        "smt-bv-mod2.m",
        "smt-mod.m",
        # contains alias statements
        "alias-and-field.m",
        "alias-in-bound.m",
        "alias-in-bound2.m",
        "alias-literal.m",
        "alias-of-alias-rule.m",
        "alias-of-alias-rule2.m",
        "alias-of-alias-stmt.m",
        "basic-aliasrule.m",
        "mixed-aliases.m",
        # `clear` of a complex type
        "clear-complex.m",
        # contains `cover`
        "cover-basic.m",
        "cover-basic2.m",
        "cover-miss.m",
        "cover-multiple.m",
        "cover-stmt.m",
        "cover-stmt-miss.m",
        "cover-trivial.m",
        "string-injection.m",
        # contains `isundefined`
        "diff-trace-arrays.m",
        "isundefined-basic.m",
        "isundefined-decl.m",
        "isundefined-element.m",
        "isundefined-function.m",
        "for-variants.m",
        "scalarset-cex.m",
        "scalarset-schedules-off.m",
        "scalarset-schedules-off-2.m",
        # contains `put`
        "for-step-0-dynamic.m",
        "put-stmt.m",
        "put-stmt2.m",
        "put-stmt3.m",
        "put-stmt4.m",
        "scalarset-put.m",
        # contains early return from a function/procedure/rule
        "return-from-rule.m",
        "return-from-ruleset.m",
        "return-from-startstate.m",
        # `exists` or `forall` with non-1 step
        "smt-bv-exists4.m",
        "smt-bv-forall4.m",
        "smt-exists4.m",
        "smt-forall4.m",
        # `liveness` inside a `ruleset`
        "liveness-in-ruleset.m",
        "liveness-in-ruleset2.m",
    )

    # test cases fo which Uclid5 is expected to fail
    uclid_fail = (
        # contains a record field with the same name as a variable
        # https://github.com/uclid-org/uclid/issues/99
        "compare-record.m",
        "smt-array-of-record.m",
        "smt-record-bool-field.m",
        "smt-record-bool-field2.m",
        "smt-record-enum-field.m",
        "smt-record-enum-field2.m",
        "smt-record-of-array.m",
        "smt-record-range-field.m",
        "smt-record-range-field2.m",
        # recursive function calls
        "recursion2.m",
        "recursion4.m",
        # reference to a field of an array element
        "193.m",
        # function calls within expressions
        "differing-type-return.m",
        "differing-type-return3.m",
        "function-call-in-if.m",
        "function-in-guard.m",
        "multiple-parameters.m",
        "multiple-parameters2.m",
        "non-const-parameters.m",
        "recursion1.m",
        "recursion5.m",
        "reference-function-parameter.m",
        "reference-function-parameter2.m",
        "section-order4.m",
        "section-order5.m",
        "section-order10.m",
        "type-shadowing2.m",
        # modifies a mutable parameter within a function, which is not valid
        # within a Uclid5 procedure
        "reference-function-parameter3.m",
    )

    args = ["murphi2uclid", "--", testcase]
    if has_valgrind():
        args = [
            "valgrind",
            "--leak-check=full",
            "--show-leak-kinds=all",
            "--error-exitcode=42",
            "--",
        ] + args
    ret, stdout, stderr = run(args)
    if has_valgrind():
        assert ret != 42, "Memory leak:\n{}{}".format(stdout, stderr)

    # if rumur was expected to reject this model, we allow murphi2uclid to fail
    should_fail = model in murphi2uclid_fail
    could_fail = tweaks.get("rumur_exit_code", 0) != 0 or should_fail

    if not could_fail:
        assert ret == 0, "Unexpected murphi2uclid exit:\n{}{}".format(stdout, stderr)

    if should_fail:
        assert ret != 0, "Unexpected murphi2uclid exit:\n{}{}".format(stdout, stderr)

    if ret != 0:
        return

    # if we do not have Uclid5 available, skip the remainder of the test
    if shutil.which("uclid") is None:
        pytest.skip("uclid5 not available for validation")

    # write the Uclid5 source to a temporary file
    src = tmp_path / "source.ucl"
    with open(str(src), "wt", encoding="utf-8") as f:
        f.write(stdout)

    # ask Uclid if the source is valid
    ret, stdout, stderr = run(["uclid", src])
    if model in uclid_fail:
        assert ret != 0, "uclid unexpectedly succeeded:\n{}{}".format(stdout, stderr)
    if model not in uclid_fail:
        assert ret == 0, "uclid failed:\n{}{}".format(stdout, stderr)


@pytest.mark.parametrize("mode", ("non-debug", "debug", "XML"))
@pytest.mark.parametrize("model", MODELS)
@pytest.mark.parametrize("multithreaded", (False, True))
@pytest.mark.parametrize("optimised", (False, True))
def test_rumur(mode, model, multithreaded, optimised, tmp_path):
    """test cases involving generating a checker and running it"""

    testcase = Path(__file__).parent / model
    debug = mode == "debug"
    xml = mode == "XML"
    tweaks = dict(parse_test_options(testcase, debug, multithreaded, xml))

    if tweaks.get("skip_reason") is not None:
        pytest.skip(tweaks["skip_reason"])

    # build up arguments to call rumur
    args = ["rumur", "--output", "/dev/stdout", testcase]
    if debug:
        args += ["--debug"]
    if xml:
        args += ["--output-format", "machine-readable"]
    if multithreaded and CPUS == 1:
        args += ["--threads", "2"]
    elif not multithreaded:
        args += ["--threads", "1"]
    args += tweaks.get("rumur_flags", [])

    if has_valgrind():
        args = [
            "valgrind",
            "--leak-check=full",
            "--show-leak-kinds=all",
            "--error-exitcode=42",
            "--",
        ] + args

    # call rumur
    ret, stdout, stderr = run(args)
    if has_valgrind():
        assert ret != 42, "Memory leak:\n{}{}".format(stdout, stderr)
    assert ret == tweaks.get("rumur_exit_code", 0), "Rumur failed:\n{}{}".format(
        stdout, stderr
    )

    # if we expected to fail, we are done
    if ret != 0:
        return

    model_c = stdout

    # build up arguments to call the C compiler
    model_bin = tmp_path / "model.exe"
    args = [cc()] + c_flags()
    if optimised:
        args += ["-O3"]
    args += ["-o", model_bin, "-", "-lpthread"]

    if needs_libatomic():
        args += ["-latomic"]

    # call the C compiler
    ret, stdout, stderr = run(args, model_c)
    assert ret == 0, "C compilation failed:\n{}{}".format(stdout, stderr)

    # now run the model itself
    ret, stdout, stderr = run([model_bin])
    assert ret == tweaks.get(
        "checker_exit_code", 0
    ), "Unexpected checker exit:\n{}{}".format(stdout, stderr)

    # if the test has a stdout expectation, check that now
    if tweaks.get("checker_output") is not None:
        assert (
            tweaks["checker_output"].search(stdout) is not None
        ), "Checker output did not match expectation regex:\n{}{}".format(
            stdout, stderr
        )

    # coarse grained check for whether the model contains a `put` statement that could
    # screw up XML validation
    with open(str(testcase), "rt", encoding="utf-8") as f:
        has_put = re.search(r"\bput\b", f.read()) is not None

    if xml and not has_put:

        model_xml = stdout

        if not has_xmllint():
            pytest.skip("xmllint not available")

        # validate the XML
        args = ["xmllint", "--relaxng", VERIFIER_RNG, "--noout", "-"]
        ret, stdout, stderr = run(args, model_xml)
        assert (
            ret == 0
        ), "Failed to XML-validate machine reachable output:\n{}{}".format(
            stdout, stderr
        )


# a model without any comments
NONE = """
var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  x := !x;
end;
"""

# a model with a single line comment
SINGLE = """
var
  x: boolean;

startstate begin
  x := -- hello world
  true;
end;

rule begin
  x := !x;
end;
"""

# a model with a multiline comment
MULTILINE = """
var
  x: boolean;

startstate begin
  x := /* hello world */ true;
end;

rule begin
  x := !x;
end;
"""

# a model with a single line comment terminated by EOF, not \n
SINGLE_EOF = """
var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  x := !x;
end; -- hello world"""

# a model with a multiline comment terminated by EOF, not */
MULTILINE_EOF = """
var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  x := !x;
end; /* hello world """

# comments inside strings that should be ignored
STRING = """
var
  x: boolean;

startstate "hello -- world" begin
  x := true;
end;

rule "hello /* world */" begin
  x := !x;
end;
"""

# a model with a mixture
MIX = """
var
  x: boolean;

startstate begin
  x := -- hello world
  /* hello world */true;
end; -- hello /* hello world */
/* hello -- hello */
rule begin
  x := !x;
end;
"""


@pytest.mark.skipif(
    shutil.which("murphi-comment-ls") is None, reason="murphi-comment-ls not found"
)
@pytest.mark.parametrize(
    "model,expectation",
    (
        (NONE, ""),
        (SINGLE, "6.8-21:  hello world\n"),
        (MULTILINE, "6.8-24:  hello world \n"),
        (SINGLE_EOF, "11.6-19:  hello world\n"),
        (MULTILINE_EOF, "11.6-20:  hello world \n"),
        (
            MIX,
            "6.8-21:  hello world\n7.3-19:  hello world \n8.6-31:  hello /* hello world */\n9.1-20:  hello -- hello \n",
        ),
    ),
)
def test_comment_parsing(model, expectation):
    """librumur should be able to retrieve comments accurately"""

    ret, stdout, stderr = run(["murphi-comment-ls"], model)

    assert ret == 0, "murphi-comment-ls failed"
    assert stderr == "", "murphi-comment-ls printed warnings/errors"

    assert stdout == expectation


def test_193():
    """
    This is a variant of 193.m that tests that we descend into TypeExprIDs when
    reordering record fields. It is hard to trigger a behaviour divergence that is
    exposed during checking, so instead we check the --debug output of rumur.

    See also https://github.com/Smattr/rumur/issues/193.
    """

    # a model containing a TypeExprID, itself referring to a record whose fields will be
    # reordered
    model = textwrap.dedent(
        """
    type
      t1: scalarset(4);

      t2: record
        a: boolean;
        b: t1;
        c: boolean;
      end;

    var x: t2;

    startstate begin
    end

    rule begin
    end
    """
    )

    # run Rumur in debug mode
    ret, _, stderr = run(["rumur", "--output", os.devnull, "--debug"], model)
    assert ret == 0, "rumur failed"

    # we should see the fields be reordered once due to encountering the original
    # TypeDecl (t2) and once due to the TypeExprID (the type of x)
    assert stderr.count("sorted fields {a, b, c} -> {a, c, b}") == 2


@pytest.mark.parametrize("arch", ("aarch64", "i386", "x86-64"))
def test_lock_freedom(arch):
    """
    Test that a compiled verifier does not depend on libatomic.

    Uses of __atomic built-ins and C11 atomics can sometimes cause the compiler to emit
    calls to libatomic instead of inline instructions. This is a problem because we use
    these in the verifier to implement lock-free algorithms, while the libatomic
    implementations take locks, defeating the purpose of using them. This test checks
    that we end up with no libatomic calls in the compiled verifier.
    """

    if arch == "aarch64":
        # this variant is only relevant on ≥ARMv8.1a
        argv = [
            cc(),
            "-std=c11",
            "-march=armv8.1-a",
            "-x",
            "c",
            "-",
            "-o",
            os.devnull,
        ]
        ret, _, _ = run(argv, "int main(void) { return 0; }")
        if ret != 0:
            pytest.skip("only relevant for ≥ARMv8.1a machines")

        cflags = ["-march=armv8.1-a"]

    elif arch == "i386":
        if platform.machine() not in ("amd64", "x86_64"):
            pytest.skip("not relevant on non-x86-64 machines")
        # check that we have a multilib compiler capable of targeting i386
        argv = [cc(), "-std=c11", "-m32", "-o", os.devnull, "-x", "c", "-"]
        program = textwrap.dedent(
            """\
        #include <stdio.h>
        int main(void) {
          printf("hello world\\n");
          return 0;
        }
        """
        )
        ret, _, _ = run(argv, program)
        if ret != 0:
            pytest.skip("compiler cannot target 32-bit code")

        cflags = ["-m32"]

    else:
        assert arch == "x86-64"
        if platform.machine() not in ("amd64", "x86_64"):
            pytest.skip("not relevant on non-x86-64 machines")

        cflags = ["-mcx16"]

    # generate a checker for a simple model
    model = "var x: boolean; startstate begin x := false; end; rule begin x := !x; end;"
    argv = ["rumur", "--output", "/dev/stdout"]
    ret, model_c, stderr = run(argv, model)
    assert ret == 0, "call to rumur failed: {}".format(stderr)

    # compile it to assembly
    argv = [
        cc(),
        "-O3",
        "-std=c11",
        "-x",
        "c",
        "-",
        "-S",
        "-o",
        "/dev/stdout",
    ] + cflags
    ret, model_s, stderr = run(argv, model_c)
    assert ret == 0, "compilation failed: {}".format(stderr)

    # check for calls to libatomic functions
    assert (
        "__atomic_" not in model_s
    ), "libatomic calls in generated code were not optimised out"


def test_murphi2murphi_decompose_array():
    """test array comparison decomposition"""

    # model from the test directory involving an array comparison
    model = Path(__file__).parent / "compare-array.m"
    assert model.exists()

    # use the complex comparison decomposition to explode the array comparison in this
    # model
    ret, transformed, _ = run(
        ["murphi2murphi", "--decompose-complex-comparisons", model]
    )
    assert ret == 0

    # the comparisons should have been decomposed into element-wise comparison
    assert re.search(r"\bx\[i0\] = y\[i0\]", transformed)
    assert re.search(r"\bx\[i0\] != y\[i0\]", transformed)

    # the generated model also should be valid syntax for Rumur
    ret, _, _ = run(["rumur", "--output", os.devnull], transformed)
    assert ret == 0


def test_murphi2murphi_decompose_record():
    """test record comparison decomposition"""

    # model from the test directory involving a record comparison
    model = Path(__file__).parent / "compare-record.m"
    assert model.exists()

    # use the complex comparison decomposition to explode the record comparison in this
    # model
    ret, transformed, _ = run(
        ["murphi2murphi", "--decompose-complex-comparisons", model]
    )
    assert ret == 0

    # the comparisons should have been decomposed into member-wise comparison
    assert re.search(r"\bx\.x = y\.x\b", transformed)
    assert re.search(r"\bx\.x != y\.x\b", transformed)

    # the generated model also should be valid syntax for Rumur
    ret, _, _ = run(["rumur", "--output", os.devnull], transformed)
    assert ret == 0


def test_murphi2murphi_explicit_semicolons():
    """test murphi2murphi’s ability to add semi-colons"""

    # pick an arbitrary test model that omits semi-colons
    model = Path(__file__).parent / "assume-statement.m"
    assert model.exists()

    # use the explicit-semicolons pass to add semi-colons
    ret, transformed, _ = run(["murphi2murphi", "--explicit-semicolons", model])
    assert ret == 0

    # we should now find the variable definition and both rules in this model are
    # semi-colon terminated
    assert re.search(r"\bx: 0 \.\. 2;$", transformed, re.MULTILINE)
    assert len(re.findall(r"^end;$", transformed, re.MULTILINE)) == 2

    # the generated model also should be valid syntax for Rumur
    ret, _, _ = run(["rumur", "--output", os.devnull], transformed)
    assert ret == 0


def test_murphi2murphi_remove_liveness():
    """test murphi2murphi’s ability to remove liveness properties"""

    # an arbitrary test model that has a liveness property
    model = Path(__file__).parent / "liveness-miss1.m"
    assert model.exists()

    # confirm it contains the liveness property we expect
    with open(str(model), "rt", encoding="utf-8") as f:
        assert 'liveness "x is 10" x = 10' in f.read()

    # use the remove-liveness pass to remove the property
    ret, transformed, _ = run(["murphi2murphi", "--remove-liveness", model])
    assert ret == 0

    # now confirm the property is no longer present
    assert 'liveness "x is 10" x = 10' not in transformed

    # the generated model also should be valid syntax for Rumur
    ret, _, _ = run(["rumur", "--output", os.devnull], transformed)
    assert ret == 0


def test_murphi2murphi_switch_to_if_const_enum():
    """test murphi2murphi’s ability to transform switches to if statements"""

    # model from the test directory involving some switch examples
    model = Path(__file__).parent / "const-enum.m"
    assert model.exists()

    # use the switch-to-if pass to remove the switch statements
    ret, transformed, _ = run(["murphi2murphi", "--switch-to-if", model])
    assert ret == 0

    # we should now be able to find some if statements in the model
    assert re.search(r"\bif X = A then\b", transformed)
    assert re.search(r"\bif X = X then\b", transformed)

    # the generated model also should be valid syntax for Rumur
    ret, _, _ = run(["rumur", "--output", os.devnull], transformed)
    assert ret == 0


def test_murphi2murphi_switch_to_if_nested():
    """test whether murphi2murphi can transform nested switches"""

    # model from the test directory involving some switch examples
    model = Path(__file__).parent / "switch-nested.m"
    assert model.exists()

    # use the switch-to-if pass to remove the switch statements
    ret, transformed, _ = run(["murphi2murphi", "--switch-to-if", model])
    assert ret == 0

    # we should now be able to find some if statements in the model
    assert re.search(r"\bif \(?x = 0\)? | \(?x = 1\)?", transformed)
    assert re.search(r"\bif x = 0 then\b", transformed)

    # the generated model also should be valid syntax for Rumur
    ret, _, _ = run(["rumur", "--output", os.devnull], transformed)
    assert ret == 0


def test_murphi2murphi_switch_to_if1():
    """conversion of switches to if statements"""

    # model from the test directory involving some switch examples
    model = Path(__file__).parent / "switch-stmt1.m"
    assert model.exists()

    # use the switch-to-if pass to remove the switch statements
    ret, transformed, _ = run(["murphi2murphi", "--switch-to-if", model])
    assert ret == 0

    # we should now be able to find some if statements in the model
    assert re.search(r"\bif x = 1 then\b", transformed)
    assert re.search(r"\belsif \(?x = 3\)? | \(?x = 4\)? then\b", transformed)

    # the generated model also should be valid syntax for Rumur
    ret, _, _ = run(["rumur", "--output", os.devnull], transformed)
    assert ret == 0


def test_murphi2murphi_switch_to_if2():
    """more conversion of switches to if statements"""

    # model from the test directory involving some switch examples
    model = Path(__file__).parent / "switch-stmt2.m"
    assert model.exists()

    # use the switch-to-if pass to remove the switch statements
    ret, transformed, _ = run(["murphi2murphi", "--switch-to-if", model])
    assert ret == 0

    # we should now be able to find some if statements in the model
    assert re.search(r"\bif x = y then\b", transformed)
    assert re.search(r"\belsif x = 5 then\b", transformed)

    # the generated model also should be valid syntax for Rumur
    ret, _, _ = run(["rumur", "--output", os.devnull], transformed)
    assert ret == 0


def test_murphi2murphi_unicode_div():
    """murphi2murphi’s ability to handle ÷"""

    # model from the test directory involving a unicode division
    model = Path(__file__).parent / "unicode-div.m"
    assert model.exists()

    # use the ASCII transformation to remove ÷
    ret, transformed, _ = run(["murphi2murphi", "--to-ascii", model])
    assert ret == 0

    # the ÷ operator should have become /
    assert re.search(r"\bx := 2 / x\b", transformed)
    assert "÷" not in transformed

    # the generated model also should be valid syntax for Rumur
    ret, _, _ = run(["rumur", "--output", os.devnull], transformed)
    assert ret == 0


def test_murphi2murphi_unicode_div2():
    """murphi2murphi’s ability to handle ∕"""

    # model from the test directory involving a unicode division
    model = Path(__file__).parent / "unicode-div2.m"
    assert model.exists()

    # use the ASCII transformation to remove ∕
    ret, transformed, _ = run(["murphi2murphi", "--to-ascii", model])
    assert ret == 0

    # the ∕ operator should have become /
    assert re.search(r"\bx := 2 / x\b", transformed)
    assert "∕" not in transformed

    # the generated model also should be valid syntax for Rumur
    ret, _, _ = run(["rumur", "--output", os.devnull], transformed)
    assert ret == 0


def test_murphi2murphi_unicode_mul():
    """murphi2murphi’s ability to handle ×"""

    # model from the test directory involving a unicode multiplication
    model = Path(__file__).parent / "unicode-mul.m"
    assert model.exists()

    # use the ASCII transformation to remove ×
    ret, transformed, _ = run(["murphi2murphi", "--to-ascii", model])
    assert ret == 0

    # the × operator should have become *
    assert re.search(r"\bx := 2 \* x\b", transformed)
    assert "×" not in transformed

    # the generated model also should be valid syntax for Rumur
    ret, _, _ = run(["rumur", "--output", os.devnull], transformed)
    assert ret == 0


def test_murphi2murphi_unicode_sub():
    """murphi2murphi’s ability to handle −"""

    # model from the test directory involving a unicode subtraction
    model = Path(__file__).parent / "unicode-sub.m"
    assert model.exists()

    # use the ASCII transformation to remove −
    ret, transformed, _ = run(["murphi2murphi", "--to-ascii", model])
    assert ret == 0

    # the − operator should have become -
    assert re.search(r"\bx := 1 - x\b", transformed)
    assert "−" not in transformed

    # the generated model also should be valid syntax for Rumur
    ret, _, _ = run(["rumur", "--output", os.devnull], transformed)
    assert ret == 0


def test_murphi2murphi_unicode_to_ascii():
    """murphi2murphi’s ability to handle ≔"""

    # model from the test directory involving a record comparison
    model = Path(__file__).parent / "unicode-assignment.m"
    assert model.exists()

    # use the ASCII transformation to remove ≔
    ret, transformed, _ = run(["murphi2murphi", "--to-ascii", model])
    assert ret == 0

    # the ≔ operator should have become :=
    assert re.search(r"\bx := true\b", transformed)
    assert re.search(r"\bx := !x\b", transformed)

    # the generated model also should be valid syntax for Rumur
    ret, _, _ = run(["rumur", "--output", os.devnull], transformed)
    assert ret == 0


def test_rumur_run_model():
    """test that rumur-run can check a basic model"""

    rumur_run = Path(__file__).absolute().parents[1] / "rumur/src/rumur-run"

    model = """
    var
      x: boolean;

    startstate begin
      x := true;
    end;

    rule begin
      x := !x;
    end;
    """

    ret, _, _ = run([sys.executable, rumur_run], model)
    assert ret == 0


def test_rumur_run_version():
    """basic test that rumur-run can execute successfully"""

    rumur_run = Path(__file__).absolute().parents[1] / "rumur/src/rumur-run"

    ret, _, _ = run([sys.executable, rumur_run, "--version"])
    assert ret == 0


@pytest.mark.skipif(shutil.which("strace") is None, reason="strace not available")
def test_strace_sandbox(tmp_path):
    """
    run a sandboxed checker under strace

    When the Linux seccomp sandbox causes the checker to terminate because it made an
    unauthorised system call, it is difficult to debug what happened without stracing
    the process to see the denied system call. This test case automates this work flow.
    If the checker runs fine with the sandbox enabled, this test case is irrelevant.
    However, if the basic-sandbox.m test fails, this test will hopefully automatically
    diagnose the failure. The purpose of this existing within the test suite itself is
    to debug failures that occur within a CI environment you do not have access to or
    cannot easily replicate, like the Debian auto builders.
    """

    if not has_sandbox():
        pytest.skip("seccomp sandboxing not supported")

    # create a basic model
    model_m = tmp_path / "model.m"
    with open(str(model_m), "wt", encoding="utf-8") as f:
        f.write(
            textwrap.dedent(
                """\
        var
          x: boolean;

        startstate begin
          x := true;
        end;

        rule begin
          x := !x;
        end;
        """
            )
        )

    # generate a sandboxed checker
    model_c = tmp_path / "model.c"
    ret, _, stderr = run(
        ["rumur", "--sandbox=on", "--output={}".format(model_c), model_m]
    )
    assert ret == 0, "rumur failed: {}".format(stderr)

    cflags = ["-std=c11"]
    ldflags = ["-lpthread"]

    # check if the compiler supports -march=native
    if has_march_native():
        cflags += ["-march=native"]

    # check if the compiler supports -mcx16
    if has_mcx16():
        cflags += ["-mcx16"]

    # check if we need libatomic support
    if needs_libatomic():
        ldflags += ["-latomic"]

    # compile the sandboxed checker
    model_exe = tmp_path / "model.exe"
    ret, _, stderr = run([cc()] + cflags + [model_c, "-o", model_exe] + ldflags)
    assert ret == 0, "C compilation failed: {}".format(stderr)

    # run the model under strace
    ret, stdout, stderr = run(["strace", model_exe])
    assert ret == 0, "model failed: {}{}".format(stdout, stderr)

    # if we did not yet see a failure, test to see if the sandbox also permits anything
    # extra we do with --debug

    # generate a sandboxed checker with debugging enabled
    ret, _, stderr = run(
        ["rumur", "--sandbox=on", "--debug", "--output={}".format(model_c), model_m]
    )
    assert ret == 0, "rumur failed: {}".format(stderr)

    # compile the sandboxed checker
    ret, _, stderr = run([cc()] + cflags + [model_c, "-o", model_exe] + ldflags)
    assert ret == 0, "C compilation failed: {}".format(stderr)

    # run the model under strace
    ret, stdout, stderr = run(["strace", model_exe])
    assert ret == 0, "model failed: {}{}".format(stdout, stderr)


def test_murphi2uclid_n():
    """
    `murphi2uclid` previously had a bug wherein the `-n` command-line option was not
    accepted. The following tests whether this bug has been reintroduced.
    """

    model = textwrap.dedent(
        """\
    const N: 2;
    var x: 0 .. 1;
    startstate begin
      x := 0;
    end;
    rule begin
      x := 1 - x;
    end;
    """
    )

    # translate a model to Uclid5 requesting a non-default bit-vector type
    ret, uclid, stderr = run(["murphi2uclid", "-n", "bv64"], model)
    assert ret == 0, "murphi2uclid failed: {}".format(stderr)

    # this should have been used for (at least) the constant
    assert (
        re.search(r"\bconst\s+N\s*:\s*bv64\b", uclid) is not None
    ), "Numeric type 'bv64' not used in output:"


def test_murphi2uclid_numeric_type():
    """
    `murphi2uclid` previously had a bug wherein the `--numeric-type` command-line option
    was ignored. The following tests whether this bug has been reintroduced.
    """

    model = textwrap.dedent(
        """\
    const N: 2;
    var x: 0 .. 1;
    startstate begin
      x := 0;
    end;
    rule begin
      x := 1 - x;
    end;
    """
    )

    # translate a model to Uclid5 requesting a non-default bit-vector type
    ret, uclid, stderr = run(["murphi2uclid", "--numeric-type=bv64"], model)
    assert ret == 0, "murphi2uclid failed: {}".format(stderr)

    # this should have been used for (at least) the constant
    assert (
        re.search(r"\bconst\s+N\s*:\s*bv64\b", uclid) is not None
    ), "Numeric type 'bv64' not used in output:"


def test_stdlib_export():
    """
    the last version of the standard library should be exported under a stable name
    """

    # find the last release version
    version = None
    changelog = Path(__file__).absolute().parents[1] / "CHANGELOG.rst"
    with open(str(changelog), "rt", encoding="utf-8") as f:
        for line in f:
            m = re.match(r"(?P<version>v\d{4}\.\d{2}\.\d{2})$", line)
            if m is not None:
                version = m.group("version")
                break
    assert version is not None, "could not determine last release version"

    # find the associated standard library snapshot
    snapshot = Path(__file__).absolute().parents[1] / "share" / version
    assert snapshot.exists(), "{} standard library not found".format(version)
    assert snapshot.is_dir(), "{} standard library snapshot is not a directory".format(
        version
    )

    # ensure it is installed
    cmakelists = Path(__file__).absolute().parents[1] / "share/CMakeLists.txt"
    seen = False
    with open(str(cmakelists), "rt", encoding="utf-8") as f:
        for line in f:
            m = re.search(r"\b(?P<version>v\d{4}\.\d{2}\.\d{2})\b", line)
            if m is not None and m.group("version") == version:
                seen = True
                break
    assert (
        seen
    ), "{} standard library snapshot does not appear to be installed by share/CMakeLists.txt".format(
        version
    )


DEBIAN_TESTS_ROOT = Path(__file__).absolute().parents[1] / "debian/tests"
"""
path to Debian autopkgtests
"""

DEBIAN_TESTS = [
    p.name
    for p in (DEBIAN_TESTS_ROOT.iterdir() if DEBIAN_TESTS_ROOT.exists() else ())
    if p.is_file() and (p.stat().st_mode & stat.S_IXUSR)
]


@pytest.mark.parametrize("testcase", DEBIAN_TESTS)
def test_debian_tests(testcase, tmp_path):
    """
    Run the Debian autopkgtests. These only exist on the `packaging/debian` branch.
    """

    exe = DEBIAN_TESTS_ROOT / testcase
    assert exe.exists()

    env = os.environ.copy()
    env["AUTOPKGTEST_TMP"] = str(tmp_path)

    proc = sp.run(
        [exe], stdout=sp.PIPE, stderr=sp.PIPE, universal_newlines=True, env=env
    )

    # skip `rumur-run` test that has complications because `rumur-run` is not executable
    # in the source tree
    if testcase == "rumur-run-model":
        assert (
            proc.returncode != 0
        ), "{} suprisingly found `rumur-run` executable: {}{}".format(
            str(exe), proc.stdout, proc.stderr
        )
        return

    assert proc.returncode == 0, "{} failed: {}{}".format(
        str(exe), proc.stdout, proc.stderr
    )
