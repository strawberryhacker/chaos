// Formatted print to sized buffer implementation

#include <chaos/print_format.h>

// Combined format flags and option flags
#define FLAG_PREFIX      0x0001
#define FLAG_LEFT        0x0002
#define FLAG_ZERO        0x0004
#define FLAG_SIGN_IGNORE 0x0008
#define FLAG_SIGN_FORCE  0x0010
#define FLAG_LOWERCASE   0x0020
#define FLAG_SIGN        0x0040
#define FLAG_STRING      0x0080
#define FLAG_CHAR        0x0100
#define FLAG_BRACKET     0x0200

// Look up table for upper-case hex, set bit 5 to convert to lower-case
const char number_lookup[] = "0123456789ABCDEF";

// Writes one character to the buf double pointer and increments the buf pointer
// accordingly. This will ignore the write if the buffer is full
static inline void put_char(char c, char** buf, char* end) {
    if (*buf < end) {
        *(*buf)++ = c;
    }
}

u32 print_format_to_buf_arg(char* buf, u32 len, const char* str, va_list arg) {
    // Save the end of the buffer so that we can detect overflow
    char* const end = buf + len;

    for (; *str; str++) {
        // Any non-formatting character is just printed to the buffer
        if (*str != '{') {
            put_char(*str, &buf, end);
            continue;
        }

        // Parse the format flags
        u16 flags = 0;
        while (*++str) {
            if      (*str == '!') flags |= FLAG_PREFIX;
            else if (*str == '<') flags |= FLAG_LEFT;
            else if (*str == '0') flags |= FLAG_ZERO;
            else if (*str == '+') flags |= FLAG_SIGN_FORCE;
            else if (*str == ' ') flags |= FLAG_SIGN_IGNORE;
            else break;
        }

        // Skip any colon delimiter
        if (*str == ':') {
            str++;
        }

        // Parse the format width. The width will be -1 if no width is given
        i32 width = -1;
        if (*str == '_') {
            width = (i32)va_arg(arg, int);
            str++;
        } else if (*str >= '0' && *str <= '9') {
            width = 0;
            while (*str >= '0' && *str <= '9') {
                width = width * 10 + (*str++ - '0');
            }
        }

        // Skip any colon delimiter
        if (*str == ':') {
            str++;
        }

        // Parse the format option. The number system is defaulting to decimal
        u8 base = 10;
        switch (*str++) {
            case 's':
            case 'S':
                flags |= FLAG_STRING;
                break;
            case 'c':
            case 'C':
                flags |= FLAG_CHAR;
                break;
            case 'i':
                flags |= FLAG_SIGN;
            case 'u':
            case 'd':
                break;
            case 'x':
                flags |= FLAG_LOWERCASE;
            case 'X':
                base = 16;
                break;
            case 'b':
            case 'B':
                base = 2;
                break;
            case 'p':
            case 'P':
                flags |= FLAG_PREFIX;
                base = 16;
                width = 8;
                break;
            case 'r':
            case 'R':
                flags |= FLAG_PREFIX | FLAG_ZERO;
                base = 2;
                width = 34;
                break;
            case '{':
                flags |= FLAG_BRACKET;
                break;
            default:
                str--;
                continue;
        }

        // Print the result based on flags, width and option 
        if (flags & FLAG_CHAR) {
            put_char((char)va_arg(arg, int), &buf, end);

        } else if (flags & FLAG_STRING) {
            // Get the string pointer
            const char* ptr = (const char *)va_arg(arg, char *);

            // If no width is given we assume the string is terminated and print
            // the entire string. If the width is given, we print exectly width
            // number of bytes. If the string is smaller than width bytes we'll
            // fill in padding characters
            if (width < 0) {
                while (*ptr) {
                    put_char(*ptr++, &buf, end);
                }
            } else {
                u32 i;
                for (i = 0; (i < width) && ptr[i]; i++);

                // Padding holds the number of padding characters to be written
                // and i holds the number of bytes to print from the given string
                u32 padding = width - i;

                // Front pad sequence
                if ((flags & FLAG_LEFT) == 0) {
                    while (padding--) {
                        put_char(' ', &buf, end);
                    }
                }

                // Print the string
                while (i--) {
                    put_char(*ptr++, &buf, end);
                }
                
                // Trailing pad sequence
                if (flags & FLAG_LEFT) {
                    while (padding--) {
                        put_char(' ', &buf, end);
                    }
                }
            }
        } else {
            // Print a number
            char num_buf[35];
            char pad_char = (flags & FLAG_ZERO) ? '0' : ' ';
            char sign = 0;
            u32 index = 0;
            i32 num = (i32)va_arg(arg, int);
            u8 lowercase = (flags & FLAG_LOWERCASE) ? FLAG_LOWERCASE : 0;
            
            // If the numer is negative and given with the i option we flip the 
            // number and add the sign
            if (num < 0 && (flags & FLAG_SIGN)) {
                sign = '-';
                num = -num;
            }

            // Check if we need to force the sign
            if ((sign == 0) && (flags & FLAG_SIGN_FORCE)) {
                sign = '+';
            }

            // Check if we need to ignore the sign
            if (flags & FLAG_SIGN_IGNORE) {
                sign = ' ';
            }

            // Convert the number to string representation
            u32 num_pos = (u32)num;
            do {
                num_buf[index++] = number_lookup[num_pos % base] | lowercase;
                num_pos /= base;
            } while(num_pos);

            // Conditionally append the prefix in case of hex or binary number
            u8 sign_prefix_pad = 0;
            if (flags & FLAG_PREFIX) {
                if (base == 16 || base == 2) {
                    sign_prefix_pad += 2;
                }
            }

            // Append the sign to the buffer
            if (sign) {
                sign_prefix_pad++;
            }

            // Get the padding
            u32 padding = 0;
            if (width >= 0) {
                padding = ((index + sign_prefix_pad) > width) ? 0 : 
                    width - index - sign_prefix_pad;
            }

            // Append the sign
            if (sign) {
                put_char(sign, &buf, end);
            }

            // Append the prefix
            if (flags & FLAG_PREFIX) {
                if (base == 16) {
                    put_char('0', &buf, end);
                    put_char('x', &buf, end);
                } else if (base == 2) {
                    put_char('0', &buf, end);
                    put_char('b', &buf, end);
                }
            }

            // Append the sign to the buffer
            if (sign) {
                sign_prefix_pad++;
            }

            // Front pad sequence
            if ((flags & FLAG_LEFT) == 0) {
                while (padding--) {
                    put_char(pad_char, &buf, end);
                }
            }

            // Print the string
            while (index) {
                put_char(num_buf[--index], &buf, end);
            }
            
            // Trailing pad sequence
            if (flags & FLAG_LEFT) {
                while (padding--) {
                    put_char(pad_char, &buf, end);
                }
            }
        }

        // If the user don't write ending bracket we don't skrip the next 
        // character
        if (*str != '}') {
            str--;
        }
    }
    return buf + len - end;
}

u32 print_format_to_buf(char* buf, u32 len, const char* str, ...) {
    va_list arg;
    va_start(arg, str);

    // Format the input
    u32 size = print_format_to_buf_arg(buf, len, str, arg);
    va_end(arg);
     
    return size;
}
