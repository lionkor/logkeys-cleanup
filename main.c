#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// state of the state machine used to decide what to write
// to stdout and what to skip
enum State {
    STATE_NORMAL,
    STATE_SKIPPING,
};

// buffer used to read file contents into
static char in_buf[4096];
// buffer used to write results into
static char out_buf[4096];

// compares if str1 starts with str2
bool safe_str_startswith(const char* haystack, size_t haystack_len, const char* needle, size_t needle_len) {
    if (haystack_len < needle_len) {
        // haystack too small to start with needle
        return false;
    } else {
        return memcmp(haystack, needle, needle_len) == 0;
    }
}

// this macro fills a StaticString struct with
// compile-time length of s.
// s must be a strin  literal.
#define STATIC_STR(s) \
    { s, sizeof(s) - 1 }

// string + string length
static struct StaticString {
    const char* str;
    size_t len;
}
// all control sequences the program should filter.
// these are initialized with the STATIC_STR macro to
// determine the string's lengths at compile-time.
s_control_sequences[]
    = {
          STATIC_STR("#+"), // special sequence followed by numbers
          STATIC_STR("Alt"),
          STATIC_STR("BckSp"),
          STATIC_STR("Del"),
          STATIC_STR("Down"),
          STATIC_STR("Down"),
          STATIC_STR("End"),
          STATIC_STR("Enter"),
          STATIC_STR("Esc"),
          STATIC_STR("Home"),
          STATIC_STR("LAlt"),
          STATIC_STR("LCtrl"),
          STATIC_STR("LMeta"),
          STATIC_STR("LShft"),
          STATIC_STR("Left"),
          STATIC_STR("PgDown"),
          STATIC_STR("PgUp"),
          STATIC_STR("RAlt"),
          STATIC_STR("RCtrl"),
          STATIC_STR("RShft"),
          STATIC_STR("Right"),
          STATIC_STR("Tab"),
          STATIC_STR("Up"),
      };

// returns true for any strings which match a control sequence
// output by logkeys. this list may not be complete.
bool is_control(const char* str, size_t len) {
    bool match = false;
    // loop over all pre-programmed control sequences until one matches
    for (size_t i = 0; i < (sizeof(s_control_sequences) / sizeof(struct StaticString)); ++i) {
        match = safe_str_startswith(str, len, s_control_sequences[i].str, s_control_sequences[i].len);
        if (match) {
            break;
        }
    }
    return match;
}

// does the cleanup on a file, writing output to stdout.
// removes all control characters.
// returns 0 on success, non-zero on failure.
int run(FILE* in) {
    size_t n = 0;
    // keep reading until feof. ferror doesn't break the loop
    // and instead returns 1.
    for (;;) {
        // little tiny state machine :^)
        enum State state = STATE_NORMAL;
        // read as much as fits into the in_buf. the return value indicates
        // how much was read, and whether an error occurred.
        n = fread(in_buf, 1, sizeof(in_buf), in);
        // keeps track of how many characters were written into the output
        size_t out_n = 0;
        // if an error occurred, return immediately
        if (n != sizeof(in_buf) && ferror(in) != 0) {
            perror("fread");
            return 1;
        } else {
            // n bytes were read, iterate through all of them.
            // copy all bytes from the input to the output that are
            // not part of a control sequence such as `<LShft>`.
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
                    // skips until this condition is met
                    if (in_buf[i] == '>') {
                        state = STATE_NORMAL;
                    }
                    break;
                }
            }
            // write only exactly out_b bytes of out_buf to stdout.
            // if this fails, exit early with failure.
            size_t wrote_n = fwrite(out_buf, 1, out_n, stdout);
            if (wrote_n != out_n && ferror(stdout) != 0) {
                perror("fwrite");
                return 1;
            }
            // end of input file is the only condition on which we can exit
            // with success.
            if (feof(in) != 0) {
                break;
            }
        }
    }
    return 0;
}

int main(int argc, char** argv) {
    if (argc == 1) {
        // read from stdin (pipe behavior)
        return run(stdin);
    } else if (argc == 2) {
        // read from file provided via args
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
