/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   process.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: igilbert <igilbert@student.42perpignan.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/18 15:50:41 by igilbert          #+#    #+#             */
/*   Updated: 2025/09/08 15:18:39 by igilbert         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "process.h"

// Utility function to trim whitespace from strings
char* trim(char *str) {
    if (!str) return NULL;
    
    // Trim leading spaces
    while (isspace((unsigned char)*str)) str++;
    
    if (*str == '\0') return str;  // All spaces
    
    // Trim trailing spaces
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    
    // Write new null terminator
    *(end + 1) = '\0';
    
    return str;
}

// Convert string with numbers to double
double extract_number(const char *str) {
    char *temp = strdup(str);
    char *clean = temp;
    
    // Skip any non-digit characters at start except for a minus sign
    while (*clean && !isdigit(*clean) && *clean != '-' && *clean != '.') clean++;
    
    double result = atof(clean);
    free(temp);
    
    return result;
}

// Parse CSV line into fields
int parse_csv_line(char *line, char **fields, int max_fields, char delimiter) {
    int count = 0;
    int in_quotes = 0;
    char *p = line;
    fields[count++] = p;
    
    while (*p && count < max_fields) {
        if (*p == '\"') {
            in_quotes = !in_quotes;
        } else if (*p == delimiter && !in_quotes) {
            *p = '\0';
            fields[count++] = p + 1;
        }
        p++;
    }
    
    // Trim quotes and whitespace from each field
    for (int i = 0; i < count; i++) {
        fields[i] = trim(fields[i]);
        
        // Remove surrounding quotes if present
        int len = strlen(fields[i]);
        if (len >= 2 && fields[i][0] == '\"' && fields[i][len-1] == '\"') {
            fields[i][len-1] = '\0';
            fields[i]++;
        }
    }
    
    return count;
}

// Normalize the date format to DD/MM/YYYY for consistent comparison
void normalize_date(const char *input_date, char *normalized_date, size_t size) {
    // Check if format is M/D/YY (like 2/1/25)
    if (strchr(input_date, '/') && strlen(input_date) <= 8) {
        int day, month, year;
        if (sscanf(input_date, "%d/%d/%d", &day, &month, &year) == 3 ||
            sscanf(input_date, "%d/%d/%d", &month, &day, &year) == 3) {
            // Convert YY to YYYY if needed
            if (year < 100) {
                year += 2000;
            }
            snprintf(normalized_date, size, "%02d/%02d/%d", day, month, year);
            printf("Normalized date from '%s' to '%s'\n", input_date, normalized_date);
            return;
        }
    }
    
    // Default: just copy the date as is
    strncpy(normalized_date, input_date, size - 1);
    normalized_date[size - 1] = '\0';
    printf("Kept original date format: %s\n", normalized_date);
}

// Extract VAT information from TVA detail lines - improved version
void extract_vat_info(const char *line, double *vat_5_5_amount, double *vat_5_5_ht, 
                     double *vat_20_amount, double *vat_20_ht) {
    printf("Processing VAT line: %s\n", line);
    
    if (strstr(line, "TVA: 5.50%") || strstr(line, "TVA: 5,50%") || strstr(line, "TVA:5.50%")) {
        char *amount_str = strstr(line, "Montant:");
        char *ht_str = strstr(line, "HT:");
        
        if (amount_str && ht_str) {
            *vat_5_5_amount = extract_number(amount_str + 8);
            *vat_5_5_ht = extract_number(ht_str + 3);
            printf("Found 5.5%% VAT: Amount=%.2f, HT=%.2f\n", *vat_5_5_amount, *vat_5_5_ht);
        }
    } else if (strstr(line, "TVA:20.00%") || strstr(line, "TVA: 20.00%") || 
               strstr(line, "TVA: 20,00%") || strstr(line, "TVA:20,00%")) {
        char *amount_str = strstr(line, "Montant:");
        char *ht_str = strstr(line, "HT:");
        
        if (amount_str && ht_str) {
            *vat_20_amount = extract_number(amount_str + 8);
            *vat_20_ht = extract_number(ht_str + 3);
            printf("Found 20%% VAT: Amount=%.2f, HT=%.2f\n", *vat_20_amount, *vat_20_ht);
        }
    }
}

void convert_date_to_julian(const char *date, char *julian) {
    // Instead of converting to Julian, we'll keep the DD/MM/YYYY format
    strncpy(julian, date, MAX_FIELD_LENGTH - 1);
    julian[MAX_FIELD_LENGTH - 1] = '\0';
}

// Get month name from month number
char* get_month_name(int month) {
    static char *month_names[] = {
        "Janvier", "Fevrier", "Mars", "Avril", "Mai", "Juin",
        "Juillet", "Aout", "Septembre", "Octobre", "Novembre", "Decembre"
    };
    
    if (month >= 1 && month <= 12) {
        return month_names[month - 1];
    }
    
    return "Inconnu";
}

// Read sales data from CAISSE-CA file - enhanced with date normalization
int read_sales_data(const char *filename, SalesData *sales_data, int max_entries) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error opening file: %s\n", filename);
        return 0;
    }
    
    printf("Reading sales data from: %s\n", filename);
    
    char line[MAX_LINE_LENGTH];
    char *fields[20];
    int current_entry = -1;
    int data_section = 0;
    int count = 0;
    
    while (fgets(line, sizeof(line), file) && count < max_entries) {
        // Remove newline character
        size_t len = strlen(line);
        if (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r'))
            line[len-1] = '\0';
        
        // Replace commas with dots for number parsing
        char *p = line;
        while (*p) {
            if (*p == ',') *p = '.';
            p++;
        }
        
        // Check for start of data section
        if (strstr(line, "Date") && strstr(line, "CA TTC") && strstr(line, "CA HT")) {
            data_section = 1;
            printf("Found data section header\n");
            continue;
        }
        
        if (data_section) {
            // Parse the line
            int field_count = parse_csv_line(line, fields, 20, ';');
            
            // Check if this is a date line with sales data
            if (field_count >= 4 && fields[0][0] != '\0' && 
                strstr(fields[0], "/") && isdigit(fields[0][0])) {
                current_entry++;
                count++;
                
                if (current_entry >= max_entries)
                    break;
                
                // Normalize date format
                char normalized_date[MAX_DATE_LENGTH];
                normalize_date(fields[0], normalized_date, MAX_DATE_LENGTH);
                
                // Copy normalized date
                strncpy(sales_data[current_entry].date, normalized_date, MAX_DATE_LENGTH - 1);
                sales_data[current_entry].date[MAX_DATE_LENGTH - 1] = '\0';
                
                // Parse CA TTC and CA HT - replace commas with dots
                sales_data[current_entry].ca_ttc = extract_number(fields[1]);
                sales_data[current_entry].ca_ht = extract_number(fields[2]);
                
                // Initialize VAT fields
                sales_data[current_entry].vat_5_5_amount = 0.0;
                sales_data[current_entry].vat_5_5_ht = 0.0;
                sales_data[current_entry].vat_20_amount = 0.0;
                sales_data[current_entry].vat_20_ht = 0.0;
                
                printf("Read sales entry: Date=%s, TTC=%.2f, HT=%.2f\n", 
                       sales_data[current_entry].date,
                       sales_data[current_entry].ca_ttc, 
                       sales_data[current_entry].ca_ht);
            } 
            // Check if this is a VAT details line
            else if (current_entry >= 0 && 
                    (strstr(line, "TVA: 5.50%") || strstr(line, "TVA:20.00%") || 
                     strstr(line, "TVA: 20.00%") || strstr(line, "TVA: 5,50%") || 
                     strstr(line, "TVA: 20,00%"))) {
                extract_vat_info(line, 
                                &sales_data[current_entry].vat_5_5_amount,
                                &sales_data[current_entry].vat_5_5_ht,
                                &sales_data[current_entry].vat_20_amount,
                                &sales_data[current_entry].vat_20_ht);
            }
        }
    }
    
    fclose(file);
    printf("Finished reading sales data. Found %d entries.\n", count);
    return count;
}

// Read payment data from CAISSE-Reglement file - enhanced with date normalization
int read_payment_data(const char *filename, PaymentData *payment_data, int max_entries) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error opening file: %s\n", filename);
        return 0;
    }
    
    printf("Reading payment data from: %s\n", filename);
    
    char line[MAX_LINE_LENGTH];
    char *fields[20];
    int data_section = 0;
    int count = 0;
    
    while (fgets(line, sizeof(line), file) && count < max_entries) {
        // Remove newline character
        size_t len = strlen(line);
        if (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r'))
            line[len-1] = '\0';
        
        // Replace commas with dots for number parsing
        char *p = line;
        while (*p) {
            if (*p == ',') *p = '.';
            p++;
        }
            
        // Check for header line with "Date", "ESPECES", "CARTES", "TOTAL"
        if (strstr(line, "Date") && strstr(line, "ESPECES") && strstr(line, "CARTES") && 
            strstr(line, "TOTAL")) {
            data_section = 1;
            printf("Found payment data section header\n");
            continue;
        }
        
        if (data_section) {
            // Parse the line
            int field_count = parse_csv_line(line, fields, 20, ';');
            
            // Check if this is a data line with date and payment info
            if (field_count >= 14 && fields[0][0] != '\0' && 
                strstr(fields[0], "/") && isdigit(fields[0][0])) {
                
                if (count >= max_entries)
                    break;
                
                // Normalize date format
                char normalized_date[MAX_DATE_LENGTH];
                normalize_date(fields[0], normalized_date, MAX_DATE_LENGTH);
                
                // Copy normalized date
                strncpy(payment_data[count].date, normalized_date, MAX_DATE_LENGTH - 1);
                payment_data[count].date[MAX_DATE_LENGTH - 1] = '\0';
                
                // Parse payment amounts
                payment_data[count].especes = extract_number(fields[1]);
                payment_data[count].cartes = extract_number(fields[3]);
                
                // Try to get the total from the last field or calculate it
                if (field_count >= 15 && fields[14] && *fields[14]) {
                    payment_data[count].total = extract_number(fields[14]);
                } else {
                    // Calculate total from available payment methods
                    payment_data[count].total = payment_data[count].especes + payment_data[count].cartes;
                }
                
                printf("Read payment entry: Date=%s, Especes=%.2f, Cartes=%.2f, Total=%.2f\n", 
                       payment_data[count].date, 
                       payment_data[count].especes, 
                       payment_data[count].cartes, 
                       payment_data[count].total);
                
                count++;
            }
            // Check if we've reached the totals line (usually has no date)
            else if (field_count >= 14 && (fields[0][0] == '\0' || !strstr(fields[0], "/")) && 
                    isdigit(fields[1][0]) && isdigit(fields[3][0])) {
                printf("Found totals line, ending payment data processing\n");
                break;
            }
        }
    }
    
    fclose(file);
    printf("Finished reading payment data. Found %d entries.\n", count);
    return count;
}

// Combine sales and payment data into journal entries - improved matching with flexible tolerance
int combine_data(SalesData *sales_data, int sales_count, 
                PaymentData *payment_data, int payment_count,
                JournalEntry *journal_entries) {
    int entry_count = 0;
    
    printf("Combining data: %d sales entries and %d payment entries\n", sales_count, payment_count);
    
    for (int i = 0; i < sales_count && entry_count < MAX_ENTRIES; i++) {
        // Skip days with no sales
        if (sales_data[i].ca_ttc == 0) {
            printf("Skipping date %s with zero sales\n", sales_data[i].date);
            continue;
        }
        
        // Find matching payment data
        PaymentData *payment = NULL;
        for (int j = 0; j < payment_count; j++) {
            printf("Comparing sales date '%s' with payment date '%s'\n", 
                   sales_data[i].date, payment_data[j].date);
                   
            if (strcmp(sales_data[i].date, payment_data[j].date) == 0) {
                payment = &payment_data[j];
                printf("Match found for date %s\n", sales_data[i].date);
                break;
            }
        }
        
        if (!payment) {
            fprintf(stderr, "Warning: No payment data found for date %s, creating entry with just sales data\n", 
                    sales_data[i].date);
            
            // Create journal entry with just sales data
            JournalEntry entry = {0};
            strcpy(entry.date, sales_data[i].date);
            convert_date_to_julian(entry.date, entry.jour);
            
            entry.vente_5_5 = sales_data[i].vat_5_5_ht;
            entry.tva_5_5 = sales_data[i].vat_5_5_amount;
            entry.vente_20 = sales_data[i].vat_20_ht;
            entry.tva_20 = sales_data[i].vat_20_amount;
            entry.cb = sales_data[i].ca_ttc * 0.8; // Estimate 80% as CB if unknown
            entry.especes = sales_data[i].ca_ttc * 0.2; // Estimate 20% as cash if unknown
            
            printf("Creating journal entry with estimated payments for date %s\n", entry.date);
            journal_entries[entry_count++] = entry;
            continue;
        }
        
        // Create journal entry
        JournalEntry entry = {0};
        strcpy(entry.date, sales_data[i].date);
        convert_date_to_julian(entry.date, entry.jour);
        
        entry.vente_5_5 = sales_data[i].vat_5_5_ht;
        entry.tva_5_5 = sales_data[i].vat_5_5_amount;
        entry.vente_20 = sales_data[i].vat_20_ht;
        entry.tva_20 = sales_data[i].vat_20_amount;
        entry.cb = payment->cartes;
        entry.especes = payment->especes;
        
        // Validate the data totals match approximately with more flexible tolerance (5%)
        double sales_total = sales_data[i].ca_ttc;
        double payment_total = payment->total;
        double tolerance = sales_total * 0.05; // 5% tolerance
        
        if (fabs(sales_total - payment_total) > tolerance) { 
            printf("Warning: For date %s, sales total (%.2f) doesn't match payment total (%.2f)\n",
                   entry.date, sales_total, payment_total);
            
            // Adjust payment values proportionally if a small discrepancy
            if (payment_total > 0 && fabs(sales_total - payment_total) < sales_total * 0.25) {
                double ratio = sales_total / payment_total;
                entry.cb *= ratio;
                entry.especes *= ratio;
                printf("Adjusted payment values by factor %.2f to match sales total\n", ratio);
            }
        }
        
        printf("Creating journal entry for date %s: 5.5%% (%.2f/%.2f), 20%% (%.2f/%.2f), CB=%.2f, Especes=%.2f\n",
               entry.date, entry.vente_5_5, entry.tva_5_5, entry.vente_20, entry.tva_20, 
               entry.cb, entry.especes);
        
        // Store the entry
        journal_entries[entry_count++] = entry;
    }
    
    printf("Combined %d entries\n", entry_count);
    return entry_count;
}

// Write journal entries to CSV file
static void format_fr(double value, char *out, size_t outsz) {
    char tmp[64];
    snprintf(tmp, sizeof(tmp), "%.2f", value);
    for (char *p = tmp; *p; ++p) if (*p == '.') *p = ',';
    strncpy(out, tmp, outsz - 1);
    out[outsz - 1] = '\0';
}

int write_journal_file(const char *filename, JournalEntry *entries, int count) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "Error creating output file: %s\n", filename);
        return 0;
    }
    
    // Write header (use 'cpte' as requested)
    fprintf(file, "Journal;Jour;cpte;Libelle;Debit;Credit\n");

    // Write entries (comptes fixes)
    for (int i = 0; i < count; i++) {
        JournalEntry *e = &entries[i];
        char a[32], b[32], c[32], d[32], cb[32], esp[32];
        format_fr(e->vente_5_5, a, sizeof(a));
        format_fr(e->tva_5_5, b, sizeof(b));
        format_fr(e->vente_20, c, sizeof(c));
        format_fr(e->tva_20, d, sizeof(d));
        format_fr(e->cb, cb, sizeof(cb));
        format_fr(e->especes, esp, sizeof(esp));
        
        // Sales 5.5% VAT (credit)
        fprintf(file, "VE;%s;7071;Vente 5,5%%;;%s\n", e->jour, a);
        // VAT 5.5% (credit)
        fprintf(file, "VE;%s;4457111;TVA 5,5%%;;%s\n", e->jour, b);
        // Sales 20% VAT (credit)
        fprintf(file, "VE;%s;7072;Vente 20%%;;%s\n", e->jour, c);
        // VAT 20% (credit)
        fprintf(file, "VE;%s;445711;TVA 20%%;;%s\n", e->jour, d);
        // Credit card payment (debit)
        fprintf(file, "VE;%s;580CB;CB;%s;\n", e->jour, cb);
        // Cash payment (debit)
        fprintf(file, "VE;%s;530;Especes;%s;\n", e->jour, esp);
    }
    
    fclose(file);
    return 1;
}

// Create output filename based on month and year
void create_output_filename(char *output_filename, size_t size) {
    time_t now;
    struct tm *time_info;
    
    time(&now);
    time_info = localtime(&now);
    
    int month = time_info->tm_mon + 1; // tm_mon is 0-11
    int year = time_info->tm_year + 1900;
    
    char *month_name = get_month_name(month);
    
    snprintf(output_filename, size, "journal VE %s %d.csv", month_name, year);
}

// Extract month and year from filename
void extract_month_year(const char *filename, int *month, int *year) {
    // Look for month names in the filename
    const char *month_names[] = {
        "Janvier", "Fevrier", "Mars", "Avril", "Mai", "Juin",
        "Juillet", "Aout", "Septembre", "Octobre", "Novembre", "Decembre"
    };
    
    *month = 0;
    *year = 0;
    
    // Check each month name
    for (int i = 0; i < 12; i++) {
        if (strstr(filename, month_names[i])) {
            *month = i + 1;
            break;
        }
    }
    
    // Extract year - look for 4 consecutive digits
    const char *ptr = filename;
    while (*ptr) {
        if (isdigit(*ptr) && isdigit(*(ptr+1)) && isdigit(*(ptr+2)) && isdigit(*(ptr+3))) {
            *year = atoi(ptr);
            break;
        }
        ptr++;
    }
}

// Main function to process files and create journal
int main(int argc, char *argv[]) {
    char ca_filename[256] = "/Users/igilbert/Desktop/Projets/ParserBocal/assets/Journal Vente Fevrier 2025/CAISSE-CA Fevrier 2025.csv";
    char reglement_filename[256] = "/Users/igilbert/Desktop/Projets/ParserBocal/assets/Journal Vente Fevrier 2025/CAISSE-Reglement Fevrier 2025.csv";
    char output_filename[256];
    
    // Allow command line arguments for filenames
    if (argc >= 3) {
        strncpy(ca_filename, argv[1], sizeof(ca_filename) - 1);
        strncpy(reglement_filename, argv[2], sizeof(reglement_filename) - 1);
    }
    
    printf("Processing files:\n1. %s\n2. %s\n", ca_filename, reglement_filename);
    
    // Create output filename based on input
    int month, year;
    extract_month_year(ca_filename, &month, &year);
    
    if (month > 0 && year > 0) {
        char *month_name = get_month_name(month);
        snprintf(output_filename, sizeof(output_filename), "journal VE %s %d.csv", month_name, year);
    } else {
        create_output_filename(output_filename, sizeof(output_filename));
    }
    
    printf("Output will be written to: %s\n", output_filename);
    
    // Read sales and payment data
    SalesData sales_data[MAX_ENTRIES];
    PaymentData payment_data[MAX_ENTRIES];
    JournalEntry journal_entries[MAX_ENTRIES];
    
    int sales_count = read_sales_data(ca_filename, sales_data, MAX_ENTRIES);
    int payment_count = read_payment_data(reglement_filename, payment_data, MAX_ENTRIES);
    
    if (sales_count == 0 || payment_count == 0) {
        fprintf(stderr, "Error: No data read from input files. Aborting.\n");
        return 1;
    }
    
    // Combine data and create journal
    int entry_count = combine_data(sales_data, sales_count, payment_data, payment_count, journal_entries);
    
    if (entry_count == 0) {
        fprintf(stderr, "Error: No matching entries found. Check date formats in input files.\n");
        return 1;
    }
    
    // Write output file
    if (!write_journal_file(output_filename, journal_entries, entry_count)) {
        fprintf(stderr, "Error writing to output file: %s\n", output_filename);
        return 1;
    }
    
    printf("Successfully processed %d entries and wrote to %s\n", entry_count, output_filename);
    return 0;
}

