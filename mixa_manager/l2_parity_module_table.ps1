# Per-module data table for run_mixa_l2_parity.ps1 (runner-uniformity
# ticket, step 2 of lingvamyxa-d6's ordering: "run_mixa_l2_parity.ps1
# -Module <name> for the 51, data-driven"). Landing in verified
# batches, per d6's gate: a batch's old per-module scripts are deleted
# only after (1) the new runner's pre-barrier verdict matches the old
# script's, row by row, on main, and (2) on d6's integration branch,
# the new runner reproduces PASS with the same trace-diff outcome for
# whichever of that batch's modules actually pass there.
#
# Two modules are deliberately NOT in this table and are not planned
# to ever be, per the inventory's own falsifying-check conclusion
# (RUNNER_INVENTORY_20260913.txt):
#   - app_main: build-only, no second execution, no trace diff, its
#     own 13+-object real dependency chain reused verbatim from
#     run_win32_smoke_selftest.ps1. Different CONTROL FLOW, not just
#     different data.
#   - app_controller: link-argument-list shape ($l2LinkArgs /
#     $oracleLinkArgs PowerShell arrays) instead of every other
#     runner's inline argument string. Also currently blocked on a
#     compiler gap regardless. Revisit once/if that shape is worth
#     normalizing on its own.
#
# One module found while extracting batch 1's own data and EXCLUDED
# from batch 1 for the same reason: audio_mp3. Its real .lm1 needs 22
# imports in one translation unit and the pinned stable compiler caps
# at 16 -- so its ORACLE side must use a separate CANDIDATE l1trans.exe
# (own hash-pinned, read-only, ticket 20260912-084943), not the shared
# $L1Trans every other module's oracle side uses. That is a real,
# separate translator selection, not a value substituted into an
# otherwise-identical shape -- so it stays its own script for now.
# (Flagged to lingvamyxa-d6; revisit if it turns out this is common
# enough to warrant an OracleTranslator override field.)
#
# Schema per module entry:
#   HeaderTrans = @(@{Src="mixa_manager\...lm1"; Out="....lm1.h"}, ...)
#     Step 0, in order. Translator is always the pinned stable $L1Trans.
#   Probe = $null, or @{ RealC = "tests\mixa_X_abi_probe_real.c";
#     L2C = "tests\mixa_X_abi_probe_l2.c"; Libs = "" }
#     Probe .c paths are relative to mixa_manager\. Libs (if non-empty)
#     is appended to BOTH probe compiles (file_win32's own probe needs
#     -lkernel32 to link).
#   OracleDeps / L2Deps = @('mixa_dep_stem', ...)
#     Real dependency modules whose .lm1 gets translated (cwd=RepoRoot,
#     plain -I RepoRoot, no runtime headers) and compiled, then linked
#     into the oracle exe / L2 exe respectively. Often the same list,
#     sometimes not (a module's own oracle .lm1 may predef a
#     dependency's full body directly, while the L2 header predefs
#     only its header -- see RUNNER_INVENTORY's own note on this).
#   RuntimeTrio = $true/$false -- calls Add-L2RuntimeSupport before the
#     L2-side compile/link when true.
#   Fixture = 'none' | 'shared-single' | 'shared-separate'
#     'none': both sides run with "" (no fixture argument).
#     'shared-single': one $FixtureDir, same literal path given to
#       both sides (no normalization needed -- audio_mp3's own sibling
#       audio family modules use this. NOTE: audio_mp3 itself is
#       excluded, see above).
#     'shared-separate': two independently-created, independently-
#       populated dirs (oracle's made up front, L2's made only inside
#       the success branch) -- fixture-root trace normalization
#       applies (ticket 20260914-001000's own reasoning: sharing one
#       root leaks state between runs).
#   LinkLibs = '' or e.g. '-lkernel32' -- appended to both the oracle
#     and the L2 final link command.
#   BarrierSet = 'UI' or 'UIA' -- which $KnownBarrier substrings apply
#     ('UI' = unknown foreign type + incompatible entry signature;
#     'UIA' = + unsupported own array declaration).

