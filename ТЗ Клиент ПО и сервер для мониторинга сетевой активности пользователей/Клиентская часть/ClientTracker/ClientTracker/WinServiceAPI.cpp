#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <string>

// Функция запуска клиентского приложения
void StartClientApplication()
{
    // Путь к расположению клиентского приложения (указать собственный)
    TCHAR clientAppPath[] = TEXT("C:\\C:\\Users\\MaksimVitalyevich\\source\\repos\\ClientDataTrackerApp\\x64\\Debug\\ClientDataTrackerApp.exe");

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);

    if (!CreateProcess(NULL, clientAppPath, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        std::cerr << "Failed to start client application." << std::endl;
    }
    else
    {
        std::cout << "Client application started." << std::endl;
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
}

VOID WINAPI ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv);
VOID WINAPI ServiceCtrlHandler(DWORD);
BOOL bDebug = FALSE;

SERVICE_STATUS ssStatus;
SERVICE_STATUS_HANDLE sshStatusHandle;
HANDLE sshStopEvent = NULL;
// Название самой службы (можно указать собственное)
LPCTSTR lpszServiceName = TEXT("MyClientServ");

int _tmain(int argc, TCHAR* argv[])
{
    SERVICE_TABLE_ENTRY DispatchTable[] =
    {
        { (LPWSTR)lpszServiceName, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
        { NULL, NULL }
    };

    if (StartServiceCtrlDispatcher(DispatchTable) == FALSE)
    {
        std::cerr << "Failed to start service dispatcher." << std::endl;
        return GetLastError();
    }
    return 0;
}

VOID WINAPI ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
    DWORD dwStatus = E_FAIL;
    DWORD dwSpecificError = 0;

    sshStatusHandle = RegisterServiceCtrlHandler(lpszServiceName, ServiceCtrlHandler);
    if (sshStatusHandle == NULL)
    {
        std::cerr << "RegisterServiceCtrlHandler failed." << std::endl;
        return;
    }

    ssStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    ssStatus.dwCurrentState = SERVICE_START_PENDING;
    ssStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    ssStatus.dwWin32ExitCode = 0;
    ssStatus.dwServiceSpecificExitCode = 0;
    ssStatus.dwCheckPoint = 0;
    ssStatus.dwWaitHint = 0;

    if (SetServiceStatus(sshStatusHandle, &ssStatus) == FALSE)
    {
        std::cerr << "SetServiceStatus failed." << std::endl;
    }

    ssStatus.dwCurrentState = SERVICE_RUNNING;
    ssStatus.dwCheckPoint = 0;
    ssStatus.dwWaitHint = 0;

    if (SetServiceStatus(sshStatusHandle, &ssStatus) == FALSE)
    {
        std::cerr << "SetServiceStatus failed." << std::endl;
    }

    // Запуск клиентского приложения
    StartClientApplication();

    sshStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (sshStopEvent == NULL)
    {
        ssStatus.dwCurrentState = SERVICE_STOPPED;
        ssStatus.dwWin32ExitCode = GetLastError();
        if (SetServiceStatus(sshStatusHandle, &ssStatus) == FALSE)
        {
            std::cerr << "SetServiceStatus failed." << std::endl;
        }
        return;
    }

    while (WaitForSingleObject(sshStopEvent, INFINITE) != WAIT_OBJECT_0)
    {
        // Здесь можно вставить дополнительные операции или ожидание
    }

    ssStatus.dwCurrentState = SERVICE_STOPPED;
    ssStatus.dwWin32ExitCode = 0;
    if (SetServiceStatus(sshStatusHandle, &ssStatus) == FALSE)
    {
        std::cerr << "SetServiceStatus failed." << std::endl;
    }
}

VOID WINAPI ServiceCtrlHandler(DWORD dwControl)
{
    switch (dwControl)
    {
    case SERVICE_CONTROL_STOP:
        if (ssStatus.dwCurrentState != SERVICE_RUNNING)
            break;
        ssStatus.dwCurrentState = SERVICE_STOP_PENDING;
        SetServiceStatus(sshStatusHandle, &ssStatus);
        SetEvent(sshStopEvent);
        return;
    default:
        break;
    }
}
