#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <string>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <dirent.h>

#include <libpiper/server.h>

// Internal Error
static void internal_error(int sock) {
    piper_response response;
    response.content_type = SERVER_INTERNAL_ERROR;
    response.content_length = 0;
    piper_server_respond(sock, &response);
}

// Callback
static int _starts_with(const char *pre, const char *str) {
    return strncmp(pre, str, strlen(pre)) == 0;
}
static int _ends_with(const char *suf, const char *str) {
    size_t suf_len = strlen(suf);
    size_t str_len = strlen(str);
    return str_len >= suf_len && !strcmp(str + str_len - suf_len, suf);
}
static int callback(piper_request *request, int sock) {
    // Log
    printf("Serving: %s\n", request->path);

    // Resolve
    char *full_path = NULL;
    {
        // Make Path
        std::string path;
        path += "./";
        path += request->path;
        // Get CWD
        char *cwd = getcwd(NULL, 0);
        // Resolve
        full_path = realpath(path.c_str(), NULL);
        // Check
        if (cwd == NULL || full_path == NULL || !_starts_with(cwd, full_path)) {
            full_path = NULL;
        }
        // Free CWD
        free(cwd);
    }

    // Check
    if (full_path == NULL) {
        static piper_response response;
        response.content_type = SERVER_NOT_FOUND_ERROR;
        response.content_length = 0;
        if (piper_server_respond(sock, &response) != 0) {
            internal_error(sock);
        }
        goto free;
    }

    // Choose Behavior
    int is_dir;
    {
        struct stat st;
        is_dir = stat(full_path, &st) == 0 && S_ISDIR(st.st_mode);
    }
    if (is_dir) {
        // Force Directory Paths To End With '/'
        if (!_ends_with("/", request->path)) {
            // Redirect
            piper_server_respond_str(sock, REDIRECT, "%s/", request->path);
            goto free;
        }

        // Directory Listing
        std::string page = "## Directory Listing";
        // List Files
        DIR *dir = opendir(full_path);
        if (dir) {
            struct dirent *entry;
            while ((entry = readdir(dir)) != NULL) {
                page += "\n=> ";
                page += entry->d_name;
                if (entry->d_type == DT_DIR) {
                    page += '/';
                }
            }
            closedir(dir);
        }
        // Sned
        if (piper_server_respond_str(sock, UTF8_GEMTEXT, "%s", page.c_str()) != 0) {
            internal_error(sock);
        }
    } else {
        // File
        std::ifstream file(full_path, std::ios::in | std::ios::binary);
        if (file && file.good()) {
            std::ostringstream data;
            data << file.rdbuf();
            file.close();
            // Get Content Type
            uint8_t content_type;
            if (_ends_with(".gmi", full_path)) {
                content_type = UTF8_GEMTEXT;
            } else if (_ends_with(".txt", full_path)) {
                content_type = UTF8_TEXT;
            } else {
                content_type = RAW_FILE;
            }
            // Send
            if (piper_server_respond_str(sock, content_type, "%s", data.str().c_str()) != 0) {
                internal_error(sock);
            }
        } else {
            // Unable To Read
            static piper_response response;
            response.content_type = SERVER_NOT_FOUND_ERROR;
            response.content_length = 0;
            if (piper_server_respond(sock, &response) != 0) {
                internal_error(sock);
            }
        }
    }

    // Free
 free:
    free(full_path);

    // Return
    return 0;
}

// Main
int main() {
    printf("Starting File Server...\n");
    // Run
    if (piper_server_run(60, 10, callback) != 0) {
        fprintf(stderr, "Failed To Start Server\n");
        return EXIT_FAILURE;
    } else {
        return EXIT_SUCCESS;
    }
}
