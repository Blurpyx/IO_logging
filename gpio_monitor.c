#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mariadb/mariadb.h>
#include <gpiod/gpiod.h>

// Define GPIO pin numbers
#define PIN1 17
#define PIN2 27

// MariaDB connection details (replace with your credentials)
const char* HOST = "localhost";
const char* USER = "root";
const char* PASSWORD = "pi";
const char* DATABASE = "Embedded_Systems";

// Function prototypes
int connect_to_db(MYSQL** conn, MYSQL_RES** result);
void log_data(MYSQL* conn, int pin, int level, const char* timestamp);
void monitor_gpio(const char* chip_name);

int main() {
  MYSQL* conn = NULL;
  MYSQL_RES* result = NULL;

  // Connect to MariaDB database
  if (connect_to_db(&conn, &result) != 0) {
    return 1;
  }

  // Monitor GPIO pins on specified chip (replace with appropriate chip name)
  monitor_gpio("gpiochip0");

  // Close database connection
  mysql_close(conn);
  mysql_free_result(result);

  return 0;
}

int connect_to_db(MYSQL** conn, MYSQL_RES** result) {
  *conn = mysql_init(NULL);
  if (*conn == NULL) {
    fprintf(stderr, "Error initializing MySQL connection\n");
    return 1;
  }

  if (mysql_real_connect(*conn, HOST, USER, PASSWORD, DATABASE, 0, NULL, 0) == NULL) {
    fprintf(stderr, "Error connecting to database: %s\n", mysql_error(*conn));
    mysql_close(*conn);
    return 1;
  }

  return 0;
}

void log_data(MYSQL* conn, int pin, int value, const char* timestamp) {
  char query[256];
  snprintf(query, sizeof(query), "INSERT INTO gpio_logs (timestamp, pin, value) VALUES ('%s', %d, '%s')", timestamp, pin, value ? "high" : "low");

  if (mysql_query(conn, query) != 0) {
    fprintf(stderr, "Error logging data: %s\n", mysql_error(conn));
  } else {
    printf("Logged data for pin %d: %s at %s\n", pin, value ? "high" : "low", timestamp);
  }
}

void monitor_gpio(const char* chip_name) {
  struct gpiod_chip* chip;
  struct gpiod_line* line1;
  struct gpiod_line* line2;
  int value1, value2;
  char timestamp[32];

  chip = gpiod_chip_open(chip_name);
  if (chip == NULL) {
    perror("gpiod_chip_open");
    return;
  }

  line1 = gpiod_chip_get_line(chip, PIN1);
  line2 = gpiod_chip_get_line(chip, PIN2);
  if (line1 == NULL || line2 == NULL) {
    perror("gpiod_chip_get_line");
    gpiod_chip_close(chip);
    return;
  }

  if (gpiod_line_request_input(line1, "gpio_monitor") != 0 ||
      gpiod_line_request_input(line2, "gpio_monitor") != 0) {
    perror("gpiod_line_request_input");
    gpiod_line_put(line1);
    gpiod_line_put(line2);
    gpiod_chip_close(chip);
    return;
  }

  while (1) {
    value1 = gpiod_line_get_value(line1);
    value2 = gpiod_line_get_value(line2);

    // Get current timestamp
    time_t now = time(NULL);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));

    // Log
    log_data(conn, PIN1, value1, timestamp);
    log_data(conn, PIN2, value2, timestamp);

    // Adjust sleep time based on your desired monitoring frequency
    sleep(1);
  }

  gpiod_line_release(line1);
  gpiod_line_release(line2);
  gpiod_chip_close(chip);
}
