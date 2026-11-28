#!/usr/bin/env python3

"""
tester for `rumur --print-rc`

Typically you want to run this with `-s --verbose` to spot anything odd.
"""

import os
import shutil
import subprocess
from pathlib import Path

import pexpect
import pytest


@pytest.fixture(scope="module")
def install(tmp_path_factory: pytest.TempPathFactory):
    """
    build and provide an installed Rumur

    Args:
        tmp_path_factory: A way to make managed temporary directories

    Yields:
        Root of a Rumur installation
    """

    # a temporary location to install into
    prefix = tmp_path_factory.mktemp("install")

    # build and install into that path
    source = Path(__file__).absolute().parents[1]
    build = tmp_path_factory.mktemp("build")
    d = "-DCMAKE_INSTALL_PREFIX={}".format(prefix)
    subprocess.run(["cmake", d, "-S", source, "-B", build], check=True)
    subprocess.run(["cmake", "--build", build], check=True)
    subprocess.run(["cmake", "--build", build, "--target", "install"], check=True)

    yield prefix


@pytest.mark.parametrize("shell", ("bash", "csh", "dash", "tcsh", "zsh"))
def test_print_rc(shell: str, install: Path, tmp_path: Path):
    """
    does `rumur --print-rc` generate a working shell snippet?

    Args:
        shell: Shell to test
        install: Root of a Rumur installation
        tmp_path: Managed scratch space
    """

    if shutil.which(shell) is None:
        pytest.skip("{} not available".format(shell))

    # locate `rumur`
    rumur = install / "bin/rumur"
    assert rumur.exists(), "installed `rumur` not found"
    assert shutil.which("rumur") != str(
        rumur
    ), "installed `rumur` is default Rumur, so test will not validate what it aims to"

    # generate an RC file
    rcfile = tmp_path / "rcfile"
    with open(rcfile, "wt", encoding="utf-8") as rc:
        subprocess.run([rumur, "--print-rc"], stdout=rc, check=True)

    # suppress loading of any shell user configuration
    args = []
    if shell == "bash":
        args += ["--rcfile", "/dev/null"]
    if shell == "zsh":
        args += ["--no-rcs"]

    # the shell should not already be configured with our installed Rumur
    p = subprocess.run(
        [shell] + args + ["-c", "which rumur"],
        stdout=subprocess.PIPE,
        check=False,
        text=True,
    )
    assert p.returncode != 0 or p.stdout.strip() != str(
        rumur
    ), "installed `rumur` is already known by this shell"

    # we should be able to source the RC file without error
    subprocess.run([shell] + args + ["-c", 'source "{}"'.format(rcfile)], check=True)

    # now we are ready to confirm the RC file actually configures what we expect

    # set a recognisable prompt we can `expect`
    env = os.environ.copy()
    env["PS1"] = "<PROMPT> "

    sh = pexpect.spawn(shell, args, timeout=5, env=env)
    sh.expect("<PROMPT> ")

    # configure our installed Rumur
    sh.sendline('source "{}"'.format(rcfile))
    sh.expect("<PROMPT> ")

    # can we now find `rumur`?
    sh.sendline("which rumur")
    sh.expect(str(rumur))
    sh.expect("<PROMPT> ")

    # can we find other Rumur tools?
    sh.sendline("which murphi2c")
    sh.expect(str(install / "bin/murphi2c"))
    sh.expect("<PROMPT> ")

    # can we compile a program using librumur?
    program = (
        "#include <rumur/rumur.h>\nint main(void) { return rumur_get_version() != 0; }"
    )
    cc_src = tmp_path / "main.cc"
    cc_src.write_text(program, encoding="utf-8")
    cxx = os.environ.get("CXX", shutil.which("c++"))
    assert cxx is not None, "C++ compiler not found"
    a_out = tmp_path / "a.out"
    sh.sendline("{} -std=c++11 {} -o {} -lrumur && echo OK".format(cxx, cc_src, a_out))
    sh.expect("OK")
    sh.expect("<PROMPT> ")

    # can we run this program?
    sh.sendline("{} && echo OK".format(a_out))
    sh.expect("OK")
    sh.expect("<PROMPT> ")

    sh.sendeof()
    sh.wait()
