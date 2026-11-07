#include <stdio.h>
#include <stddef.h>
#include <limits.h>
#include <stdbool.h>

typedef enum {
    STATUS_OK = 0,
    STATUS_INVALID_ARGUMENT,
    STATUS_BUFFER_TOO_SMALL,
    STATUS_INPUT_EMPTY,
    STATUS_INPUT_OVERFLOW,
    STATUS_INPUT_MALFORMED
} StatusCode;
static unsigned int bitPlus(unsigned int a, unsigned int b) {

    while (b) {
        unsigned int c = a & b;
        a = a ^ b;
        b = c << 1;
    }
    return a;
}

StatusCode funqBase2r(const int number, const unsigned int r, char * out_buffer, const size_t buffer_size) {
    if (out_buffer == NULL || r < 1 || r > 5) {
        return STATUS_INVALID_ARGUMENT;
    }
    if (buffer_size < 35) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    static const char digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUV";
    char temp[35];
    size_t temp_idx = 0;

    unsigned int is_negative = 0;
    unsigned int u_val = 0;

    if (number < 0) {
        is_negative = 1;
        u_val = bitPlus((unsigned int)(~number), 1);
    } else {
        u_val = (unsigned int)number;
    }

    if (u_val == 0) {
        temp[temp_idx] = '0';
        temp_idx = bitPlus((unsigned int)temp_idx, 1);
    } else {
        const unsigned int mask = ~( (~0U) << r );
        while (u_val) {
            unsigned int digit = u_val & mask;
            temp[temp_idx] = digits[digit];
            temp_idx = bitPlus((unsigned int)temp_idx, 1);
            u_val = u_val >> r;
        }
    }

    if (is_negative) {
        temp[temp_idx] = '-';
        temp_idx = bitPlus((unsigned int)temp_idx, 1);
    }

    size_t out_idx = 0;
    while (temp_idx) {
        temp_idx = bitPlus((unsigned int)temp_idx, ~0U);
        out_buffer[out_idx] = temp[temp_idx];
        out_idx = bitPlus((unsigned int)out_idx, 1);
    }
    out_buffer[out_idx] = '\0';

    return STATUS_OK;
}

static StatusCode parse_int(char *  str, int * out_val) {
    if (str == NULL || out_val == NULL) return STATUS_INVALID_ARGUMENT;

    const char *p = str;
    while (*p == ' ' || *p == '\t') p++;

    if (*p == '\0' || *p == '\n') return STATUS_INPUT_EMPTY;

    bool negative = false;
    if (*p == '-') {
        negative = true;
        p++;
    } else if (*p == '+') {
        p++;
    }

    if (*p < '0' || *p > '9') return STATUS_INPUT_MALFORMED;

    long long acc = 0;
    while (*p >= '0' && *p <= '9') {
        acc = acc * 10 + (*p - '0');
        if (!negative && acc > INT_MAX) return STATUS_INPUT_OVERFLOW;
        if (negative && (-acc) < INT_MIN) return STATUS_INPUT_OVERFLOW;
        p++;
    }

    while (*p == ' ' || *p == '\t') p++;
    if (*p != '\0' && *p != '\n' && *p != '\r') {
        return STATUS_INPUT_MALFORMED;
    }

    *out_val = negative ? (int)(-acc) : (int)acc;
    return STATUS_OK;
}

int main(void) {
    char input_line[128];
    int number = 0;
    int r_in = 0;

    printf("Enter an integer: ");
    if (fgets(input_line, sizeof(input_line), stdin) == NULL) {
        fprintf(stderr, "Error: failed to read input.\n");
        return 1;
    }

    StatusCode st_num = parse_int(input_line, &number);
    switch (st_num) {
        case STATUS_OK:
            break;
        case STATUS_INPUT_OVERFLOW:
            fprintf(stderr, "error: the entered number is out of int range.\n");
            return 1;
        case STATUS_INPUT_MALFORMED:
        case STATUS_INPUT_EMPTY:
        default:
            fprintf(stderr, "error: invalid number format.\n");
            return 1;
    }

    printf("Enter parameter r (an integer from 1 to 5): ");
    if (fgets(input_line, sizeof(input_line), stdin) == NULL) {
        fprintf(stderr, "Error: failed to read input.\n");
        return 1;
    }

    StatusCode st_r = parse_int(input_line, &r_in);
    if (st_r != STATUS_OK || r_in < 1 || r_in > 5) {
        fprintf(stderr, "error: r must be an integer in the range [1, 5].\n");
        return 1;
    }

    char result_buf[64];
    StatusCode operStatus = funqBase2r(number, (unsigned int)r_in, result_buf, sizeof(result_buf));

    switch (operStatus) {
        case STATUS_OK:
            printf("Result in base %u (2^%d): %s\n", 1U << r_in, r_in, result_buf);
            break;
        case STATUS_BUFFER_TOO_SMALL:
            fprintf(stderr, "Error: buffer size is insufficient.\n");
            return 1;
        default:
            fprintf(stderr, "error: code %d.\n", operStatus);
            return 1;
    }

    return 0;
}
