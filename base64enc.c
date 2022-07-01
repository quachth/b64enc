#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <err.h>
#include <stdint.h>
#ifndef UINT8_MAX
#error "No support for uint_t"
#endif

int main(int argc, char *argv[]) {
        static char const alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                               "abcdefghijklmnopqrstuvwxyz"
                               "0123456789+/=";

        if (argc > 2) {
                err(errno = EINVAL, "Too many arguments.");
                return EXIT_FAILURE;
        }

        FILE* input;
        if (argc == 1) {                                                // read from stdin
                input = stdin;
        }
        if (argc == 2) {                                                // read from file given, if it exists, or stfin if file is "-"
                if (strcmp(argv[1], "-") == 0 ) {
                        input = stdin;
                }
                else {
                        input = fopen(argv[1],"r");
                        if (input == NULL) {
                                err(errno = ENOENT, "No such file exists.");
                                return EXIT_FAILURE;
                        }
                }
        }

        int wrap_counter = 0;

        for (;;) {
                uint8_t in[3] = {0};
                uint8_t out[4] = {0};
                fflush(stdout);
                size_t bytes_read = fread(in, sizeof (*in), (sizeof(in)/sizeof(*in)), input);

                if (ferror(stdin)) {
                        err(errno, "fread failed.");
                }

                if ((bytes_read == 0) && feof(input)) {                 // if there are no more bytes, end
                        printf("\n");
                        break;
                }
                
                out[0] = out[1] = out[2] = out[3] = 0;
                out[0] = in[0] >> 2;                                    // high 6 bits from first character
                out[1] = ((in[0] & 0x03) << 4 | (in[1] >> 4));          // low 2 bits from first character with high 4 bits from second character
                out[2] = ((in[1] & 0x0F) << 2 | (in[2] >> 6));          // low 4 bits from the second character with high 2 bits from third character
                out[3] = in[2] & 63;                                    // low 6 bits from the third character

                if (bytes_read < 3) {                                   // if under 3 bytes were read, replace characters 3 and/or 4 with "="
                        out[3] = (sizeof (alphabet) / sizeof (alphabet[0])-1);
                        if (bytes_read == 1) {
                                out[2] = (sizeof (alphabet) / sizeof (alphabet[0])-1);
                        }
                }

                for (int i=0; i<4; i++) {                               // iterates through encoded
                        fwrite(alphabet+out[i], sizeof (alphabet[0]), 1, stdout);
                        if (ferror(stdout)) {
                                err(errno, "fwrite failed.");
                        }
                        wrap_counter += 1;
                        if (wrap_counter == 76) {
                                fwrite("\n", sizeof(char), 1, stdout);
                                if (ferror(stdout)) {
                                        err(errno, "Line wrap failed");
                                }
                                wrap_counter = 0;
                        }
                }

                fflush(stdout);
                if (feof(input)) {
                        if (wrap_counter !=0) {
                                printf("\n");
                        }
                        printf("Finished.\n");
                        break;
                }
        }
        fclose(input);
        return 0;
}
