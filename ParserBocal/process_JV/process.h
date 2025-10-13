/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   process.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: igilbert <igilbert@student.42perpignan.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/18 15:50:41 by igilbert          #+#    #+#             */
/*   Updated: 2025/05/05 11:11:06 by igilbert         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PROCESS_H
#define PROCESS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <fcntl.h>

#define MAX_LINE_LENGTH 4096
#define MAX_DATE_LENGTH 20
#define MAX_ENTRIES 100
#define MAX_FIELD_LENGTH 256
#define MAX_FILENAME_LENGTH 512

// Structure to hold sales data from CAISSE-CA file
typedef struct {
    char date[MAX_DATE_LENGTH];
    double ca_ttc;
    double ca_ht;
    double vat_5_5_amount;
    double vat_5_5_ht;
    double vat_20_amount;
    double vat_20_ht;
} SalesData;

// Structure to hold payment data from CAISSE-Reglement file
typedef struct {
    char date[MAX_DATE_LENGTH];
    double especes;
    double cartes;
    double total;
} PaymentData;

// Structure to hold a combined journal entry
typedef struct {
    char date[MAX_DATE_LENGTH];
    char jour[MAX_FIELD_LENGTH]; // Julian date
    double vente_5_5;            // Sales at 5.5% VAT
    double tva_5_5;              // 5.5% VAT amount
    double vente_20;             // Sales at 20% VAT
    double tva_20;               // 20% VAT amount
    double cb;                   // Card payments
    double especes;              // Cash payments
} JournalEntry;

// Function prototypes
int read_sales_data(const char *filename, SalesData *sales_data, int max_entries);
int read_payment_data(const char *filename, PaymentData *payment_data, int max_entries);
int combine_data(SalesData *sales_data, int sales_count, 
                 PaymentData *payment_data, int payment_count,
                 JournalEntry *journal_entries);
int write_journal_file(const char *filename, JournalEntry *entries, int count);
char* get_month_name(int month);
void convert_date_to_julian(const char *date, char *julian);
void create_output_filename(char *output_filename, size_t size);
void extract_month_year(const char *filename, int *month, int *year);

// Helper functions
char* trim(char *str);
int parse_csv_line(char *line, char **fields, int max_fields, char delimiter);
double extract_number(const char *str);
void extract_vat_info(const char *line, double *vat_5_5_amount, double *vat_5_5_ht, 
                     double *vat_20_amount, double *vat_20_ht);

#endif /* PROCESS_H */