#include <curl/curl.h>
#include <stdio.h>
#include <fcntl.h>
#include <string>

size_t http_callback(void *buffer, size_t sz, size_t nmemb, void *userp)
{
    /*
    This callback function gets called by libcurl as soon as there
    is data received that needs to be saved. buffer points to the delivered
    data, and the size of that data is sz multiplied with nmemb.

    The callback function will be passed as much data as possible in all
    invokes, but you must not make any assumptions. It may be one byte, it may
    be thousands. The maximum amount of body data that will be passed to the
    write callback is defined in the curl.h header file: CURL_MAX_WRITE_SIZE
    (the usual default is 16K). If CURLOPT_HEADER is enabled, which makes header
    data get passed to the write callback, you can get up to
    CURL_MAX_HTTP_HEADER bytes of header data passed into it. This usually means
    100K.

    This function may be called with zero bytes data if the transferred file is
    empty.

    The data passed to this function will not be zero terminated!
    */

    /* Calculate the amount of data that was passed in */
    size_t size = sz * nmemb;

    bool is_data_received = size > 0;
    if (is_data_received) {
        // Print the message that was received.
        // It will not be null terminated, so use fwrite instead of printf.
        fwrite(buffer, sz, nmemb, stdout);
        printf("\n");
    } else {
        printf("Received an empty response.\n");
    }

    return size;
}

int main(int argc, char *argv[])
{
    
    
    // Final message format:
    // http://api.thingspeak.com/update?api_key=ZKE95ZURWV7DW8B0
	// &field1=username
	// &field2=Official%20app%20says%20hello

    // argc is the number of command-line arguments provided to the program.
    // The first argument (argv[0]) is always the name of the program.

    char *input_function_name = argv[0];  // Points to the program name
    

    if (argc != 3) { // Ensure exactly 2 arguments are provided
    printf("Usage:\n");
    printf("%s <username> <message>\n", input_function_name);
    return 1;
    }

    char *input_username = argv[1];      // Points to the username provided by the user
    char *input_message = argv[2];    // Points to the message provided by the user

    std::string THINGSPEAK_URL = "http://api.thingspeak.com/update?api_key=ZKE95ZURWV7DW8B0";
    std::string username = input_username;
    std::string message = input_message;
    std::string final_url = THINGSPEAK_URL + "&field1=" + username + "&field2=" + message;


    // Initialise the HTTP library
    CURL *curl = curl_easy_init();
    if (!curl) {
        printf("Failed to initialise the curl library\n");
        return 1;
    }

    // Set the callback function that will receive the actual data
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, http_callback);

    // Configure the URL to load
    curl_easy_setopt(curl, CURLOPT_URL, final_url.c_str());

    // Send the HTTP request
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }

    // Done
    return 0;
}
