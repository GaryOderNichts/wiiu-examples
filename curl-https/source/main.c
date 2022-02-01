#include <curl/curl.h>

#include <whb/proc.h>
#include <whb/log.h>
#include <whb/log_console.h>

#include <romfs-wiiu.h>

#include <string.h>

static size_t writefunction(void* data, size_t size, size_t nmemb, void* userptr)
{
    size_t total_size = size * nmemb;

    // Copy data to a null-terminated buffer
    char buf[total_size + 1];
    memcpy(buf, data, total_size);
    buf[total_size] = '\0';

    // Print to log console line-by-line
    char* to_print = strtok(buf, "\n");
    while (to_print != NULL) {
        WHBLogPrint(to_print);
        to_print = strtok(NULL, "\n");
    }

    return total_size;
}

int main(int argc, char const *argv[])
{
    // Initialize ProcUI
    WHBProcInit();

    // Initialize a log console
    WHBLogConsoleInit();

    // Initialize romfs for the certificate bundle
    romfsInit();

    WHBLogPrintf("curl-https example");
    WHBLogConsoleDraw();

    // Initialize curl
    CURLcode res = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (res != CURLE_OK) {
        WHBLogPrintf("curl_global_init: %d", res);
        goto done;
    }

    // Start a curl session
    CURL* curl = curl_easy_init();
    if (!curl) {
        WHBLogPrintf("curl_easy_init: failed");
        curl_global_cleanup();
        goto done;
    }

    // Use the certificate bundle in the romfs
    curl_easy_setopt(curl, CURLOPT_CAINFO, "romfs:/cacert.pem");

    // Set the custom write function
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunction);

    // Set the download URL
    curl_easy_setopt(curl, CURLOPT_URL, "https://raw.githubusercontent.com/GaryOderNichts/wiiu-examples/main/curl-https/wiiulogo.txt");

    WHBLogPrintf("Starting download...");
    WHBLogConsoleDraw();

    // Perform the download
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        WHBLogPrintf("curl_easy_perform: %d", res);
    }

    // Done, clean up and exit
    curl_easy_cleanup(curl);
    curl_global_cleanup();

done: ;
    WHBLogPrintf("Done, press HOME to exit");
    WHBLogConsoleDraw();

    // Wait until the user exits the application
    while (WHBProcIsRunning()) { }

    romfsExit();
    WHBLogConsoleFree();
    WHBProcShutdown();

    return 0;
}
