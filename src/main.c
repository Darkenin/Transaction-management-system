#include "sqlite3.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "cJSON.c"

typedef struct account {
    char name[100];
    int acctNum;
    double balance;
    char description[100];
} Account;

static int callback(void *data, int argc, char **argv, char **azColName){
    const char* context = (const char*)data; // Cast data back to its original type

    printf("Context: %s\n", context);
    int i;
    for ( i = 0; i < argc; i++)
    {
        printf("%s = %s\n",azColName[i], argv[i] ? argv[i] : NULL);
    }
    printf("\n");
    return 0;
}
int restart();
void saveMoney(sqlite3 *db);
void deductMoney(sqlite3 *db);
void liquidateAccount(sqlite3 *db);
void createAccount(sqlite3 *db);
void viewAccountReport(sqlite3 *db);
void startConsole();
int startTransactions();
void convertToJson(Account acct);
int seq();

int main(int argc, char const *argv[])
{
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc, n, res;
    char *sql;
    const char* data = "Database records";
    Account acct;

    /* open database */
    rc = sqlite3_open("test.db", &db);

    if(rc){
        fprintf(stderr,"Can't open database: %s\n", sqlite3_errmsg(db));
        return 0;
    } else{
        fprintf(stdout,"Opened database: \n");
    }

   sql = "CREATE TABLE ACCOUNT("
          "ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
          "NAME TEXT NOT NULL, "
          "ACCOUNT_NUMBER INT NOT NULL, "
          "BALANCE DOUBLE NOT NULL, "
          "DESCRIPTION TEXT NOT NULL);";

    rc = sqlite3_exec(db,sql,callback,(void*)data, &zErrMsg);

    if(rc != SQLITE_OK){
        fprintf(stderr,"SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return 0;
    } else{
        fprintf(stdout,"Table created Successfully\n");
    }
    startConsole();
    // n = startTransactions();
    while ((n = startTransactions()) != EOF)
    {
        if(!isspace(n) && !isalnum(n )&& !(n >= 5) ){

            if (n ==1){
            createAccount(db);
            }
            else if(n ==2){
                saveMoney(db);
            }
            else if(n ==3){
                deductMoney(db);
            }
            else if(n ==4){
                viewAccountReport(db);
            }
            else
                liquidateAccount(db);
        }

        else{
            printf("Invalid option. Please select a valid option (1-5).\n");
            continue;
        }

        res = restart();
        if (res==2){
             printf("Thanks for your time.\n");
            continue;
        }
        else
            break;
    }

    printf("%c Raphael\n", 153);

    sqlite3_close(db);
    return 0;

}

int restart(){
  int n = seq();
  return n;
}


/*Function to save money to an account*/
void saveMoney(sqlite3 *db) {
    char *zErrMsg = 0;
    int rc;
    int accountID;
    double amount;
    char sql[100];
    printf("Input the account and amount");
    scanf("%d %lf",&accountID,&amount);

    sprintf(sql, "UPDATE ACCOUNT SET BALANCE = BALANCE + %.2f WHERE ID = %d;", amount, accountID);

    rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        printf("Saved $%.2f to account %d\n", amount, accountID);
    }
}

/* Function to deduct money from an account*/
void deductMoney(sqlite3 *db) {
    char *zErrMsg = 0;
    int rc;
    int accountID;
    double amount;
    char sql[100];

    printf("Input the account and amount");
    scanf("%d %lf",&accountID,&amount);


    sprintf(sql, "UPDATE ACCOUNT SET BALANCE = BALANCE - %.2f WHERE ID = %d;", amount, accountID);

    rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        printf("Deducted $%.2f from account %d\n", amount, accountID);
    }
}

/*Function to liquidate an account (set balance to 0)*/
void liquidateAccount(sqlite3 *db) {
    char *zErrMsg = 0;
    int rc;
    char sql[100];
    int accountID;

    printf("Input the accountID and amount");
    scanf("%d %lf",&accountID);


    sprintf(sql, "UPDATE ACCOUNT SET BALANCE = 0 WHERE ID = %d;", accountID);

    rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        printf("Account %d has been liquidated (balance set to 0)\n", accountID);
    }
}

