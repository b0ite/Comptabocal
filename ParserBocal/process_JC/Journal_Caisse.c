#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 1024
#define MAX_FIELD_LENGTH 256

// Comptes fixes pour le Journal de Caisse: 530 (crédit), 580 (débit)

// Helpers to sanitize and format numbers
static void remove_unicode_nbsp(char *s) {
    if (!s) return;
    unsigned char *src = (unsigned char *)s;
    unsigned char *dst = (unsigned char *)s;
    while (*src) {
        // U+00A0 (C2 A0) or U+202F (E2 80 AF)
        if (src[0] == 0xC2 && src[1] == 0xA0) { src += 2; continue; }
        if (src[0] == 0xE2 && src[1] == 0x80 && src[2] == 0xAF) { src += 3; continue; }
        // Skip regular spaces inside numbers (we'll control formatting)
        if (*src == ' ') { src++; continue; }
        *dst++ = *src++;
    }
    *dst = '\0';
}

static double parse_number(const char *s) {
    char buf[128];
    int j = 0, started = 0, dot = 0;
    for (const unsigned char *p = (const unsigned char *)s; *p; p++) {
        unsigned char c = *p;
        if (!started) {
            if (c == '-' || isdigit(c)) { buf[j++] = (char)c; started = 1; }
            else if (c == ',' || c == '.') { buf[j++] = '.'; started = 1; dot = 1; }
        } else {
            if (isdigit(c)) buf[j++] = (char)c;
            else if ((c == ',' || c == '.') && !dot) { buf[j++] = '.'; dot = 1; }
            else if (c == ' ' || c == '\t' || c == '\r' || c == '\n') continue;
            else if (c == 0xC2 && p[1] == 0xA0) { p++; continue; }
            else if (c == 0xE2 && p[1] == 0x80 && p[2] == 0xAF) { p += 2; continue; }
            else break;
        }
        if (j >= (int)sizeof(buf) - 1) break;
    }
    buf[j] = '\0';
    return (j > 0) ? atof(buf) : 0.0;
}

static void format_amount(double value, char *out, size_t outsz) {
    char tmp[64];
    snprintf(tmp, sizeof(tmp), "%.2f", value);
    // Replace dot with comma
    for (char *p = tmp; *p; p++) if (*p == '.') *p = ',';
    strncpy(out, tmp, outsz - 1);
    out[outsz - 1] = '\0';
}

// Function to extract month and year from a date string in format DD/MM/YYYY
void extract_month_year(const char *date, char *month, char *year) {
    // Assuming date format is DD/MM/YYYY
    strncpy(month, date + 3, 2);  // Extract MM
    month[2] = '\0';
    strncpy(year, date + 6, 4);   // Extract YYYY
    year[4] = '\0';
}

// Function to get month name from month number
void get_month_name(const char *month_num, char *month_name) {
    const char *months[] = {"Janvier", "Fevrier", "Mars", "Avril", "Mai", "Juin", 
                           "Juillet", "Aout", "Septembre", "Octobre", "Novembre", "Decembre"};
    int idx = atoi(month_num) - 1;  // Convert month string to integer and subtract 1 for 0-based index
    if (idx >= 0 && idx < 12) {
        strcpy(month_name, months[idx]);
    } else {
        strcpy(month_name, "Unknown");
    }
}

// Function to check if a file is an Excel file based on extension
int is_excel_file(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (ext != NULL) {
        if (strcmp(ext, ".xlsx") == 0 || strcmp(ext, ".xls") == 0) {
            return 1;
        }
    }
    return 0;
}

// Function to convert negative values to positive
void make_positive(char *value) {
    if (value[0] == '-') {
        // Remove negative sign by shifting all characters one position left
        memmove(value, value + 1, strlen(value));
    }
}

