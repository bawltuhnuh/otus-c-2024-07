#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <math.h>

void handle_sqlite_error(int rc, const char *msg)
{
    if(rc != SQLITE_OK){
        fprintf(stderr, "%s\n", msg);
        exit(EXIT_FAILURE);
    }
}

double parse_number(const char* str)
{
    if (str == NULL) {
        return NAN;
    }
    char* endptr;
    double val = strtod(str, &endptr);

    if (endptr == str || *endptr != '\0') {
        return NAN;
    } else {
        return val;
    }
}

typedef struct {
    int count;
    double sum;
    double min_value;
    double max_value;
    double avg;
    double variance;
} Statistics;

static int first_pass_callback(void *data, int argc, char **argv, char **azColName)
{
    Statistics *stats = (Statistics *) data;

    double value = parse_number(argv[0]);

    if (!isnan(value)) {
        stats->count++;
        stats->sum += value;

        if(isnan(stats->min_value) || stats->min_value > value) {
            stats->min_value = value;
        }

        if(isnan(stats->max_value) || stats->max_value < value) {
            stats->max_value = value;
        }
    }

    return 0;
}

static int second_pass_callback(void *data, int argc, char **argv, char **azColName)
{
    Statistics *stats = (Statistics *) data;

    double value = parse_number(argv[0]);

    if (!isnan(value)) {
        double diff = value - stats->avg;
        stats->variance += diff * diff;
    }

    return 0;
}

int main(int argc, char** argv)
{
    if(argc != 4) {
        printf("Usage: %s db_file table_name column_name\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *db_filename = argv[1];
    const char *table_name = argv[2];
    const char *column_name = argv[3];

    sqlite3 *db;
    int rc = sqlite3_open(db_filename, &db);
    handle_sqlite_error(rc, "Can't open database");

    Statistics stats = { .count=0, .sum=0, .min_value=NAN, .max_value=NAN, .avg=0, .variance=0 };

    char sql_query_first_pass[1024];
    snprintf(sql_query_first_pass, sizeof(sql_query_first_pass),
              "SELECT %s FROM %s;", column_name, table_name);

    rc = sqlite3_exec(db, sql_query_first_pass, first_pass_callback, &stats, NULL);
    handle_sqlite_error(rc, "SQL error on first pass");

    if(stats.count <= 0) {
        puts("No valid numeric values found in the specified column.");
        sqlite3_close(db);
        return EXIT_SUCCESS;
    }

    stats.avg = stats.sum / stats.count;

    char sql_query_second_pass[1024];
    snprintf(sql_query_second_pass, sizeof(sql_query_second_pass),
              "SELECT %s FROM %s;", column_name, table_name);

    rc = sqlite3_exec(db, sql_query_second_pass, second_pass_callback, &stats, NULL);
    handle_sqlite_error(rc, "SQL error on second pass");

    stats.variance /= stats.count;

    sqlite3_close(db);

    printf("\nСреднее: %.2f\nМаксимальное: %.2f\nМинимальное: %.2f\nСумма: %.2f\nДисперсия: %.2f\n",
           stats.avg, stats.max_value, stats.min_value, stats.sum, stats.variance);

    return EXIT_SUCCESS;
}
