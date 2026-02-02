# Euler Method CLI

Small CLI utility to run Euler's method. It uses tinyexpr to parse the function string for y'.

## Build

```sh
g++ -std=c++20 tinyexpr.cpp euler.cpp -o euler
```

## Usage

The program has two modes: Euler table mode and direction-field mode.

### 1) Euler Table Mode

```sh
./euler "f(x,y)" step x0 y0 x_end precision [flag]
```

Arguments (in order):
- function for `y'`
- step size
- initial `x`
- initial `y`
- final `x`
- precision
- optional flag

Flags:
- `-l` : LaTeX table output
- `-c` : CSV table output (`x,y,y',Δy`)
- `-cr`: CSV Euler segments (`x0,y0,x1,y1`)

Default output (no flag) is a terminal table with columns: `n, x, y, y', Δy`.

### 2) Direction Field Mode

```sh
./euler "f(x,y)" x_min y_min x_max y_max x_grid y_grid precision -df
```

This outputs TikZ code for a direction field scaled to fit the plot window.

To overlay an Euler curve on the same field:

```sh
./euler "f(x,y)" x_min y_min x_max y_max x_grid y_grid h precision -dfc
./euler "f(x,y)" x_min y_min x_max y_max x_grid y_grid h x_init y_init precision -dfc
```

Where:
- `h` is Euler step size for the red approximation curve
- `x_init,y_init` are optional curve initial conditions (defaults to `x_min,y_min`)

Examples:

```sh
# Pretty printed table
./euler "0.3*(300 - y)" 0.1 0 350 10 6

# LaTeX output
./euler "0.3*(300 - y)" 0.1 0 350 10 6 -l > table.tex

# CSV output
./euler "0.3*(300 - y)" 0.1 0 350 10 6 -c > table.csv

# CSV line segments for Euler steps
./euler "0.3*(300 - y)" 0.1 0 350 10 6 -cr > segments.csv

# Direction field (TikZ)
./euler "-y/(500+x)" 0 60 210 95 5 5 5 -df > field.tex

# Direction field + Euler curve (TikZ)
./euler "-y/(500+x)" 0 60 210 95 5 5 3 0 90 5 -dfc > field_curve.tex
```

Notes:
- tinyexpr does not use implicit multiplication, so `0.3x` will fail; use `0.3*x`.
- Precision is used in simulation updates (rounded each step), not just display.

## Tinyexpr Syntax Notes

- Only `x` and `y` are available as variables.
- Variable and function names must start with a letter, then use letters, digits, `.` or `_`.
- Function arguments are comma-separated: `max(a,b)`, `pow(x,2)`.
- Common built-ins include `sin`, `cos`, `tan`, `log`, `exp`, `sqrt`, `abs`, plus constants like `pi` and `e`.
