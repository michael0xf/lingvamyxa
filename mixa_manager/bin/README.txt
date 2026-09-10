mixa_manager/bin - runnable artefacts, put here by hand

Mikhail's standing instruction, 2026-09-10: when mixa_manager produces anything
RUNNABLE, a copy goes here so it can be started without knowing where the build
put it.

What belongs here
    a built application, once one exists
    any standalone demo or tool worth starting by hand

What does not
    the selftest executables. run_mixa.ps1 builds those into build/mixa and
    runs them itself; they are a suite result, not something to launch.

Nothing here is committed - see .gitignore in this directory. Binaries are not
tracked in this repository, and a copy of a binary is still a binary. The build
that produced it is reproducible from the source and the translator identity
run_mixa.ps1 prints; the file itself is not evidence of anything.

Empty until the first version described in FIRST_VERSION.txt runs.
