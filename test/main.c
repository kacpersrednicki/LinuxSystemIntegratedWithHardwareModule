#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 6

#define A_DIRECTORY "/proc/proj4srekac/rejA"
#define S_DIRECTORY "/proc/proj4srekac/rejS"
#define W_DIRECTORY "/proc/proj4srekac/rejW"

unsigned int read_file(char *filePath) {
        int fd_in, to_read, value_ready;
        char buf[BUF_SIZE];
        fd_in = open(filePath, O_RDONLY);

        if (fd_in < 0) {
                printf("Open %s – error: %d\n", filePath, errno);
                exit(1);
        }

        to_read = read(fd_in, buf, BUF_SIZE);

        if (to_read < 0) {
                fprintf(stderr, "Error\n", filePath, errno);
                close(fd_in);
                return 0;
        }

        buf[to_read] = '\0';
        sscanf(buf, "%o", &value_ready);

        close(fd_in);
        return (unsigned int)value_ready;
}


int write_file(char *filePath, unsigned int value){
        int fd_out, to_write, length;
        char buf[BUF_SIZE];

        fd_out = open(filePath, O_RDWR);

        if(fd_out < 0) {
                printf("Open %s – error: %d\n", filePath, errno);
                exit(2);
        }

        snprintf(buf, BUF_SIZE, "%o", value);
	length = strlen(buf);
        lseek(fd_out, 0L, SEEK_SET);
        to_write = write(fd_out, buf, length);

        if(to_write != length) {
                fprintf(stderr, "Error, wrong nuber if bytes.\n", length, to_write);
                close(fd_out);
                return -1;
        }

        close(fd_out);
        return 0;
}

int test_id = 0;
int successful_tests = 0;
int test_values[3] = {017, 034, 071};
int theo_results[3] = {057, 0153, 0415};

void wait(int iters){
        for(int i = 1; i <= iters; i++){}
}

void wait_until_ready() {
        unsigned int a, s;
        a = read_file(A_DIRECTORY);
        do {
                s = read_file(S_DIRECTORY);
                sleep(2); // co sekunde
        } while (s < a);
}

void passed_or_failed(int val, int res) {
        if(val == res) {
            printf("Test PASSED!\n");
            successful_tests++;
        } else {
            printf("Test FAILED!\n");
        }
}

void test_write_read_A() {
        test_id++;
        int val = 07;
        printf("\n-----------------------TEST%d-----------------------\n", test_id);
        printf("Test: Writing and reading rejA.\n");
        printf("Writing value %#o to rejA...\n", val);
        write_file(A_DIRECTORY, val);
        wait(100000);
        printf("Reading from rejA...\n");
        int res = read_file(A_DIRECTORY);
        wait(100000);
        printf("Value: %#o\n", res);
        passed_or_failed(val, res);
}

void test_calculation(int val, int theo_res) {
        test_id++;
        printf("\n-----------------------TEST%d-----------------------\n", test_id);
        printf("Test: Searching for %#oth prime number.\n", val);
        printf("Writing value %#o to rejA...\n", val);
        write_file(A_DIRECTORY, val);
        wait(100000);
        printf("Searching...\n");
        wait_until_ready();
        printf("Ready, reading result from rejW...\n");
        int res = read_file(W_DIRECTORY);
        wait(100000);
        printf("Value: %#o (should be %#o).\n", res, theo_res);
        passed_or_failed(res, theo_res);
}

void test_status(int val) {
        test_id++;
        printf("\n-----------------------TEST%d-----------------------\n", test_id);
        printf("Test: Verifying the status.\n");
        printf("Writing value %#o to rejA...\n", val);
        write_file(A_DIRECTORY, val);
        wait(100000);
        printf("Searching and checking status at rejS...\n");
        unsigned int a, s;
        a = read_file(A_DIRECTORY);
        do {
                s = read_file(S_DIRECTORY);
                printf("Current status: %#oth prime number.\n", s);
                sleep(2); // co 2 sekundy
        } while (s < a);
        printf("Ready, status (S) is %#oth prime number, same as argument (A).\n", s);
        passed_or_failed(a, s);
}

void test_errors() {
        printf("\n--------------------TEST_ERRORS--------------------\n");

        printf("Writing value %#o to rejS...\n", 04);
        write_file(S_DIRECTORY, 04);
        wait(100000);

        printf("\nWriting value %#o to rejW...\n", 04);
        write_file(W_DIRECTORY, 04);
        wait(100000);

        printf("\nWriting value %d to rejA...\n", -1);
        write_file(A_DIRECTORY, -1);
        wait(100000);

        printf("\nWriting value %#o to rejA...\n", 00);
        write_file(A_DIRECTORY, 00);
        wait(100000);

        printf("\nWriting value %#o to rejA...\n", 01751);
        write_file(A_DIRECTORY, 01751);
        wait(100000);

        printf("Error tests FINISHED!\n");
}

int main(void) {
        printf("Starting tests...\n");
        printf("All values OCT!\n");

        test_write_read_A();
        test_calculation(test_values[0], theo_results[0]);
        test_calculation(test_values[1], theo_results[1]);
        test_calculation(test_values[2], theo_results[2]);
        test_status(test_values[1]);

        printf("\n----------------------SUMMARY----------------------\n");
        printf("All tests completed: %d/%d successful.\n", successful_tests, test_id);
        printf("Also bonus error tests in progress...\n");

        test_errors();

        return 0;
}

