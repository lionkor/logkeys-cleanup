#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

enum State {
    STATE_NORMAL,
    STATE_SKIPPING,
};

static char in_buf[4096];
static char out_buf[4096];

bool safe_str_startswith(const char* str1, size_t str1len, const char* str2) {
    size_t str2len = strlen(str2);
    return strncmp(str1, str2, (str2len < str1len ? str2len : str1len)) == 0;
}

bool is_control(const char* str, size_t len) {
    return safe_str_startswith(str, len, "Enter")
        || safe_str_startswith(str, len, "Up")
        || safe_str_startswith(str, len, "Esc")
        || safe_str_startswith(str, len, "Home")
        || safe_str_startswith(str, len, "End")
        || safe_str_startswith(str, len, "Tab")
        || safe_str_startswith(str, len, "Del")
        || safe_str_startswith(str, len, "Down")
        || safe_str_startswith(str, len, "PgUp")
        || safe_str_startswith(str, len, "PgDown")
        || safe_str_startswith(str, len, "BckSp")
        || safe_str_startswith(str, len, "Right")
        || safe_str_startswith(str, len, "LMeta")
        || safe_str_startswith(str, len, "LShft")
        || safe_str_startswith(str, len, "LCtrl")
        || safe_str_startswith(str, len, "RCtrl")
        || safe_str_startswith(str, len, "RShft")
        || safe_str_startswith(str, len, "Alt")
        || safe_str_startswith(str, len, "LAlt")
        || safe_str_startswith(str, len, "RAlt")
        || safe_str_startswith(str, len, "Left")
        || (safe_str_startswith(str, len, "#+"));
}

int run(FILE* in) {
    size_t n = 0;
    // what to skip to (inclusive) when in STATE_SKIPPING
    // little tiny state machine :^)
    enum State state = STATE_NORMAL;
    for (;;) {
        n = fread(in_buf, 1, sizeof(in_buf), in);
        size_t out_n = 0;
        // error
        if (n != sizeof(in_buf) && ferror(in) != 0) {
            perror("fread");
            return 1;
        } else {
            // n bytes were read, iterate through all
            for (size_t i = 0; i < n; ++i) {
                switch (state) {
                case STATE_NORMAL:
                    // < followed by some control character word first letter
                    if (in_buf[i] == '<' && i + 1 < n && is_control(&in_buf[i + 1], n - i + 1)) {
                        state = STATE_SKIPPING;
                    } else {
                        out_buf[out_n] = in_buf[i];
                        ++out_n;
                    }
                    break;
                case STATE_SKIPPING:
                    if (in_buf[i] == '>') {
                        state = STATE_NORMAL;
                    }
                    break;
                }
            }
            size_t wrote_n = fwrite(out_buf, 1, out_n, stdout);
            if (wrote_n != out_n && ferror(stdout) != 0) {
                perror("fwrite");
                return 1;
            }
            if (feof(in) != 0) {
                break;
            }
        }
    }
    return 0;
}

int main(int argc, char** argv) {
    if (argc == 1) {
        return run(stdin);
    } else if (argc == 2) {
        FILE* file = fopen(argv[1], "r");
        if (!file) {
            perror("fopen");
            return 1;
        }
        int ret = run(file);
        fclose(file);
        return ret;
    } else {
        fprintf(stderr, "invalid arguments, expected no arguments or filename.");
        return 1;
    }
}