$ModuleTable = @{
    'app_window' = @{
        HeaderTrans = @(
            @{ Src = 'mixa_manager\mixa_app_window.h.lm1'; Out = 'mixa_app_window.lm1.h' }
            @{ Src = 'mixa_manager\mixa_tiles_l2.h.lm1'; Out = 'mixa_tiles_l2.lm1.h' }
            @{ Src = 'mixa_manager\mixa_button_dispatch_l2.h.lm1'; Out = 'mixa_button_dispatch_l2.lm1.h' }
            @{ Src = 'mixa_manager\mixa_app_window_l2.h.lm1'; Out = 'mixa_app_window_l2.lm1.h' }
        )
        Probe = @{ RealC = 'tests\mixa_app_window_abi_probe_real.c'; L2C = 'tests\mixa_app_window_abi_probe_l2.c'; Libs = '' }
        OracleDeps = @()
        L2Deps = @('mixa_buttons', 'mixa_draw')
        RuntimeTrio = $true
        Fixture = 'none'
        LinkLibs = ''
        BarrierSet = 'UI'
    }
    'backend_ctors_headless' = @{
        HeaderTrans = @(
            @{ Src = 'mixa_manager\mixa_backend_ctors_headless_l2.h.lm1'; Out = 'mixa_backend_ctors_headless_l2.lm1.h' }
        )
        Probe = $null
        OracleDeps = @('mixa_event_fifo', 'mixa_backend_headless')
        L2Deps = @('mixa_event_fifo', 'mixa_backend_headless')
        RuntimeTrio = $false
        Fixture = 'none'
        LinkLibs = ''
        BarrierSet = 'UIA'
    }
    'backend_headless' = @{
        HeaderTrans = @(
            @{ Src = 'mixa_manager\mixa_backend_headless_l2.h.lm1'; Out = 'mixa_backend_headless_l2.lm1.h' }
        )
        Probe = $null
        OracleDeps = @('mixa_event_fifo')
        L2Deps = @('mixa_event_fifo')
        RuntimeTrio = $false
        Fixture = 'none'
        LinkLibs = ''
        BarrierSet = 'UIA'
    }
    'backend_table' = @{
        HeaderTrans = @(
            @{ Src = 'mixa_manager\mixa_backend_table_l2.h.lm1'; Out = 'mixa_backend_table_l2.lm1.h' }
        )
        Probe = $null
        OracleDeps = @('mixa_event_fifo', 'mixa_backend_headless', 'mixa_backend_ctors_headless')
        L2Deps = @('mixa_event_fifo', 'mixa_backend_headless', 'mixa_backend_ctors_headless')
        RuntimeTrio = $false
        Fixture = 'shared-separate'
        LinkLibs = ''
        BarrierSet = 'UIA'
    }
    'copy' = @{
        HeaderTrans = @(
            @{ Src = 'mixa_manager\mixa_dir.h.lm1'; Out = 'mixa_dir.lm1.h' }
            @{ Src = 'mixa_manager\mixa_dir_win32.h.lm1'; Out = 'mixa_dir_win32.lm1.h' }
            @{ Src = 'mixa_manager\mixa_selection_walk.h.lm1'; Out = 'mixa_selection_walk.lm1.h' }
            @{ Src = 'mixa_manager\mixa_fileio_win32.h.lm1'; Out = 'mixa_fileio_win32.lm1.h' }
            @{ Src = 'mixa_manager\mixa_fileio.h.lm1'; Out = 'mixa_fileio.lm1.h' }
            @{ Src = 'mixa_manager\mixa_copy.h.lm1'; Out = 'mixa_copy.lm1.h' }
            @{ Src = 'mixa_manager\mixa_copy_l2.h.lm1'; Out = 'mixa_copy_l2.lm1.h' }
        )
        Probe = $null
        OracleDeps = @()
        L2Deps = @('mixa_selection_walk', 'mixa_fileio_win32')
        RuntimeTrio = $false
        Fixture = 'shared-separate'
        LinkLibs = ''
        BarrierSet = 'UI'
    }
    'remove' = @{
        HeaderTrans = @(
            @{ Src = 'mixa_manager\mixa_dir.h.lm1'; Out = 'mixa_dir.lm1.h' }
            @{ Src = 'mixa_manager\mixa_dir_win32.h.lm1'; Out = 'mixa_dir_win32.lm1.h' }
            @{ Src = 'mixa_manager\mixa_selection_walk.h.lm1'; Out = 'mixa_selection_walk.lm1.h' }
            @{ Src = 'mixa_manager\mixa_remove.h.lm1'; Out = 'mixa_remove.lm1.h' }
            @{ Src = 'mixa_manager\mixa_remove_l2.h.lm1'; Out = 'mixa_remove_l2.lm1.h' }
        )
        Probe = $null
        OracleDeps = @()
        L2Deps = @('mixa_selection_walk')
        RuntimeTrio = $false
        Fixture = 'shared-separate'
        LinkLibs = ''
        BarrierSet = 'UI'
    }
    'share_button' = @{
        HeaderTrans = @(
            @{ Src = 'mixa_manager\mixa_share_win32.h.lm1'; Out = 'mixa_share_win32.lm1.h' }
            @{ Src = 'mixa_manager\mixa_share.h.lm1'; Out = 'mixa_share.lm1.h' }
            @{ Src = 'mixa_manager\mixa_share_button.h.lm1'; Out = 'mixa_share_button.lm1.h' }
            @{ Src = 'mixa_manager\mixa_share_button_l2.h.lm1'; Out = 'mixa_share_button_l2.lm1.h' }
        )
        Probe = $null
        OracleDeps = @()
        L2Deps = @('mixa_share_action')
        RuntimeTrio = $true
        Fixture = 'shared-separate'
        LinkLibs = ''
        BarrierSet = 'UIA'
    }
    'share_win32' = @{
        HeaderTrans = @(
            @{ Src = 'mixa_manager\mixa_share_win32.h.lm1'; Out = 'mixa_share_win32.lm1.h' }
            @{ Src = 'mixa_manager\mixa_share.h.lm1'; Out = 'mixa_share.lm1.h' }
            @{ Src = 'mixa_manager\mixa_share_win32_l2.h.lm1'; Out = 'mixa_share_win32_l2.lm1.h' }
        )
        Probe = $null
        OracleDeps = @(@{ Stem = 'mixa_share_dep_oracle'; Src = 'mixa_manager\mixa_share.lm1' })
        L2Deps = @(@{ Stem = 'mixa_share_dep_l2'; Src = 'mixa_manager\mixa_share.lm1' })
        RuntimeTrio = $false
        Fixture = 'shared-separate'
        LinkLibs = '-lruntimeobject -luser32 -lole32'
        BarrierSet = 'UIA'
        # Real WinRT header (windows.applicationmodel.datatransfer.h)
        # only exists under the Windows SDK's own winrt include subtree,
        # not on gcc's normal search path -- the old script's own
        # -idirafter flag, reused verbatim (the SDK version string is
        # this machine's installed one, same as the old script's own
        # default parameter value).
        ExtraCompileFlags = '-idirafter "C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\winrt"'
    }
    'remove_confirm' = @{
        HeaderTrans = @(
            @{ Src = 'mixa_manager\mixa_dir.h.lm1'; Out = 'mixa_dir.lm1.h' }
            @{ Src = 'mixa_manager\mixa_dir_win32.h.lm1'; Out = 'mixa_dir_win32.lm1.h' }
            @{ Src = 'mixa_manager\mixa_selection_walk.h.lm1'; Out = 'mixa_selection_walk.lm1.h' }
            @{ Src = 'mixa_manager\mixa_file_manager.h.lm1'; Out = 'mixa_file_manager.lm1.h' }
            @{ Src = 'mixa_manager\mixa_remove.h.lm1'; Out = 'mixa_remove.lm1.h' }
            @{ Src = 'mixa_manager\mixa_fm_remove.h.lm1'; Out = 'mixa_fm_remove.lm1.h' }
            @{ Src = 'mixa_manager\mixa_app_window.h.lm1'; Out = 'mixa_app_window.lm1.h' }
            @{ Src = 'mixa_manager\mixa_remove_confirm.h.lm1'; Out = 'mixa_remove_confirm.lm1.h' }
            @{ Src = 'mixa_manager\mixa_remove_confirm_l2.h.lm1'; Out = 'mixa_remove_confirm_l2.lm1.h' }
        )
        Probe = @{ RealC = 'tests\mixa_remove_confirm_abi_probe_real.c'; L2C = 'tests\mixa_remove_confirm_abi_probe_l2.c'; Libs = '' }
        OracleDeps = @('mixa_fm_remove', 'mixa_app_window')
        L2Deps = @('mixa_fm_remove', 'mixa_app_window')
        RuntimeTrio = $false
        Fixture = 'shared-separate'
        LinkLibs = ''
        BarrierSet = 'UI'
    }
    'selection_walk' = @{
        HeaderTrans = @(
            @{ Src = 'mixa_manager\mixa_dir.h.lm1'; Out = 'mixa_dir.lm1.h' }
            @{ Src = 'mixa_manager\mixa_dir_win32.h.lm1'; Out = 'mixa_dir_win32.lm1.h' }
            @{ Src = 'mixa_manager\mixa_selection_walk.h.lm1'; Out = 'mixa_selection_walk.lm1.h' }
            @{ Src = 'mixa_manager\mixa_selection_walk_l2.h.lm1'; Out = 'mixa_selection_walk_l2.lm1.h' }
        )
        Probe = $null
        OracleDeps = @()
        L2Deps = @('mixa_dir_win32', 'mixa_selection')
        RuntimeTrio = $false
        Fixture = 'shared-separate'
        LinkLibs = ''
        BarrierSet = 'UI'
    }
    'cmdline_dispatch' = @{
        HeaderTrans = @(
            @{ Src = 'mixa_manager\mixa_process_marker.h.lm1'; Out = 'mixa_process_marker.lm1.h' }
            @{ Src = 'mixa_manager\mixa_cmdline_dispatch_l2.h.lm1'; Out = 'mixa_cmdline_dispatch_l2.lm1.h' }
        )
        Probe = @{ RealC = 'tests\mixa_cmdline_dispatch_abi_probe_real.c'; L2C = 'tests\mixa_cmdline_dispatch_abi_probe_l2.c'; Libs = '' }
        OracleDeps = @('mixa_process_marker', 'mixa_process_win32', 'mixa_file_win32')
        L2Deps = @('mixa_process_marker', 'mixa_process_win32', 'mixa_file_win32')
        RuntimeTrio = $true
        Fixture = 'shared-separate'
        LinkLibs = '-lkernel32'
        BarrierSet = 'UI'
    }
    'audio_mp3' = @{
        HeaderTrans = @(
            @{ Src = 'mixa_manager\mixa_dir_win32.h.lm1'; Out = 'mixa_dir_win32.lm1.h' }
            @{ Src = 'mixa_manager\mixa_dir.h.lm1'; Out = 'mixa_dir.lm1.h' }
            @{ Src = 'mixa_manager\mixa_audio_win32.h.lm1'; Out = 'mixa_audio_win32.lm1.h' }
            @{ Src = 'mixa_manager\mixa_audio.h.lm1'; Out = 'mixa_audio.lm1.h' }
            @{ Src = 'mixa_manager\mixa_audio_scan.h.lm1'; Out = 'mixa_audio_scan.lm1.h' }
            @{ Src = 'mixa_manager\mixa_audio_button.h.lm1'; Out = 'mixa_audio_button.lm1.h' }
            @{ Src = 'mixa_manager\mixa_app_window.h.lm1'; Out = 'mixa_app_window.lm1.h' }
            @{ Src = 'mixa_manager\mixa_audio_panel.h.lm1'; Out = 'mixa_audio_panel.lm1.h' }
            @{ Src = 'mixa_manager\mixa_audio_launch.h.lm1'; Out = 'mixa_audio_launch.lm1.h' }
            @{ Src = 'mixa_manager\mixa_audio_mp3.h.lm1'; Out = 'mixa_audio_mp3.lm1.h' }
            @{ Src = 'mixa_manager\mixa_audio_mp3_l2.h.lm1'; Out = 'mixa_audio_mp3_l2.lm1.h' }
        )
        Probe = $null
        OracleDeps = @()
        L2Deps = @()
        RuntimeTrio = $true
        Fixture = 'shared-separate'
        LinkLibs = ''
        BarrierSet = 'UIA'
    }
    'fm_copy' = @{
        HeaderTrans = @(
            @{ Src = 'mixa_manager\mixa_dir.h.lm1'; Out = 'mixa_dir.lm1.h' }
            @{ Src = 'mixa_manager\mixa_dir_win32.h.lm1'; Out = 'mixa_dir_win32.lm1.h' }
            @{ Src = 'mixa_manager\mixa_selection_walk.h.lm1'; Out = 'mixa_selection_walk.lm1.h' }
            @{ Src = 'mixa_manager\mixa_fileio_win32.h.lm1'; Out = 'mixa_fileio_win32.lm1.h' }
            @{ Src = 'mixa_manager\mixa_fileio.h.lm1'; Out = 'mixa_fileio.lm1.h' }
            @{ Src = 'mixa_manager\mixa_copy.h.lm1'; Out = 'mixa_copy.lm1.h' }
            @{ Src = 'mixa_manager\mixa_file_manager.h.lm1'; Out = 'mixa_file_manager.lm1.h' }
            @{ Src = 'mixa_manager\mixa_fm_copy.h.lm1'; Out = 'mixa_fm_copy.lm1.h' }
            @{ Src = 'mixa_manager\mixa_fm_copy_l2.h.lm1'; Out = 'mixa_fm_copy_l2.lm1.h' }
        )
        Probe = $null
        OracleDeps = @('mixa_event_fifo', 'mixa_backend_table', 'mixa_backend_headless', 'mixa_backend_ctors_headless', 'mixa_pump')
        L2Deps = @(
            @{ Stem = 'mixa_fm_copy_l2_link'; Src = 'mixa_manager\tests\mixa_fm_copy_l2_link.lm1' }
            'mixa_event_fifo', 'mixa_backend_table', 'mixa_backend_headless', 'mixa_backend_ctors_headless', 'mixa_pump'
        )
        RuntimeTrio = $true
        Fixture = 'shared-separate'
        LinkLibs = ''
        BarrierSet = 'UI'
    }
    'fm_remove' = @{
        HeaderTrans = @(
            @{ Src = 'mixa_manager\mixa_dir_win32.h.lm1'; Out = 'mixa_dir_win32.lm1.h' }
            @{ Src = 'mixa_manager\mixa_dir.h.lm1'; Out = 'mixa_dir.lm1.h' }
            @{ Src = 'mixa_manager\mixa_selection_walk.h.lm1'; Out = 'mixa_selection_walk.lm1.h' }
            @{ Src = 'mixa_manager\mixa_file_manager.h.lm1'; Out = 'mixa_file_manager.lm1.h' }
            @{ Src = 'mixa_manager\mixa_remove.h.lm1'; Out = 'mixa_remove.lm1.h' }
            @{ Src = 'mixa_manager\mixa_fm_remove.h.lm1'; Out = 'mixa_fm_remove.lm1.h' }
            @{ Src = 'mixa_manager\mixa_fm_remove_l2.h.lm1'; Out = 'mixa_fm_remove_l2.lm1.h' }
        )
        Probe = $null
        OracleDeps = @()
        L2Deps = @(
            @{ Stem = 'mixa_fm_remove_l2_link'; Src = 'mixa_manager\tests\mixa_fm_remove_l2_link.lm1' }
        )
        RuntimeTrio = $true
        Fixture = 'shared-separate'
        LinkLibs = ''
        BarrierSet = 'UI'
    }
    'buttons' = @{
        HeaderTrans = @(
            @{ Src = 'mixa_manager\mixa_tiles_l2.h.lm1'; Out = 'mixa_tiles_l2.lm1.h' }
            @{ Src = 'mixa_manager\mixa_button_dispatch_l2.h.lm1'; Out = 'mixa_button_dispatch_l2.lm1.h' }
            @{ Src = 'mixa_manager\mixa_buttons_l2.h.lm1'; Out = 'mixa_buttons_l2.lm1.h' }
        )
        Probe = @{ RealC = 'tests\mixa_buttons_abi_probe_real.c'; L2C = 'tests\mixa_buttons_abi_probe_l2.c'; Libs = '' }
        OracleDeps = @()
        L2Deps = @('mixa_tiles')
        RuntimeTrio = $true
        Fixture = 'none'
        LinkLibs = ''
        BarrierSet = 'UI'
    }
    'help' = @{
        HeaderTrans = @(
            @{ Src = 'mixa_manager\mixa_help_l2.h.lm1'; Out = 'mixa_help_l2.lm1.h' }
        )
        Probe = @{ RealC = 'tests\mixa_help_abi_probe_real.c'; L2C = 'tests\mixa_help_abi_probe_l2.c'; Libs = '' }
        OracleDeps = @('mixa_file_win32')
        L2Deps = @('mixa_file_win32')
        RuntimeTrio = $true
        Fixture = 'shared-separate'
        LinkLibs = '-lkernel32'
        BarrierSet = 'UI'
    }
    'app_path' = @{
        HeaderTrans = @(
            @{ Src = 'mixa_manager\mixa_app_path_l2.h.lm1'; Out = 'mixa_app_path_l2.lm1.h' }
        )
        Probe = @{ RealC = 'tests\mixa_app_path_abi_probe_real.c'; L2C = 'tests\mixa_app_path_abi_probe_l2.c'; Libs = '' }
        OracleDeps = @()
        L2Deps = @()
        RuntimeTrio = $true
        Fixture = 'none'
        LinkLibs = ''
        BarrierSet = 'UI'
    }
    'audio_button' = @{
        HeaderTrans = @(
            @{ Src = 'mixa_manager\mixa_audio_win32.h.lm1'; Out = 'mixa_audio_win32.lm1.h' }
            @{ Src = 'mixa_manager\mixa_audio.h.lm1'; Out = 'mixa_audio.lm1.h' }
            @{ Src = 'mixa_manager\mixa_audio_button.h.lm1'; Out = 'mixa_audio_button.lm1.h' }
            @{ Src = 'mixa_manager\mixa_audio_button_l2.h.lm1'; Out = 'mixa_audio_button_l2.lm1.h' }
        )
        Probe = $null
        OracleDeps = @()
        L2Deps = @()
        RuntimeTrio = $true
        Fixture = 'none'
        LinkLibs = ''
        BarrierSet = 'UIA'
    }
    'audio' = @{
        HeaderTrans = @(
            @{ Src = 'mixa_manager\mixa_audio_win32.h.lm1'; Out = 'mixa_audio_win32.lm1.h' }
            @{ Src = 'mixa_manager\mixa_audio.h.lm1'; Out = 'mixa_audio.lm1.h' }
            @{ Src = 'mixa_manager\mixa_audio_l2.h.lm1'; Out = 'mixa_audio_l2.lm1.h' }
        )
        Probe = $null
        OracleDeps = @()
        L2Deps = @()
        RuntimeTrio = $true
        Fixture = 'none'
        LinkLibs = ''
        BarrierSet = 'UIA'
    }
    'audio_launch' = @{
        HeaderTrans = @(
            @{ Src = 'mixa_manager\mixa_audio_win32.h.lm1'; Out = 'mixa_audio_win32.lm1.h' }
            @{ Src = 'mixa_manager\mixa_audio.h.lm1'; Out = 'mixa_audio.lm1.h' }
            @{ Src = 'mixa_manager\mixa_audio_button.h.lm1'; Out = 'mixa_audio_button.lm1.h' }
            @{ Src = 'mixa_manager\mixa_app_window.h.lm1'; Out = 'mixa_app_window.lm1.h' }
            @{ Src = 'mixa_manager\mixa_audio_panel.h.lm1'; Out = 'mixa_audio_panel.lm1.h' }
            @{ Src = 'mixa_manager\mixa_audio_launch.h.lm1'; Out = 'mixa_audio_launch.lm1.h' }
            @{ Src = 'mixa_manager\mixa_audio_launch_l2.h.lm1'; Out = 'mixa_audio_launch_l2.lm1.h' }
        )
        Probe = $null
        OracleDeps = @()
        L2Deps = @()
        RuntimeTrio = $true
        Fixture = 'none'
        LinkLibs = ''
        BarrierSet = 'UIA'
    }
    'audio_panel' = @{
        HeaderTrans = @(
            @{ Src = 'mixa_manager\mixa_audio_win32.h.lm1'; Out = 'mixa_audio_win32.lm1.h' }
            @{ Src = 'mixa_manager\mixa_audio.h.lm1'; Out = 'mixa_audio.lm1.h' }
            @{ Src = 'mixa_manager\mixa_audio_button.h.lm1'; Out = 'mixa_audio_button.lm1.h' }
            @{ Src = 'mixa_manager\mixa_app_window.h.lm1'; Out = 'mixa_app_window.lm1.h' }
            @{ Src = 'mixa_manager\mixa_audio_panel.h.lm1'; Out = 'mixa_audio_panel.lm1.h' }
            @{ Src = 'mixa_manager\mixa_audio_panel_l2.h.lm1'; Out = 'mixa_audio_panel_l2.lm1.h' }
        )
        Probe = $null
        OracleDeps = @()
        L2Deps = @()
        RuntimeTrio = $true
        Fixture = 'none'
        LinkLibs = ''
        BarrierSet = 'UIA'
    }
    'audio_scan' = @{
        HeaderTrans = @(
            @{ Src = 'mixa_manager\mixa_audio_win32.h.lm1'; Out = 'mixa_audio_win32.lm1.h' }
            @{ Src = 'mixa_manager\mixa_audio.h.lm1'; Out = 'mixa_audio.lm1.h' }
            @{ Src = 'mixa_manager\mixa_audio_scan.h.lm1'; Out = 'mixa_audio_scan.lm1.h' }
            @{ Src = 'mixa_manager\mixa_audio_scan_l2.h.lm1'; Out = 'mixa_audio_scan_l2.lm1.h' }
            @{ Src = 'mixa_manager\mixa_dir_win32.h.lm1'; Out = 'mixa_dir_win32.lm1.h' }
            @{ Src = 'mixa_manager\mixa_dir.h.lm1'; Out = 'mixa_dir.lm1.h' }
        )
        Probe = $null
        OracleDeps = @()
        L2Deps = @()
        RuntimeTrio = $true
        Fixture = 'shared-separate'
        LinkLibs = ''
        BarrierSet = 'UIA'
    }
    'audio_win32' = @{
        HeaderTrans = @(
            @{ Src = 'mixa_manager\mixa_audio_win32.h.lm1'; Out = 'mixa_audio_win32.lm1.h' }
            @{ Src = 'mixa_manager\mixa_audio.h.lm1'; Out = 'mixa_audio.lm1.h' }
            @{ Src = 'mixa_manager\mixa_audio_win32_l2.h.lm1'; Out = 'mixa_audio_win32_l2.lm1.h' }
        )
        Probe = $null
        OracleDeps = @()
        L2Deps = @()
        RuntimeTrio = $false
        Fixture = 'bespoke-audio-wav'
        LinkLibs = '-lwinmm'
        BarrierSet = 'UIA'
        # Real MCI has a genuine path-length limit for its quoted device
        # path (mixa_audio_win32_l2_port.txt: 137 chars fails, 125
        # succeeds, deterministically, on this machine). This run
        # directory naming + "of"/"lf" fixture subdir naming is the
        # exact shape already proven to stay under that limit -- do not
        # let this module fall back to the generic longer naming.
        ShortBaseDir = 'aw32l2p'
        OracleFixtureSubdir = 'of'
        L2FixtureSubdir = 'lf'
    }
    'calculator_syntax' = @{
        HeaderTrans = @(
            @{ Src = 'mixa_manager\mixa_calculator_syntax.h.lm1'; Out = 'mixa_calculator_syntax.lm1.h' }
            @{ Src = 'mixa_manager\mixa_calculator_syntax_l2.h.lm1'; Out = 'mixa_calculator_syntax_l2.lm1.h' }
        )
        Probe = $null
        OracleDeps = @()
        L2Deps = @()
        RuntimeTrio = $true
        Fixture = 'none'
        LinkLibs = ''
        BarrierSet = 'UIA'
    }
    'dir_win32' = @{
        HeaderTrans = @(
            @{ Src = 'mixa_manager\mixa_dir_win32.h.lm1'; Out = 'mixa_dir_win32.lm1.h' }
            @{ Src = 'mixa_manager\mixa_dir.h.lm1'; Out = 'mixa_dir.lm1.h' }
            @{ Src = 'mixa_manager\mixa_dir_win32_l2.h.lm1'; Out = 'mixa_dir_win32_l2.lm1.h' }
        )
        Probe = $null
        OracleDeps = @()
        L2Deps = @()
        RuntimeTrio = $false
        Fixture = 'shared-separate'
        LinkLibs = ''
        BarrierSet = 'UIA'
    }
    'file_win32' = @{
        HeaderTrans = @(
            @{ Src = 'mixa_manager\mixa_file_win32_l2.h.lm1'; Out = 'mixa_file_win32_l2.lm1.h' }
        )
        Probe = @{ RealC = 'tests\mixa_file_win32_abi_probe_real.c'; L2C = 'tests\mixa_file_win32_abi_probe_l2.c'; Libs = '-lkernel32' }
        OracleDeps = @()
        L2Deps = @()
        RuntimeTrio = $false
        Fixture = 'none'
        LinkLibs = '-lkernel32'
        BarrierSet = 'UIA'
    }
    'fileio_win32' = @{
        HeaderTrans = @(
            @{ Src = 'mixa_manager\mixa_fileio_win32.h.lm1'; Out = 'mixa_fileio_win32.lm1.h' }
            @{ Src = 'mixa_manager\mixa_fileio.h.lm1'; Out = 'mixa_fileio.lm1.h' }
            @{ Src = 'mixa_manager\mixa_fileio_win32_l2.h.lm1'; Out = 'mixa_fileio_win32_l2.lm1.h' }
        )
        Probe = $null
        OracleDeps = @()
        L2Deps = @()
        RuntimeTrio = $true
        Fixture = 'shared-separate'
        LinkLibs = ''
        BarrierSet = 'UI'
    }
    'pointer' = @{
        HeaderTrans = @(
            @{ Src = 'mixa_manager\mixa_pointer_l2.h.lm1'; Out = 'mixa_pointer_l2.lm1.h' }
        )
        Probe = @{ RealC = 'tests\mixa_pointer_abi_probe_real.c'; L2C = 'tests\mixa_pointer_abi_probe_l2.c'; Libs = '' }
        OracleDeps = @()
        L2Deps = @()
        RuntimeTrio = $false
        Fixture = 'none'
        LinkLibs = ''
        BarrierSet = 'UI'
    }
    'process_marker' = @{
        HeaderTrans = @(
            @{ Src = 'mixa_manager\mixa_process_marker.h.lm1'; Out = 'mixa_process_marker.lm1.h' }
            @{ Src = 'mixa_manager\mixa_process_marker_l2.h.lm1'; Out = 'mixa_process_marker_l2.lm1.h' }
        )
        Probe = @{ RealC = 'tests\mixa_process_marker_abi_probe_real.c'; L2C = 'tests\mixa_process_marker_abi_probe_l2.c'; Libs = '' }
        OracleDeps = @()
        L2Deps = @()
        RuntimeTrio = $true
        Fixture = 'none'
        LinkLibs = '-lkernel32'
        BarrierSet = 'UI'
    }
    'process_win32' = @{
        HeaderTrans = @(
            @{ Src = 'mixa_manager\mixa_process_win32_l2.h.lm1'; Out = 'mixa_process_win32_l2.lm1.h' }
        )
        Probe = $null
        OracleDeps = @()
        L2Deps = @()
        RuntimeTrio = $true
        Fixture = 'shared-separate'
        LinkLibs = ''
        BarrierSet = 'UI'
    }
    'share' = @{
        HeaderTrans = @(
            @{ Src = 'mixa_manager\mixa_share_win32.h.lm1'; Out = 'mixa_share_win32.lm1.h' }
            @{ Src = 'mixa_manager\mixa_share.h.lm1'; Out = 'mixa_share.lm1.h' }
            @{ Src = 'mixa_manager\mixa_share_l2.h.lm1'; Out = 'mixa_share_l2.lm1.h' }
        )
        Probe = $null
        OracleDeps = @()
        L2Deps = @()
        RuntimeTrio = $true
        Fixture = 'shared-separate'
        LinkLibs = ''
        BarrierSet = 'UI'
    }
    'text_rect' = @{
        HeaderTrans = @(
            @{ Src = 'mixa_manager\mixa_tiles_l2.h.lm1'; Out = 'mixa_tiles_l2.lm1.h' }
        )
        Probe = @{ RealC = 'tests\mixa_tiles_abi_probe_real.c'; L2C = 'tests\mixa_tiles_abi_probe_l2.c'; Libs = '' }
        OracleDeps = @()
        L2Deps = @()
        RuntimeTrio = $true
        Fixture = 'none'
        LinkLibs = ''
        BarrierSet = 'UI'
    }
    'event_fifo' = @{
        HeaderTrans = @(
            @{ Src = 'mixa_manager\mixa_event_fifo_l2.h.lm1'; Out = 'mixa_event_fifo_l2.lm1.h' }
        )
        Probe = @{ RealC = 'tests\mixa_event_fifo_abi_probe_real.c'; L2C = 'tests\mixa_event_fifo_abi_probe_l2.c'; Libs = '' }
        OracleDeps = @()
        L2Deps = @()
        RuntimeTrio = $true
        Fixture = 'none'
        LinkLibs = ''
        BarrierSet = 'UI'
    }
    'cmdline' = @{
        HeaderTrans = @(
            @{ Src = 'mixa_manager\mixa_cmdline_l2.h.lm1'; Out = 'mixa_cmdline_l2.lm1.h' }
        )
        Probe = @{ RealC = 'tests\mixa_cmdline_abi_probe_real.c'; L2C = 'tests\mixa_cmdline_abi_probe_l2.c'; Libs = '' }
        OracleDeps = @()
        L2Deps = @()
        RuntimeTrio = $true
        Fixture = 'none'
        LinkLibs = ''
        BarrierSet = 'UI'
    }
    'pump' = @{
        HeaderTrans = @(
            @{ Src = 'mixa_manager\mixa_pump_l2.h.lm1'; Out = 'mixa_pump_l2.lm1.h' }
        )
        Probe = $null
        OracleDeps = @('mixa_event_fifo', 'mixa_backend_table', 'mixa_backend_headless', 'mixa_backend_ctors_headless')
        L2Deps = @('mixa_event_fifo', 'mixa_backend_table', 'mixa_backend_headless', 'mixa_backend_ctors_headless')
        RuntimeTrio = $true
        Fixture = 'none'
        LinkLibs = ''
        BarrierSet = 'UI'
    }
}