/*Function to create an account */
void createAccount(sqlite3 *db) {
    char *zErrMsg = 0;
    int rc;
    Account acct;
    char sql[100];

    // Use parameterized query with placeholders
    sprintf(sql, "INSERT INTO ACCOUNT (NAME, ACCOUNT_NUMBER, BALANCE, DESCRIPTION) VALUES (?, ?, ?, ?);");

    sqlite3_stmt *stmt;

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL prepare error: %s\n", sqlite3_errmsg(db));
        return;
    }

    // Get console input for the statement values
    printf("Enter account name:\n ");
    scanf("%s", acct.name);

    printf("Enter account number: \n");
    scanf("%d", &acct.acctNum);

    printf("Enter account balance: \n");
    scanf("%lf", &acct.balance);

    printf("Enter account description:\n ");
    scanf("%s", acct.description);

    convertToJson(acct);

    // Bind values to placeholders
    sqlite3_bind_text(stmt, 1, acct.name, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, acct.acctNum);
    sqlite3_bind_double(stmt, 3, acct.balance);
    sqlite3_bind_text(stmt, 4, acct.description, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "SQL execution error: %s\n", sqlite3_errmsg(db));
    } else {
        printf("Account %s has been created\n", acct.name);
    }

    sqlite3_finalize(stmt);
}

/* Function to view user account record*/
void viewAccountReport(sqlite3 *db) {
    char *zErrMsg = 0;
    int rc;
    char sql[100];
     int accountID;
     const char* data = "Account records";

    scanf("%d",&accountID);

    sprintf(sql, "SELECT * FROM ACCOUNT WHERE ID = %d;",accountID);

    rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        printf("This are the Account report\n");
    }
}

void startConsole(){
    char name[20];
    printf("\t*** Welcome to the Transaction Management system(TMS) ***\t\n\n");
    printf("What is the name: ");
    scanf("%s",name);
    printf("Welcome %s\n\n",name);
    printf("-------********------\n");
    printf("The application provides options for\n\n");
    printf("1. Account opening\n");
    printf("2. Saving\n");
    printf("3. Withdrawals\n");
    printf("4. View Account record\n\a");
    printf("5. Liquidate Accounts\n\n");

    // printf("\tSelect an option between 1 - 5:  ");


}

int startTransactions(){
    int n;
    printf("\tSelect an option between 1 - 5:  ");
    scanf("%d",&n);

    return n;
}


void convertToJson(Account acct){
    // create a cJSON object
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "acct_name", acct.name);
    cJSON_AddNumberToObject(json, "acctnum", acct.acctNum);
    cJSON_AddNumberToObject(json, "balance", acct.balance);
    cJSON_AddStringToObject(json, "description", acct.description);

    // convert the cJSON object to a JSON string
    char *json_str = cJSON_Print(json);

    // write the JSON string to a file
    FILE *fp = fopen("data.json", "w");
    if (fp == NULL) {
        printf("Error: Unable to open the file.\n");
    }
    printf("%s\n", json_str);
    fputs(json_str, fp);
    fclose;
    // free the JSON string and cJSON object
    cJSON_free(json_str);
    cJSON_Delete(json);
}
/*
 * Checks if the user wants to continue the program.
 * Returns 0 if the user wants to terminate the program, and 1 otherwise.
 */
int seq(){
    int type;
    printf("Would you like to continue (1/2):\n");
    scanf("%d", &type);


    switch (type)
    {
        case 1:
            printf("Thanks for using this program\n\n");
            break;
        case 2:
            printf("Restarting the program...\n");
            printf("1. Account opening\n");
            printf("2. Save money\n");
            printf("3. Withdrawal\n");
            printf("4. View Account record\n\a");
            printf("5. Liquidate Account\n");
            printf("Enter the option 1-5 \n");
            break;
    }
    return type;
}

// 1. create account  -- done
// 2. view report   -- done
// 3. check balance  -- done
// 4. printf closeing terminal message -- done
// 5. Add cJSON capability

// -1: This indicates that SQLite should automatically
// determine the length of acct.name by looking for the null terminator.
// SQLITE_STATIC: The destructor function (in this case,
//  indicating that SQLite should not attempt to free the memory associated with acct.name).