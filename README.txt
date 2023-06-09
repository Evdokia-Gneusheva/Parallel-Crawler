# Web Crawler Program

This program allows you to crawl a specific number of web links using a specified number of threads. It takes six parameters: `only_local`, `max_links_per_thread`, `max_level`, `disregard_leftovers`, `num_threads`, and `num_links`.

## Requirements

- C++11 or later
- Curl library

For Windows users, you may need to change the include paths in the makefile to match your system setup. Specifically, check the line `CFLAGS = -std=c++11 -Wall -g -I "C:/curl/curl-8.1.2_2-win64-mingw/include"` and adjust as needed.

## How to Compile

To compile the program, simply run the `make` command in the terminal. The provided makefile will handle the rest.


## How to Run

After compiling the program, you can run it using the following syntax:

./main [only_local] [max_links_per_thread] [max_level] [disregard_leftovers] [num_threads] [num_links]


Each of the parameters should be an integer value, with boolean values entered as `1` for `true` and `0` for `false`.

For example:

./main 1 50 3 0 2 1000


This will run the program with `only_local` set to `true`, `max_links_per_thread` set to `50`, `max_level` set to `3`, `disregard_leftovers` set to `false`, `num_threads` set to `2`, and `num_links` set to `1000`.

If no arguments are provided, the program will use default values.

## Output

The program will output the elapsed time, the number of links visited, the total number of links tried to visit, and the number of links per thread. This information will be printed to the console.