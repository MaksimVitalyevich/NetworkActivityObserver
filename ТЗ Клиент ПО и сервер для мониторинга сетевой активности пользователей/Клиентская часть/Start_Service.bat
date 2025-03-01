@echo off

:: Путь к исполняемому файлу службы
set SERVICE_PATH=C:\Users\MaksimVitalyevich\source\repos\WinServiceAPI\x64\Debug\WinServiceAPI.exe
set SERVICE_NAME=ClientService

:: Проверка наличия службы
sc query %SERVICE_NAME% >nul 2>&1
if %errorlevel% == 0 (
    echo Служба %SERVICE_NAME% уже существует.
    echo Перезапуск службы...
    sc stop %SERVICE_NAME%
    sc start %SERVICE_NAME%
) else (
    echo Создание и запуск службы %SERVICE_NAME%...
    sc create %SERVICE_NAME% binPath= "%SERVICE_PATH%"
    sc start %SERVICE_NAME%
)

echo Выполнено.
pause