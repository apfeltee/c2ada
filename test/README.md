
This directory isn't meant as a testsuite, but rather for (manual) testing that c2ada runs at all.

Usage example:

    # compile hello.c into ada code:
    ../c2ada hello.c
    # change into newly created bindings directory
    cd bindings
    # build hello (*might* not work. outdated ada spec perhaps?)
    # the flag '-aI..' instructs gnat to add the source directory to its search path
    gnat make hello -aI../..
    # since the previous command fails almost always, I've no idea what happens past that. sorry.

That's it, theoretically.

