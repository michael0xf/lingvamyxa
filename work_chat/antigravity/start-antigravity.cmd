@echo off
rem Explicitly user-authorized: automatically approve all Antigravity tool requests.
cd /d "C:\Nyasha_Planet\lingvamyxa"
"C:\Users\mtkra\AppData\Local\agy\bin\agy.exe" --continue --dangerously-skip-permissions %*
