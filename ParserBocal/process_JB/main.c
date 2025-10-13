/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: igilbert <igilbert@student.42perpignan.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/03 12:12:34 by igilbert          #+#    #+#             */
/*   Updated: 2025/09/24 11:27:34 by igilbert         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "process.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

static void get_month_name(int month, char *out, size_t outsz) {
    const char *months[] = {"Janvier","Fevrier","Mars","Avril","Mai","Juin","Juillet","Aout","Septembre","Octobre","Novembre","Decembre"};
    if (month >= 1 && month <= 12) {
        snprintf(out, outsz, "%s", months[month-1]);
    } else {
        snprintf(out, outsz, "Inconnu");
    }
}

static int extract_first_date_mm_yyyy(FILE *in, int *out_month, int *out_year) {
    char line[MAX_LINE_SIZE];
    long pos = ftell(in);
    int d = 0, m = 0, y = 0;
    while (fgets(line, sizeof(line), in)) {
        // Look for DD/MM/YYYY at start or anywhere in line
        for (char *p = line; *p; ++p) {
            if (sscanf(p, "%2d/%2d/%4d", &d, &m, &y) == 3) {
                if (m >= 1 && m <= 12 && y >= 1900) {
                    *out_month = m; *out_year = y;
                    fseek(in, pos, SEEK_SET);
                    return 1;
                }
            }
        }
    }
    fseek(in, pos, SEEK_SET);
    return 0;
}

void print_usage(const char *program_name) {
    printf("Usage: %s <input_file> [chart_of_accounts_file]\n", program_name);
    printf("Creates ./Journal Bq {Mois} {Annee}.csv based on the input data date.\n");
    printf("If chart_of_accounts_file is not specified, Plan Comptable 2025.csv will be used.\n");
}

int main(int argc, char *argv[]) {
    FILE *input_file;
    FILE *output_file;
    int lines_processed;
    const char *chart_of_accounts_file = "Plan Comptable 2025.csv";
    
    // Check command line arguments
    if (argc < 2 || argc > 3) {
        print_usage(argv[0]);
        return 1;
    }
    
    // If chart of accounts file is provided, use it
    if (argc == 3) {
        chart_of_accounts_file = argv[2];
    }
    
    // Open input file
    input_file = fopen(argv[1], "r");
    if (!input_file) {
        fprintf(stderr, "Error: Could not open input file %s\n", argv[1]);
        return 2;
    }
    
    // Determine month/year from input and build output path ./stuffs/Journal Bq {Mois} {Annee}.csv
    int month = 0, year = 0;
    char month_name[16];
    if (!extract_first_date_mm_yyyy(input_file, &month, &year)) {
        // Fallback to current month/year if not found
        time_t now = time(NULL);
        struct tm *tm = localtime(&now);
        month = tm ? (tm->tm_mon + 1) : 1;
        year = tm ? (tm->tm_year + 1900) : 1970;
    }
    get_month_name(month, month_name, sizeof(month_name));

    char out_path[512];
    snprintf(out_path, sizeof(out_path), "Journal Bq %s %d.csv", month_name, year);

    // Open output file
    output_file = fopen(out_path, "w");
    if (!output_file) {
        fprintf(stderr, "Error: Could not create output file %s\n", out_path);
        fclose(input_file);
        return 3;
    }
    
    // Process the file
    lines_processed = process_csv_file(input_file, output_file, chart_of_accounts_file);
    
    // Close files
    fclose(input_file);
    fclose(output_file);
    
    if (lines_processed > 0) {
        printf("Successfully processed %d lines.\n", lines_processed);
        printf("Output written to %s\n", out_path);
    }
    
    return (lines_processed > 0) ? 0 : 4;
}

