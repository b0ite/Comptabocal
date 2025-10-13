/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   process.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: igilbert <igilbert@student.42perpignan.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/03 12:12:37 by igilbert          #+#    #+#             */
/*   Updated: 2025/09/08 15:18:39 by igilbert         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "process.h"

// --- helpers ---------------------------------------------------------------
static void build_401_from_label(const char *label, char *out, size_t outsz) {
    // Build account like 401 + UPPERCASE(ALNUM(label)) with spaces and punctuation removed
    if (!out || outsz == 0) return;
    size_t j = 0;
    if (outsz > 1) out[j++] = '4';
    if (j < outsz - 1) out[j++] = '0';
    if (j < outsz - 1) out[j++] = '1';
    if (!label) { out[j] = '\0'; return; }
    for (const unsigned char *p = (const unsigned char *)label; *p && j < outsz - 1; ++p) {
        unsigned char c = *p;
        if (isalnum(c)) {
            out[j++] = (char)toupper(c);
        }
        // Skip non-alnum (spaces, punctuation, diacritics bytes)
    }
    out[j] = '\0';
}
static void trim_whitespace(char *s) {
    if (!s) return;
    // Trim trailing CR/LF/space
    size_t len = strlen(s);
    while (len > 0 && (s[len-1] == '\r' || s[len-1] == '\n' || isspace((unsigned char)s[len-1]))) {
        s[--len] = '\0';
    }
    // Trim leading spaces
    size_t start = 0;
    while (s[start] && isspace((unsigned char)s[start])) start++;
    if (start) memmove(s, s + start, strlen(s + start) + 1);
}

static void remove_surrounding_quotes(char *s) {
    if (!s || !*s) return;
    size_t len = strlen(s);
    if (len >= 2) {
        char first = s[0];
        char last = s[len - 1];
        if ((first == '"' && last == '"') || (first == '\'' && last == '\'')) {
            s[len - 1] = '\0';
            memmove(s, s + 1, len - 1);
        }
    }
}

static int icompare(const char *a, const char *b) {
    // case-insensitive strcmp
    unsigned char ca, cb;
    while (*a && *b) {
        ca = (unsigned char)tolower((unsigned char)*a);
        cb = (unsigned char)tolower((unsigned char)*b);
        if (ca != cb) return (int)ca - (int)cb;
        a++; b++;
    }
    return (int)(unsigned char)tolower((unsigned char)*a) - (int)(unsigned char)tolower((unsigned char)*b);
}

static int icontains(const char *haystack, const char *needle) {
    if (!haystack || !needle || !*needle) return 0;
    size_t nlen = strlen(needle);
    for (const char *p = haystack; *p; p++) {
        if (strncasecmp(p, needle, nlen) == 0) return 1;
    }
    return 0;
}

static void normalize_amount_positive(const char *src, char *dst) {
    // Copy, drop leading '-', trim, and format decimal point to comma
    if (!src || !dst) return;
    size_t j = 0;
    for (size_t i = 0; src[i]; i++) {
        if (i == 0 && src[i] == '-') continue; // drop leading minus
        dst[j++] = src[i];
    }
    dst[j] = '\0';
    // remove spaces inside
    size_t w = 0;
    for (size_t i = 0; dst[i]; i++) {
        if (!isspace((unsigned char)dst[i])) dst[w++] = dst[i];
    }
    dst[w] = '\0';
    // cleanup and format
    // ...existing code...
    // Reuse existing clean_string + format_number
    clean_string(dst);
    format_number(dst);
    if (dst[0] == '\0') strcpy(dst, "0");
}

// Clean a string by removing quotes and trimming whitespace
void clean_string(char *str) {
    if (!str || *str == '\0')
        return;
    trim_whitespace(str);
    remove_surrounding_quotes(str);
    trim_whitespace(str);
}

// Format number with comma as decimal separator
void format_number(char *str) {
    if (!str || *str == '\0')
        return;

    // Remove quotes & surrounding spaces
    clean_string(str);

    // Replace decimal point with comma
    char *dot = strchr(str, '.');
    if (dot) {
        *dot = ',';
    }
}

// Function to load the chart of accounts
int load_chart_of_accounts(const char *filename, AccountInfo *accounts, int max_accounts) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Could not open chart of accounts file %s\n", filename);
        return 0;
    }
    
    char line[MAX_LINE_SIZE];
    int count = 0;
    int line_count = 0;
    
    // Skip header line
    if (fgets(line, MAX_LINE_SIZE, file) == NULL) {
        fclose(file);
        return 0;
    }
    
    // Read accounts
    while (fgets(line, MAX_LINE_SIZE, file) && count < max_accounts) {
        line_count++;
        
        // Skip empty lines
        if (strlen(line) <= 1)
            continue;
            
        char *token;
        char *rest = line;
        
        // Extract account number
        token = strtok(rest, ";");
        if (token) {
            strncpy(accounts[count].number, token, MAX_FIELD_SIZE - 1);
            accounts[count].number[MAX_FIELD_SIZE - 1] = '\0';
            clean_string(accounts[count].number);
        } else {
            continue;
        }
        
        // Extract account name
        token = strtok(NULL, ";");
        if (token) {
            strncpy(accounts[count].name, token, MAX_FIELD_SIZE - 1);
            accounts[count].name[MAX_FIELD_SIZE - 1] = '\0';
            clean_string(accounts[count].name);
            count++;
        }
    }
    
    fclose(file);
    printf("Loaded %d accounts from %s\n", count, filename);
    return count;
}

// Function to find an account by keyword in the name
const char* find_account_by_keyword(const char *keyword, AccountInfo *accounts, int account_count) {
    if (!keyword || strlen(keyword) == 0)
        return NULL;

    // First, try exact match on account number (case-insensitive just in case)
    for (int i = 0; i < account_count; i++) {
        if (icompare(accounts[i].number, keyword) == 0) {
            return accounts[i].number;
        }
    }

    // Then, search for the keyword in account names (case-insensitive)
    for (int i = 0; i < account_count; i++) {
        if (icontains(accounts[i].name, keyword)) {
            return accounts[i].number;
        }
    }

    // If 401+keyword exists (common for suppliers)
    char supplier_code[MAX_FIELD_SIZE];
    snprintf(supplier_code, sizeof(supplier_code), "401%s", keyword);
    for (int i = 0; i < account_count; i++) {
        if (icompare(accounts[i].number, supplier_code) == 0) {
            return accounts[i].number;
        }
    }

    return NULL;
}

// Parse a line from the bank statement into a BankOperation structure
int parse_bank_operation(char *line, BankOperation *operation) {
    char *token;
    char *rest = line;
    char temp[MAX_FIELD_SIZE] = {0};
    int field_count = 0;

    // Initialize the operation fields
    memset(operation, 0, sizeof(BankOperation));

    // Parse the line field by field
    token = strtok(rest, ";");
    if (token) {
        strncpy(operation->date, token, MAX_FIELD_SIZE - 1);
        clean_string(operation->date);
        field_count++;
    } else {
        return 0; // Not enough fields
    }

    token = strtok(NULL, ";");
    if (token) {
        strncpy(operation->operation, token, MAX_FIELD_SIZE - 1);
        clean_string(operation->operation);
        field_count++;
    } else {
        return 0;
    }

    token = strtok(NULL, ";");
    if (token) {
        strncpy(operation->debit, token, MAX_FIELD_SIZE - 1);
        clean_string(operation->debit);
        field_count++;
    } else {
        return 0;
    }

    token = strtok(NULL, ";");
    if (token) {
        strncpy(operation->credit, token, MAX_FIELD_SIZE - 1);
        clean_string(operation->credit);
        field_count++;
    } else {
        return 0;
    }

    token = strtok(NULL, ";");
    if (token) {
        strncpy(operation->devise, token, MAX_FIELD_SIZE - 1);
        clean_string(operation->devise);
        field_count++;
    } else {
        return 0;
    }

    token = strtok(NULL, ";");
    if (token) {
        strncpy(operation->date_valeur, token, MAX_FIELD_SIZE - 1);
        clean_string(operation->date_valeur);
        field_count++;
    } else {
        return 0;
    }

    token = strtok(NULL, ";");
    if (token) {
        strncpy(operation->libelle, token, MAX_FIELD_SIZE - 1);
        clean_string(operation->libelle);
        field_count++;
    }

    // If this operation has details, store it
    token = strtok(NULL, "\n");
    if (token) {
        strncpy(operation->details, token, MAX_FIELD_SIZE - 1);
        clean_string(operation->details);
    }

    return field_count;
}

// Convert a bank operation to journal entries
int convert_to_journal_entries(BankOperation *operation, JournalEntry *entries, 
                              AccountInfo *accounts, int account_count) {
    int entry_count = 0;

    // Skip empty lines or if date is empty
    if (strlen(operation->date) == 0) {
        return 0;
    }

    // Skip header lines or bank information
    if (operation->date[0] != '0' && operation->date[0] != '1' && 
        operation->date[0] != '2' && operation->date[0] != '3') {
        return 0;
    }

    // Resolve common accounts via plan comptable with fallback
    const char *acc_5121 = find_account_by_keyword("5121", accounts, account_count);
    if (!acc_5121) acc_5121 = "5121";
    const char *acc_580 = find_account_by_keyword("580", accounts, account_count);
    if (!acc_580) acc_580 = "580";
    const char *acc_627 = find_account_by_keyword("627", accounts, account_count);
    if (!acc_627) acc_627 = "627";

    // REMISE CB operations (Card payments received)
    if (strstr(operation->operation, "REMISE CB")) {
        // First entry: Credit clearing account with gross amount
        strcpy(entries[entry_count].journal, "BP");
        strcpy(entries[entry_count].jour, operation->date);
        strcpy(entries[entry_count].compte, acc_580);
        strcpy(entries[entry_count].libelle, "CB");
        strcpy(entries[entry_count].debit, "");
        // Extract gross amount from BT line
        char gross_amount[MAX_FIELD_SIZE] = {0};
        if (strstr(operation->details, "BT ")) {
            char *bt_start = strstr(operation->details, "BT ");
            char *e_pos = strstr(bt_start, "E COM");
            if (e_pos) {
                size_t len = (size_t)(e_pos - (bt_start + 2));
                if (len >= sizeof(gross_amount)) len = sizeof(gross_amount) - 1;
                strncpy(gross_amount, bt_start + 2, len);
                gross_amount[len] = '\0';
                clean_string(gross_amount);
                format_number(gross_amount);
                strcpy(entries[entry_count].credit, gross_amount);
            } else {
                char tmp[MAX_FIELD_SIZE];
                strncpy(tmp, operation->credit, sizeof(tmp) - 1); tmp[sizeof(tmp) - 1] = '\0';
                format_number(tmp);
                strcpy(entries[entry_count].credit, tmp);
            }
        } else {
            char tmp[MAX_FIELD_SIZE];
            strncpy(tmp, operation->credit, sizeof(tmp) - 1); tmp[sizeof(tmp) - 1] = '\0';
            format_number(tmp);
            strcpy(entries[entry_count].credit, tmp);
        }
        entry_count++;

        // Second entry: Debit commission fees
        strcpy(entries[entry_count].journal, "BP");
        strcpy(entries[entry_count].jour, operation->date);
        strcpy(entries[entry_count].compte, acc_627);
        strcpy(entries[entry_count].libelle, "CB");
        char commission[MAX_FIELD_SIZE] = {0};
        if (strstr(operation->details, "COM ")) {
            char *com_start = strstr(operation->details, "COM ");
            if (com_start) {
                char *e_pos = strstr(com_start, "E");
                if (e_pos && e_pos > com_start + 4) {
                    size_t len = (size_t)(e_pos - (com_start + 4));
                    if (len >= sizeof(commission)) len = sizeof(commission) - 1;
                    strncpy(commission, com_start + 4, len);
                    commission[len] = '\0';
                }
            }
        }
        if (commission[0]) {
            clean_string(commission);
            format_number(commission);
            strcpy(entries[entry_count].debit, commission);
            strcpy(entries[entry_count].credit, "");
        } else {
            strcpy(entries[entry_count].debit, "0");
            strcpy(entries[entry_count].credit, "");
        }
        entry_count++;

        // Third entry: Debit bank account with net credited amount
        strcpy(entries[entry_count].journal, "BP");
        strcpy(entries[entry_count].jour, operation->date);
        strcpy(entries[entry_count].compte, acc_5121);
        strcpy(entries[entry_count].libelle, "CB");
        char net_amt[MAX_FIELD_SIZE];
        strncpy(net_amt, operation->credit, sizeof(net_amt) - 1); net_amt[sizeof(net_amt) - 1] = '\0';
        format_number(net_amt);
        strcpy(entries[entry_count].debit, net_amt);
        strcpy(entries[entry_count].credit, "");
        entry_count++;
    }
    // CARTE X0067 operations (Card payments made)
    else if (strstr(operation->operation, "CARTE X0067")) {
        char libelle[MAX_FIELD_SIZE] = {0};
        char *space = strchr(operation->operation + 11, ' ');
        if (space) {
            strncpy(libelle, space + 1, sizeof(libelle) - 1);
        } else {
            strcpy(libelle, "CARTE BANCAIRE");
        }

        // Determine account code based on the payment
        char compte[MAX_FIELD_SIZE] = {0};

        // Try to find account based on libelle
        const char *found_account = NULL;

        // Special cases handling
        if (strstr(libelle, "FACEBK")) {
            found_account = find_account_by_keyword("FACEBOOK", accounts, account_count);
            strcpy(libelle, "PUB FACEBOOK");
        }
        else if (strstr(libelle, "AMAZON")) {
            found_account = find_account_by_keyword("AMAZON", accounts, account_count);
        }
        else if (strstr(libelle, "LEROY MERLIN") || strstr(libelle, "ADEO*LEROY")) {
            found_account = find_account_by_keyword("LEROYMERLIN", accounts, account_count);
            strcpy(libelle, "LEROY MERLIN");
        }
        else if (strstr(libelle, "AVERY")) {
            found_account = find_account_by_keyword("AVERY", accounts, account_count);
        }
        else if (strstr(libelle, "ORANGE")) {
            found_account = find_account_by_keyword("ORANGE", accounts, account_count);
        }
        else if (strstr(libelle, "ORANAISE")) {
            found_account = find_account_by_keyword("RESTAURANT", accounts, account_count);
            strcpy(libelle, "Restaurant");
        }
        else {
            // Try to find a matching account by extracting keywords from libelle (without modifying libelle)
            char libelle_copy[MAX_FIELD_SIZE];
            strncpy(libelle_copy, libelle, sizeof(libelle_copy) - 1);
            libelle_copy[sizeof(libelle_copy) - 1] = '\0';
            char *word = strtok(libelle_copy, " ");
            while (word && !found_account) {
                if (strlen(word) > 3) {
                    found_account = find_account_by_keyword(word, accounts, account_count);
                }
                word = strtok(NULL, " ");
            }
        }

        // Default account if no match found -> 401 + UPPER(libelle) without spaces
        if (found_account) {
            strcpy(compte, found_account);
        } else {
            build_401_from_label(libelle, compte, sizeof(compte));
        }

        // First entry: Debit to supplier account
        strcpy(entries[entry_count].journal, "BP");
        strcpy(entries[entry_count].jour, operation->date);
        strcpy(entries[entry_count].compte, compte);
        strcpy(entries[entry_count].libelle, libelle);
        // Clean the debit amount (remove minus sign and format)
        char clean_debit[MAX_FIELD_SIZE];
        normalize_amount_positive(operation->debit, clean_debit);
        strcpy(entries[entry_count].debit, clean_debit);
        strcpy(entries[entry_count].credit, "");
        entry_count++;

        // Second entry: Credit from bank account
        strcpy(entries[entry_count].journal, "BP");
        strcpy(entries[entry_count].jour, operation->date);
        strcpy(entries[entry_count].compte, acc_5121);
        strcpy(entries[entry_count].libelle, libelle);
        strcpy(entries[entry_count].debit, "");
        strcpy(entries[entry_count].credit, clean_debit);
        entry_count++;
    }
    // VRST GAB operations (Cash deposits)
    else if (strstr(operation->operation, "VRST GAB")) {
        // First entry: Credit to cash clearing (580)
        strcpy(entries[entry_count].journal, "BP");
        strcpy(entries[entry_count].jour, operation->date);
        strcpy(entries[entry_count].compte, acc_580);
        strcpy(entries[entry_count].libelle, "Versement especes");
        strcpy(entries[entry_count].debit, "");
        char credit_amt[MAX_FIELD_SIZE];
        strncpy(credit_amt, operation->credit, sizeof(credit_amt) - 1); credit_amt[sizeof(credit_amt) - 1] = '\0';
        format_number(credit_amt);
        strcpy(entries[entry_count].credit, credit_amt);
        entry_count++;

        // Second entry: Debit to bank account
        strcpy(entries[entry_count].journal, "BP");
        strcpy(entries[entry_count].jour, operation->date);
        strcpy(entries[entry_count].compte, acc_5121);
        strcpy(entries[entry_count].libelle, "Versement especes");
        strcpy(entries[entry_count].debit, credit_amt);
        strcpy(entries[entry_count].credit, "");
        entry_count++;
    }
    // VIR RECU operations (Received transfers)
    else if (strstr(operation->operation, "VIR RECU")) {
        // Determine account code and description based on details
        char compte[MAX_FIELD_SIZE] = {0};
        const char *acc_default = find_account_by_keyword("44567", accounts, account_count);
        strcpy(compte, acc_default ? acc_default : "44567");
        char libelle[MAX_FIELD_SIZE] = "Remboursement TVA";

        if (strstr(operation->details, "SIE MOSSON")) {
            const char *found_account = find_account_by_keyword("44567", accounts, account_count);
            if (found_account) {
                strcpy(compte, found_account);
            }
            strcpy(libelle, "Remboursement TVA");
        }

        // First entry: Credit appropriate account
        strcpy(entries[entry_count].journal, "BP");
        strcpy(entries[entry_count].jour, operation->date);
        strcpy(entries[entry_count].compte, compte);
        strcpy(entries[entry_count].libelle, libelle);
        strcpy(entries[entry_count].debit, "");
        char amt[MAX_FIELD_SIZE];
        strncpy(amt, operation->credit, sizeof(amt) - 1); amt[sizeof(amt) - 1] = '\0';
        format_number(amt);
        strcpy(entries[entry_count].credit, amt);
        entry_count++;

        // Second entry: Debit bank account
        strcpy(entries[entry_count].journal, "BP");
        strcpy(entries[entry_count].jour, operation->date);
        strcpy(entries[entry_count].compte, acc_5121);
        strcpy(entries[entry_count].libelle, libelle);
        strcpy(entries[entry_count].debit, amt);
        strcpy(entries[entry_count].credit, "");
        entry_count++;
    }
    // PRELEVEMENT EUROPEEN operations (Direct debits)
    else if (strstr(operation->operation, "PRELEVEMENT EUROPEEN")) {
        // Determine account code and description based on details
        char compte[MAX_FIELD_SIZE] = {0};
        const char *acc_default = find_account_by_keyword("401DIVERS", accounts, account_count);
        strcpy(compte, acc_default ? acc_default : "401DIVERS");
        char libelle[MAX_FIELD_SIZE] = "Prelevement";
        const char *found_account = NULL;

        if (strstr(operation->details, "CEP TRESO SANTE PREV")) {
            found_account = find_account_by_keyword("4375", accounts, account_count);
            strcpy(libelle, "Prevoyance");
        } else if (strstr(operation->details, "AXA")) {
            found_account = find_account_by_keyword("6161", accounts, account_count);
            strcpy(libelle, "AXA");
        } else if (strstr(operation->details, "URSSAF")) {
            if (strstr(operation->details, "FEV")) {
                found_account = find_account_by_keyword("644101", accounts, account_count);
                strcpy(libelle, "URSSAF ASF");
            } else {
                found_account = find_account_by_keyword("431", accounts, account_count);
                strcpy(libelle, "URSSAF");
            }
        } else if (strstr(operation->details, "HAXE DIRECT")) {
            found_account = find_account_by_keyword("HAXE DIRECT", accounts, account_count);
            strcpy(libelle, "HAXE DIRECT");
        } else if (strstr(operation->details, "GC RE HOKODO")) {
            found_account = find_account_by_keyword("ANKORSTORE", accounts, account_count);
            strcpy(libelle, "Ankorstore");
        } else if (strstr(operation->details, "METAC")) {
            found_account = find_account_by_keyword("TIME", accounts, account_count);
            strcpy(libelle, "TIME METAC");
        } else if (strstr(operation->details, "IONOS")) {
            found_account = find_account_by_keyword("IONOS", accounts, account_count);
            strcpy(libelle, "IONOS");
        }

        if (found_account) {
            strcpy(compte, found_account);
        } else {
            build_401_from_label(libelle, compte, sizeof(compte));
        }

        // First entry: Debit to appropriate account
        strcpy(entries[entry_count].journal, "BP");
        strcpy(entries[entry_count].jour, operation->date);
        strcpy(entries[entry_count].compte, compte);
        strcpy(entries[entry_count].libelle, libelle);
        char clean_debit[MAX_FIELD_SIZE];
        normalize_amount_positive(operation->debit, clean_debit);
        strcpy(entries[entry_count].debit, clean_debit);
        strcpy(entries[entry_count].credit, "");
        entry_count++;

        // Second entry: Credit from bank account
        strcpy(entries[entry_count].journal, "BP");
        strcpy(entries[entry_count].jour, operation->date);
        strcpy(entries[entry_count].compte, acc_5121);
        strcpy(entries[entry_count].libelle, libelle);
        strcpy(entries[entry_count].debit, "");
        strcpy(entries[entry_count].credit, clean_debit);
        entry_count++;
    }
    // VIR EUROPEEN EMIS operations (Outgoing transfers)
    else if (strstr(operation->operation, "VIR EUROPEEN EMIS")) {
        // Determine account code and description based on details
        char compte[MAX_FIELD_SIZE] = {0};
        const char *acc_default = find_account_by_keyword("401DIVERS", accounts, account_count);
        strcpy(compte, acc_default ? acc_default : "401DIVERS");
        char libelle[MAX_FIELD_SIZE] = "Virement";
        const char *found_account = NULL;

        if (strstr(operation->details, "FREJAVILLE Carla")) {
            found_account = find_account_by_keyword("421", accounts, account_count);
            strcpy(libelle, "Salaire Janvier");
        } else if (strstr(operation->details, "SCOP EPICE")) {
            found_account = find_account_by_keyword("SCOPEPICE", accounts, account_count);
            strcpy(libelle, "SCOP EPICE");
        } else if (strstr(operation->details, "COMPAGNIE DU BICARBONATE")) {
            found_account = find_account_by_keyword("COMPAGNIEBIC", accounts, account_count);
            strcpy(libelle, "Cie Bicarbonate");
        } else if (strstr(operation->details, "ECODIS")) {
            found_account = find_account_by_keyword("ECODIS", accounts, account_count);
            strcpy(libelle, "ECODIS");
        } else if (strstr(operation->details, "SCI JC")) {
            found_account = find_account_by_keyword("SCIJC", accounts, account_count);
            strcpy(libelle, "Loyer Février 2025");
        }

        if (found_account) {
            strcpy(compte, found_account);
        } else {
            build_401_from_label(libelle, compte, sizeof(compte));
        }

        // First entry: Debit to appropriate account
        strcpy(entries[entry_count].journal, "BP");
        strcpy(entries[entry_count].jour, operation->date);
        strcpy(entries[entry_count].compte, compte);
        strcpy(entries[entry_count].libelle, libelle);
        char clean_debit[MAX_FIELD_SIZE];
        normalize_amount_positive(operation->debit, clean_debit);
        strcpy(entries[entry_count].debit, clean_debit);
        strcpy(entries[entry_count].credit, "");
        entry_count++;

        // Second entry: Credit from bank account
        strcpy(entries[entry_count].journal, "BP");
        strcpy(entries[entry_count].jour, operation->date);
        strcpy(entries[entry_count].compte, acc_5121);
        strcpy(entries[entry_count].libelle, libelle);
        strcpy(entries[entry_count].debit, "");
        strcpy(entries[entry_count].credit, clean_debit);
        entry_count++;
    }
    // COTISATION MENSUELLE operations (Bank fees)
    else if (strstr(operation->operation, "COTISATION MENSUELLE")) {
        const char *found_account = find_account_by_keyword("627", accounts, account_count);
        char compte[MAX_FIELD_SIZE];
        strcpy(compte, found_account ? found_account : acc_627);

        // First entry: Debit to bank fees account
        strcpy(entries[entry_count].journal, "BP");
        strcpy(entries[entry_count].jour, operation->date);
        strcpy(entries[entry_count].compte, compte);
        strcpy(entries[entry_count].libelle, "Cotisation Jazz Pro");
        char clean_debit[MAX_FIELD_SIZE];
        normalize_amount_positive(operation->debit, clean_debit);
        strcpy(entries[entry_count].debit, clean_debit);
        strcpy(entries[entry_count].credit, "");
        entry_count++;

        // Second entry: Credit from bank account
        strcpy(entries[entry_count].journal, "BP");
        strcpy(entries[entry_count].jour, operation->date);
        strcpy(entries[entry_count].compte, acc_5121);
        strcpy(entries[entry_count].libelle, "Cotisation Jazz Pro");
        strcpy(entries[entry_count].debit, "");
        strcpy(entries[entry_count].credit, clean_debit);
        entry_count++;
    }
    // COMMISSION RELEVE or COM REL operations (Bank fees)
    else if (strstr(operation->operation, "COMMISSION RELEVE") || 
             strstr(operation->operation, "COM REL")) {
        const char *found_account = find_account_by_keyword("627", accounts, account_count);
        char compte[MAX_FIELD_SIZE];
        strcpy(compte, found_account ? found_account : acc_627);

        // First entry: Debit to bank fees account
        strcpy(entries[entry_count].journal, "BP");
        strcpy(entries[entry_count].jour, operation->date);
        strcpy(entries[entry_count].compte, compte);
        strcpy(entries[entry_count].libelle, "Commission LCR");
        char clean_debit[MAX_FIELD_SIZE];
        normalize_amount_positive(operation->debit, clean_debit);
        strcpy(entries[entry_count].debit, clean_debit);
        strcpy(entries[entry_count].credit, "");
        entry_count++;

        // Second entry: Credit from bank account
        strcpy(entries[entry_count].journal, "BP");
        strcpy(entries[entry_count].jour, operation->date);
        strcpy(entries[entry_count].compte, acc_5121);
        strcpy(entries[entry_count].libelle, "Commission LCR");
        strcpy(entries[entry_count].debit, "");
        strcpy(entries[entry_count].credit, clean_debit);
        entry_count++;
    }
    // RELEVE LCR DOMICIL operations (Bank fees)
    else if (strstr(operation->operation, "RELEVE LCR DOMICIL")) {
        const char *found_account = find_account_by_keyword("EVOOTRADE", accounts, account_count);
        char compte[MAX_FIELD_SIZE];
        strcpy(compte, found_account ? found_account : "401EVOOTRADE");

        // First entry: Debit to supplier account
        strcpy(entries[entry_count].journal, "BP");
        strcpy(entries[entry_count].jour, operation->date);
        strcpy(entries[entry_count].compte, compte);
        strcpy(entries[entry_count].libelle, "EVOOTRADE");
        char clean_debit[MAX_FIELD_SIZE];
        normalize_amount_positive(operation->debit, clean_debit);
        strcpy(entries[entry_count].debit, clean_debit);
        strcpy(entries[entry_count].credit, "");
        entry_count++;

        // Second entry: Credit from bank account
        strcpy(entries[entry_count].journal, "BP");
        strcpy(entries[entry_count].jour, operation->date);
        strcpy(entries[entry_count].compte, acc_5121);
        strcpy(entries[entry_count].libelle, "EVOOTRADE");
        strcpy(entries[entry_count].debit, "");
        strcpy(entries[entry_count].credit, clean_debit);
        entry_count++;
    }

    return entry_count;
}

// Write a journal entry to the output file
void write_journal_entry(FILE *output, JournalEntry *entry) {
    fprintf(output, "%s;%s;%s;%s;%s;%s\n",
            entry->journal,
            entry->jour,
            entry->compte,
            entry->libelle,
            entry->debit,
            entry->credit);
}

// Process the bank statement and convert it to journal entries
int process_bank_statement(FILE *input, FILE *output, const char *chart_of_accounts_file) {
    char line[MAX_LINE_SIZE];
    BankOperation operation;
    JournalEntry entries[MAX_OPERATIONS];
    AccountInfo accounts[MAX_ACCOUNTS];
    int line_count = 0;
    int total_entries = 0;
    int header_written = 0;
    
    // Load chart of accounts
    int account_count = load_chart_of_accounts(chart_of_accounts_file, accounts, MAX_ACCOUNTS);
    if (account_count == 0) {
        fprintf(stderr, "Warning: No accounts loaded from %s. Using default account codes.\n", 
                chart_of_accounts_file);
    }
    
    // Skip header and bank information lines
    while (fgets(line, MAX_LINE_SIZE, input)) {
        line_count++;
        
        // Skip empty lines
        if (strlen(line) <= 1)
            continue;
            
        // Check if this is the headers line (Date;Nature de l'opération;...)
        if (strstr(line, "Date;Nature de l")) {
            // Write the header for the journal
            fprintf(output, "Journal;Jour;cpte;Libelle;Debit;Credit\n");
            header_written = 1;
            break;
        }
    }
    
    // If we couldn't find the headers line, return error
    if (!header_written) {
        fprintf(stderr, "Error: Could not find headers line in input file\n");
        return -1;
    }
    
    // Process each line of the input file
    while (fgets(line, MAX_LINE_SIZE, input)) {
        line_count++;
        
        // Skip empty lines
        if (strlen(line) <= 1)
            continue;

        // Skip lines that appear to be detail lines (starting with empty fields)
        if (line[0] == '"' && line[1] == '"')
            continue;
            
        // Parse the bank operation
        memset(&operation, 0, sizeof(BankOperation));
        if (parse_bank_operation(line, &operation) == 0)
            continue;
            
        // Skip operations without a date
        if (strlen(operation.date) == 0)
            continue;
            
        // Skip lines that don't look like valid operations
        if (operation.date[0] != '0' && operation.date[0] != '1' && 
            operation.date[0] != '2' && operation.date[0] != '3')
            continue;

        // Capture details from next line if it's a BT line (for commission calculation)
        if (strstr(operation.operation, "REMISE CB")) {
            char next_line[MAX_LINE_SIZE];
            long pos = ftell(input);
            if (fgets(next_line, MAX_LINE_SIZE, input)) {
                if (strstr(next_line, "BT ")) {
                    strncpy(operation.details, next_line, MAX_FIELD_SIZE - 1);
                    operation.details[MAX_FIELD_SIZE - 1] = '\0';
                    clean_string(operation.details);
                    line_count++;
                } else {
                    // Not a BT line, go back
                    fseek(input, pos, SEEK_SET);
                }
            }
        }
        
        // Convert the operation to journal entries
        memset(entries, 0, sizeof(entries));
        int entry_count = convert_to_journal_entries(&operation, entries, accounts, account_count);
        
        // Write the entries to the output file
        for (int i = 0; i < entry_count; i++) {
            write_journal_entry(output, &entries[i]);
            total_entries++;
        }
    }
    
    return total_entries;
}

// Wrapper function to maintain compatibility with main.c
int process_csv_file(FILE *input, FILE *output, const char *chart_of_accounts_file) {
    return process_bank_statement(input, output, chart_of_accounts_file);
}

