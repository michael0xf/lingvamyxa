Mixa Manager - implementation on the stable L1
==============================================

Source model: Mixa_Manager_model.txt (draft 0.4) at the repository root.

The one architectural decision, and why
---------------------------------------
The model is written in Qt/C++ terms: classes, signals, QProcess, a Qt
renderer. L1 emits ANSI C99. So the boundary is not a matter of taste, it is
fixed by what L1 can express:

  IN L1, because it is pure data and logic with no OS and no Qt:
      core/      Cell, Style, Palette, TextRect, OverlayRect, VisualRect,
                 Selection
      input/     InputNormalizer, InputEvent, OwnCommandMapper,
                 TerminalInputEncoder, MouseEncoder
      terminal/  TerminalProfile, TerminalKey, TerminalModes - tables and rules
      overlay/   OverlayCommands - a command list, not a rasteriser
      ui/        panel, viewer, editor and command-line LOGIC over TextRect

  BEHIND A C ABI, declared here with external: and a hand-written header:
      render/    QtTextRenderer, OverlayRenderer, Compositor, FontMetrics,
                 GlyphCache
      backend/   UnixPtyBackend, WindowsConptyBackend, PipeProcessBackend
      input/     QtEventCollector, ClipboardAdapter - the collection side only
      terminal/  VtermAdapter - libvterm itself

Section 0.3 is what makes this work: every producer ends at the same
TextRect + OverlayRect, and "the renderer does not need to know which producer
created the text screen". So the renderer can be the last thing built, and
everything above it is testable without it.

Build
-----
Translated by the STABLE L1, not the live one:

    stg\l1_baseline\build\l1trans\gen2\l1trans.exe

That binary is not in git; it is produced by stg\l1_baseline\gate.ps1.

This directory is deliberately SELF-CONTAINED: no predef of l1src, only its own
headers and libc. That is what lets it sit at the repository root while being
built by a translator from the frozen tree, and it keeps it independent of which
L1 tree happens to be live. Paths inside .lm1 are written relative to the
repository root, which is the build root here.

Milestone 1 - core, with no Qt at all
-------------------------------------
Model section 2 lists TestProducer: synthetic text screens for tests, demos and
renderer debugging. That is the first thing to build, because it needs nothing
from the platform and it pins the contract every other producer must meet.

    mixa_core.h          Cell, TextRect
    mixa_text_rect.lm1   allocate, address a cell, fill, write text, dump
    tests\               selftest over a dumped screen

A dumped screen is plain text, so the whole of milestone 1 is verifiable by
string comparison and needs neither a window nor a font.

House rules inherited from the project
--------------------------------------
Valued aggregates are banned, L1 and L2 alike. Cells are reached by pointer:
mixa_cell_at(rect, row, col) returns an address, never a cell by value. The
explicit C door c. is for extravagant exceptions only.

Cells live in one contiguous row-major block for the same reason branch storage
does (ABI 14.1): stable addresses, order is the index, and a copy is a block
copy. The block is sized at open and does not grow; a resize builds a new rect,
which is also what model section 13 describes.
