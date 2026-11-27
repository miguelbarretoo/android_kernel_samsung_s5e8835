#!/usr/bin/env python3
"""Pack the modules a model lists, and their dependencies, GKI-style.

usage: stage.py <installed /lib/modules/<release>> <model .load list> <out dir>

The output is flat, as Android first-stage init expects in /lib/modules:
every .ko at the top level, modules.load with the listed modules in order,
and modules.dep / modules.softdep / modules.alias rewritten to bare names.
"""
import os
import shutil
import sys


def parse_dep(path):
    dep = {}
    with open(path) as f:
        for line in f:
            mod, _, rest = line.partition(":")
            dep[os.path.basename(mod.strip())] = (mod.strip(), rest.split())
    return dep


def main():
    src, listing, out = sys.argv[1:4]
    dep = parse_dep(os.path.join(src, "modules.dep"))

    wanted = []
    with open(listing) as f:
        for line in f:
            name = line.split("#", 1)[0].strip()
            if not name:
                continue
            if not name.endswith(".ko"):
                name += ".ko"
            if name not in dep:
                sys.exit(f"stage.py: {name} is listed but was not built")
            wanted.append(name)

    packed = []
    todo = list(wanted)
    while todo:
        name = todo.pop()
        if name in packed:
            continue
        packed.append(name)
        todo.extend(os.path.basename(d) for d in dep[name][1])

    os.makedirs(out, exist_ok=True)
    for name in packed:
        shutil.copy2(os.path.join(src, dep[name][0]), os.path.join(out, name))

    with open(os.path.join(out, "modules.dep"), "w") as f:
        for name in sorted(packed):
            deps = " ".join(os.path.basename(d) for d in dep[name][1])
            f.write(f"{name}:{' ' + deps if deps else ''}\n")
    with open(os.path.join(out, "modules.load"), "w") as f:
        f.write("".join(n + "\n" for n in wanted))
    for extra in ("modules.softdep", "modules.alias"):
        path = os.path.join(src, extra)
        if os.path.exists(path):
            shutil.copy2(path, os.path.join(out, extra))


if __name__ == "__main__":
    main()
