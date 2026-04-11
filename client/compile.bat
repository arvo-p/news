@echo off
echo Compiling...
gcc -c main.c -I.
gcc -c cmd.c -I.
gcc -c colorscheme.c -I.
gcc -c loadfiles.c -I.
gcc -c nav.c -I.
gcc -c tabs.c -I.
gcc -c ui.c -I.

echo Linking...
gcc main.o cmd.o colorscheme.o loadfiles.o nav.o tabs.o ui.o -o news.exe -lpthread -lshell32

if %errorlevel% equ 0 (
    echo Compilation successful!
) else (
    echo Compilation failed.
)
pause
