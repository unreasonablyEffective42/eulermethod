# Euler Method CLI

Small CLI utility to run Euler's method. It uses tinyexpr to parse the function string for y'.

## Build

```sh
g++ -std=c++20 tinyexpr.cpp euler.cpp -o euler
```

## Usage

Arguments (in order):
- function for y'
- step size
- initial x
- initial y
- final x
- precision
- optional flag: `-l` for LaTeX output, `-c` for CSV output

Examples:

```sh
# Pretty printed table
./euler "0.3*(300 - y)" 0.1 0 350 10 6

# LaTeX output
./euler "0.3*(300 - y)" 0.1 0 350 10 6 -l > table.tex

# CSV output
./euler "0.3*(300 - y)" 0.1 0 350 10 6 -c > table.csv
```

Notes:
- tinyexpr does not use implicit multiplication, so `0.3x` will fail; use `0.3*x`.

## Tinyexpr Syntax Notes

- Only `x` and `y` are available as variables.
- Variable and function names must start with a letter, then use letters, digits, `.` or `_`.
- Function arguments are comma-separated: `max(a,b)`, `pow(x,2)`.
- Common built-ins include `sin`, `cos`, `tan`, `log`, `exp`, `sqrt`, `abs`, plus constants like `pi` and `e`.