// Function to check if a value is zero
int is_zero(const char *value) {
    // Sanitize and check
    char tmp[MAX_FIELD_LENGTH];
    strncpy(tmp, value, sizeof(tmp) - 1); tmp[sizeof(tmp) - 1] = '\0';
    remove_unicode_nbsp(tmp);
    for (char *p = tmp; *p; ++p) if (*p == ',') *p = '.';
    double num = parse_number(tmp);
    return (num == 0.0);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    // Check if the input file is an Excel file
    if (is_excel_file(argv[1])) {
        printf("Error: Excel files (.xlsx/.xls) are not supported directly.\n");
        printf("Please convert the Excel file to CSV format first and then run the program.\n");
        printf("You can do this by opening the Excel file and using 'Save As' with CSV format.\n");
        return 1;
    }

    FILE *input_file = fopen(argv[1], "r");
    if (!input_file) {
        printf("Error: Could not open input file %s\n", argv[1]);
        return 1;
    }

    // Skip the first 5 lines
    char line[MAX_LINE_LENGTH];
    for (int i = 0; i < 5; i++) {
        if (!fgets(line, sizeof(line), input_file)) {
            printf("Error: Input file has less than 5 lines\n");
            fclose(input_file);
            return 1;
        }
    }

    // Create variables for storing data
    char month[3] = "";
    char year[5] = "";
    char month_name[20] = "";
    char output_filename[256] = "";
    FILE *output_file = NULL;
    int first_record = 1;

    // Process each line
    while (fgets(line, sizeof(line), input_file)) {
        // Parse the line to extract date and retrait
        char *token;
        char *rest = line;
        char date_value[MAX_FIELD_LENGTH] = "";
        char retrait_value[MAX_FIELD_LENGTH] = "";
        int field_count = 0;

        // Determine the delimiter
        char delimiter[2] = {0};
        delimiter[0] = (strchr(line, ';') != NULL) ? ';' : ',';
        
        while ((token = strtok_r(rest, delimiter, &rest))) {
            if (field_count == 0) {
                // First field is date
                strcpy(date_value, token);
            } else if (field_count == 4) {
                // Fifth field is retrait
                strcpy(retrait_value, token);
            }
            field_count++;
        }

        // Skip line if retrait is empty
        if (strlen(retrait_value) == 0) {
            continue;
        }

        // Trim CR/LF from retrait
        char *p = retrait_value;
        while (*p) {
            if (*p == '\n' || *p == '\r') {
                *p = '\0';
                break;
            }
            p++;
        }

        // Make sure date value is not empty before proceeding
        if (strlen(date_value) == 0) {
            continue;
        }
        
        // Convert negative to positive and sanitize
        make_positive(retrait_value);
        remove_unicode_nbsp(retrait_value);

        // Skip if retrait value is zero
        if (is_zero(retrait_value)) {
            continue;
        }

        // If this is the first record, create the output file
        if (first_record) {
            extract_month_year(date_value, month, year);
            get_month_name(month, month_name);
            
            sprintf(output_filename, "Journal Caisse %s %s.csv", month_name, year);
            output_file = fopen(output_filename, "w");
            if (!output_file) {
                printf("Error: Could not create output file %s\n", output_filename);
                fclose(input_file);
                return 1;
            }
            
            // Write header: 'cpte' and with cpte after libelle (Journal;Jour;Libelle;cpte;Debit;Credit)
            fprintf(output_file, "Journal;Jour;Libelle;cpte;Debit;Credit\n");
            first_record = 0;
        }

        // Prepare formatted amount with comma
        for (char *q = retrait_value; *q; ++q) if (*q == ',') *q = '.';
        double val = parse_number(retrait_value);
        char amt[64];
        format_amount(val, amt, sizeof(amt));

    // Comptes fixes: crédit 530, débit 580 (with 'cpte' after libelle)
    fprintf(output_file, "CA;%s;Prlv caisse;530;;%s\n", date_value, amt);
    fprintf(output_file, "CA;%s;Prlv caisse;580;%s;\n", date_value, amt);
    }

    // Clean up
    fclose(input_file);
    if (output_file) {
        fclose(output_file);
        printf("Successfully created %s\n", output_filename);
    } else {
        printf("No valid data found in input file\n");
    }

    return 0;
}
