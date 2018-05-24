@echo off
SETLOCAL ENABLEDELAYEDEXPANSION
set "mypath=%~dp0"
set "pattern=Usage:"
FOR /f %%A IN ('scoop') DO (
    SET "VAR1=%%A"
    if "!VAR1:%pattern%=!" == "!VAR1!" (
        @echo "Scoop is not installed go to website scoop.sh"
        @pause
        goto:eof
    ) else (
        GOTO CHECK-VSCODE
    )
)

:CHECK-VSCODE
    set "pattern=Installed: No"
    FOR /f "delims=" %%A IN ('scoop info vscode') DO (
        SET "VAR1=%%A"
        if "!VAR1:%pattern%=!" == "!VAR1!" (
            echo off
        ) else (
            @echo "Vscode is not installed by scoop; you can do scoop install vscode"
            set "vscode=no"
            GOTO CHECK-LLVM
        )
    )
    set "vscode=yes"
    GOTO CHECK-LLVM

:CHECK-LLVM
    set "pattern=PATH"
    set "check=(simulated)"
    FOR /f "delims=" %%A IN ('scoop info llvm') DO (
        SET "VAR1=%%A"
        if "!VAR1:%pattern%=!" == "!VAR1!" (
            echo off
            if "!VAR1:%check%=!" == "!VAR1!" (
                echo off
            ) else (
                @echo "LLVM is not installed by scoop run scoop install llvm if you want to use clang and vscode intellisense; We will now look for gcc"
                set "vscode=no"
                @pause 
                GOTO CHECK-GCC
            )
        ) else (
            set "str=%%A"
            set "str=!str:~14!"
            set "str=!str:\=/!"
            set "LLVM=!str:C:=/c!"
        )
    )
    set "str=%PATH"
    @echo %LLVM%
    if [%2] == [] (
        set "clang=False"
    ) else (
        set "clang=True"
    )
    @echo %clang%
    GOTO CHECK-GCC

:CHECK-GCC
    set "pattern=PATH"
    set "check=(simulated)"
    set "gnu=False"
    FOR /f "delims=" %%A IN ('scoop info gcc') DO (
        SET "VAR1=%%A"
        if "!VAR1:%check%=!" == "!VAR1!" (
            echo off
        ) else (
            @echo "gcc is not installed by scoop run scoop install gcc for compilation with gcc and to have libraries to compile with clang"
            @pause 
            GOTO:eof
        )

        if "!VAR1:%pattern%=!" == "!VAR1!" (
            echo off 
        ) else (
            set "str=%%A"
            set "str=!str:~14!"
            set "str=!str:\=/!"
            set "GCC=!str:C:=/c!"
        )
    )
    @echo %GCC%
    set "filename=%~n0"
    cd %mypath%
    msys2 %mypath%%filename%.sh %GCC% %LLVM% %1 %clang%

