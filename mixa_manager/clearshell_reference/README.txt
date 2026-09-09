ClearShell reference source
Read-only. This is the PORT TARGET, not our code.

Copied from
    C:\Nyasha_Planet\git\clear_shell\ClearShell\app\src\main\java
on 2026-09-09, so that everyone working on mixa_manager can read it without
needing the Android project. Java sources only.

DELIBERATELY NOT COPIED: AndroidManifest.xml and res/ - manifests, layouts,
drawables, mipmaps and value files are Android service configuration and say
nothing about the behaviour being ported.

NEVER EDIT ANYTHING IN THIS DIRECTORY. It is a snapshot of someone else's
working project. Fixing something here fixes nothing, and a divergence between
this copy and the real project is worse than not having the copy - if a change
belongs upstream, say so rather than making it here.

If it is refreshed, refresh it wholesale from the same path rather than
patching, and say so in the commit.

Where the port's conclusions are recorded, with references into this tree:
mixa_manager/PORT_OF_CLEARSHELL.txt.

One fact worth keeping from a read_me.txt in mtk/map that was NOT copied: that
package is a published library in its own right - "OverlappingTreesPlainMap",
shipped as mtkmap-*.jar - with MapReader, MapWriter, Log and BaseFactoryImpl as
its example utilities rather than as part of the shell. So treat mtk/map and
com/mtk/map as a LIBRARY the shell uses, not as shell code: it is where Item,
Tag, Array and the tables live.

That file was left out because the rest of it is the author's contact details,
which do not belong in a public repository.
