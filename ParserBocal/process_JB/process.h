/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   process.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: igilbert <igilbert@student.42perpignan.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/03 12:12:38 by igilbert          #+#    #+#             */
/*   Updated: 2025/05/06 12:20:59 by igilbert         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PROCESS_H
# define PROCESS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_SIZE 2048
#define MAX_FIELD_SIZE 256
#define MAX_OPERATIONS 10
#define MAX_ACCOUNTS 2000

// Structure for a bank operation from the source file
typedef struct {
    char date[MAX_FIELD_SIZE];
    char operation[MAX_FIELD_SIZE];
    char debit[MAX_FIELD_SIZE];
    char credit[MAX_FIELD_SIZE];
    char devise[MAX_FIELD_SIZE];
    char date_valeur[MAX_FIELD_SIZE];
    char libelle[MAX_FIELD_SIZE];
    char details[MAX_FIELD_SIZE];
} BankOperation;

// Structure for a journal entry in the target format
typedef struct {
    char journal[MAX_FIELD_SIZE];
    char jour[MAX_FIELD_SIZE];
    char compte[MAX_FIELD_SIZE];
    char libelle[MAX_FIELD_SIZE];
    char debit[MAX_FIELD_SIZE];
    char credit[MAX_FIELD_SIZE];
} JournalEntry;

// Structure for an account from the chart of accounts
typedef struct {
    char number[MAX_FIELD_SIZE];
    char name[MAX_FIELD_SIZE];
} AccountInfo;

// Function to parse a line from the bank statement
int parse_bank_operation(char *line, BankOperation *operation);

// Function to determine the account code based on operation type
void determine_accounts(BankOperation *operation, JournalEntry *entries, int *entry_count, 
                        AccountInfo *accounts, int account_count);

// Function to convert a bank operation to journal entries
int convert_to_journal_entries(BankOperation *operation, JournalEntry *entries, 
                               AccountInfo *accounts, int account_count);

// Function to write a journal entry to the output file
void write_journal_entry(FILE *output, JournalEntry *entry);

// Main processing function
int process_bank_statement(FILE *input, FILE *output, const char *chart_of_accounts_file);

// Function wrapper for compatibility with main.c
int process_csv_file(FILE *input, FILE *output, const char *chart_of_accounts_file);

// Function to load the chart of accounts
int load_chart_of_accounts(const char *filename, AccountInfo *accounts, int max_accounts);

// Function to find an account by keyword in the name
const char* find_account_by_keyword(const char *keyword, AccountInfo *accounts, int account_count);

// Utility function to clean a string (remove quotes, trim whitespace)
void clean_string(char *str);

// Utility function to format decimal numbers with comma as separator
void format_number(char *str);

#endif

