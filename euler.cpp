#include "tinyexpr.h"
#include <algorithm>
#include <cmath>
#include <format>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
/* This is a small cli utility to run eulers method
 *  it uses tinyexpr to parse the function string for y',
 *  then takes step size, initial x and y, final x, precision,
 *  and an optional output flag.
 *  with no flags it will print a pretty printed table to
 *  the terminal. with -l it will output a latex document
 *  in text form to the terminal, and -c will do the same
 *  but in csv format. -cr outputs csv line segments for each
 *  Euler step as (x0,y0,x1,y1). with the flags just cat the output
 *  into an appropriate file type
 *  ex1 pretty printed table: ./euler "0.3*(300 - y)" 0.1 0 350 10 6
 *  ex2 as latex file       : ./euler "0.3*(300 - y)" 0.1 0 350 10 6 -l >
 * table.tex ex3 as csv file         : ./euler "0.3*(300 - y)" 0.1 0 350 10 6 -c
 * > table.csv
 *
 *  tinyexpr does not use implicit multiplication so 0.3x will fail, but 0.3*x
 * will work
 */
struct colwidth {
  int ind;
  int dep;
  int der;
  int dy;
  colwidth() = default;
  colwidth(std::string independent, std::string dependent,
           std::string derivative, std::string deltay)
      : ind(independent.length()), dep(dependent.length()),
        der(derivative.length()), dy(deltay.length()) {}
  colwidth(int ind_, int dep_, int der_, int dy_)
      : ind(ind_), dep(dep_), der(der_), dy(dy_) {}
};

struct stepValues {
  std::string x, y, yp, dy;
  stepValues(double x_, double y_, double yp_, double dy_, int precision) {
    std::ostringstream x__, y__, yp__, dy__;
    x__ << std::fixed << std::setprecision(precision) << ' ' << x_ << ' ';
    y__ << std::fixed << std::setprecision(precision) << ' ' << y_ << ' ';
    yp__ << std::fixed << std::setprecision(precision) << ' ' << yp_ << ' ';
    dy__ << std::fixed << std::setprecision(precision) << ' ' << dy_ << ' ';
    x = x__.str();
    y = y__.str();
    yp = yp__.str();
    dy = dy__.str();
  }
  colwidth getCols() { return colwidth(x, y, yp, dy); }
};

void printLatex(std::vector<stepValues> values, colwidth maxwidths,
                int precision);
void printTable(std::vector<stepValues> values, colwidth maxwidths,
                int precision);
void printCsv(std::vector<stepValues> values, int precision);
void printCsvSegments(std::vector<stepValues> values, int precision);

double roundToPrecision(double value, int precision) {
  double scale = std::pow(10.0, precision);
  return std::round(value * scale) / scale;
}

void eulersMethod(std::string fn, double step, double x0, double y0, double end,
                  int precision, bool latexout, bool csvout,
                  bool csvsegmentsout) {
  double x = roundToPrecision(x0, precision);
  double y = roundToPrecision(y0, precision);
  step = roundToPrecision(step, precision);
  double yp;
  te_parser tep;
  std::vector<stepValues> values;
  tep.set_variables_and_functions({{"x", &x}, {"y", &y}});
  [[maybe_unused]] auto result = tep.evaluate(fn);
  if (!tep.success()) {
    throw std::runtime_error("error parsing expr" + fn);
  }
  while (!(x > end)) {
    yp = roundToPrecision(tep.evaluate(), precision);
    double dy = roundToPrecision(yp * step, precision);
    values.push_back(stepValues(x, y, yp, dy, precision));
    x = roundToPrecision(x + step, precision);
    y = roundToPrecision(y + dy, precision);
  }
  int xm = 0, ym = 0, ypm = 0, dym = 0;
  colwidth cur;
  for (stepValues val : values) {
    cur = val.getCols();
    xm = cur.ind > xm ? cur.ind : xm;
    ym = cur.dep > ym ? cur.dep : ym;
    ypm = cur.der > ypm ? cur.der : ypm;
    dym = cur.dy > dym ? cur.dy : dym;
  }
  colwidth maxwidths = colwidth(xm, ym, ypm, dym);
  if (latexout) {
    printLatex(values, maxwidths, precision);
  } else if (csvsegmentsout) {
    printCsvSegments(values, precision);
  } else if (csvout) {
    printCsv(values, precision);
  } else {
    printTable(values, maxwidths, precision);
  }
}
void printCsv(std::vector<stepValues> values, int precision) {
  std::string xc = "x", yc = "y", ypc = "y'", dyc = "Δy";
  std::ostringstream buffer;
  buffer << std::format("{},{},{},{}\n", xc, yc, ypc, dyc);
  for (stepValues value : values) {
    buffer << std::format("{},{},{},{}\n", value.x, value.y, value.yp,
                          value.dy);
  }
  std::cout << buffer.str();
}

void printCsvSegments(std::vector<stepValues> values, int precision) {
  std::ostringstream buffer;
  buffer << "x0,y0,x1,y1\n";
  for (size_t i = 0; i + 1 < values.size(); i++) {
    double x0 = std::stod(values[i].x), y0 = std::stod(values[i].y),
           x1 = std::stod(values[i + 1].x), y1 = std::stod(values[i + 1].y);
    buffer << std::format("{:.{}f},{:.{}f},{:.{}f},{:.{}f}\n", x0, precision,
                          y0, precision, x1, precision, y1, precision);
  }
  std::cout << buffer.str();
}

void printLatex(std::vector<stepValues> values, colwidth maxwidths,
                int precision) {
  std::ostringstream buffer;
  std::string nc = "n", xc = "x", yc = "y", ypc = "y'", dyc = "$\\Delta$y";
  buffer << "\\documentclass{article}\n\\usepackage[margin=1in]{geometry}"
            "\n\\usepackage{longtable}\n\\begin{document}\n";
  buffer
      << "\\begin{center} \n  \\begin{longtable}{|c|c|c|c|c|}\n    \\hline\n";
  buffer << std::format("{}{} & {} & {} & {} & {} \\\\\n    \\hline\n", "    ",
                        nc, xc, yc, ypc, dyc);
  int n = 0;
  for (stepValues value : values) {
    buffer << std::format("{}{} & {} & {} & {} & {}\\\\\n    \\hline\n", "   ",
                          n, value.x, value.y, value.yp, value.dy);
    n++;
  }
  buffer << "  \\end{longtable} \n\\end{center}\n\\end{document}";
  std::cout << buffer.str();
}

void printTable(std::vector<stepValues> values, colwidth maxwidths,
                int precision) {
  std::string nc = "n ", xc = "x ", yc = "y ", ypc = "y' ", dyc = "Δy ";
  int xm = maxwidths.ind, ym = maxwidths.dep, yp = maxwidths.der,
      dy = maxwidths.dy;
  int nm = std::max(static_cast<int>(std::to_string(values.size()).length()),
                    static_cast<int>(nc.length()));
  std::ostringstream buffer;
  nc = std::format("{:>{}}", nc, nm);
  xc = std::format("{:>{}}", xc, xm), yc = std::format("{:>{}}", yc, ym),
  ypc = std::format("{:>{}}", ypc, yp), dyc = std::format("{:>{}}", dyc, dy);
  buffer << nc << '|' << xc << '|' << yc << '|' << ypc << '|' << dyc
         << std::endl;
  int n = 0;
  for (stepValues value : values) {
    buffer << std::format("{:>{}}", n, nm) << '|'
           << std::format("{:>{}}", value.x, xm) << '|'
           << std::format("{:>{}}", value.y, ym) << '|'
           << std::format("{:>{}}", value.yp, yp) << '|'
           << std::format("{:>{}}", value.dy, dy) << std::endl;
    n++;
  }
  std::cout << buffer.str();
}
double slope(std::string fn, double x, double y) {
  te_parser tep;
  tep.set_variables_and_functions({{"x", &x}, {"y", &y}});
  [[maybe_unused]] auto result = tep.evaluate(fn);
  if (!tep.success()) {
    throw std::runtime_error("error parsing expr" + fn);
  }
  return result;
}
// x/y ranges and spacing for a direction field grid.
void printDField(const std::string &expr, double x0, double y0, double xe,
                 double ye, double xstep, double ystep, int precision,
                 bool plotCurve, double curveStep, double curveX0,
                 double curveY0) {
  if (xstep <= 0 || ystep <= 0) {
    throw std::runtime_error("direction field steps must be positive");
  }
  if (plotCurve && curveStep <= 0) {
    throw std::runtime_error("curve step must be positive");
  }
  if (xe < x0 || ye < y0) {
    throw std::runtime_error("direction field range must be increasing");
  }
  if (ye == y0) {
    throw std::runtime_error("direction field y range must be non-zero");
  }

  std::ostringstream buffer;
  constexpr double segLen = 2.0;
  const double xrange = xe - x0;
  const double yrange = ye - y0;
  const double yScale = xrange / yrange;
  const double yTop = y0 + xrange;
  const double ySampleStep = ystep / yScale;
  auto mapY = [&](double y) { return y0 + (y - y0) * yScale; };

  buffer << "\\begin{center}\n"
            "\\resizebox{\\linewidth}{!}{%\n";
  buffer << std::format("\\begin{{tikzpicture}}[scale=0.12]\n"
                        "  \\draw[->] ({:.{}f},{:.{}f}) -- ({:.{}f},{:.{}f}) "
                        "node[right] {{$t$}};\n"
                        "  \\draw[->] ({:.{}f},{:.{}f}) -- ({:.{}f},{:.{}f}) "
                        "node[above] {{$y$}};\n",
                        x0, precision, y0, precision, xe, precision, y0,
                        precision, x0, precision, y0, precision, x0, precision,
                        yTop, precision);

  for (double x = x0; x <= xe + 1e-12; x += xstep) {
    for (double y = y0; y <= ye + 1e-12; y += ySampleStep) {
      double m = slope(expr, x, y);
      // Slopes are adjusted so the displayed field remains correct under
      // the custom y-axis scaling.
      double mScaled = m * yScale;
      double dx = segLen / std::sqrt(1.0 + mScaled * mScaled);
      double dy = mScaled * dx;
      double yCenter = mapY(y);
      double xL = x - dx / 2.0, yL = yCenter - dy / 2.0;
      double xR = x + dx / 2.0, yR = yCenter + dy / 2.0;
      buffer << std::format(
          "  \\draw[blue!70] ({:.{}f},{:.{}f}) -- ({:.{}f},{:.{}f});\n", xL,
          precision, yL, precision, xR, precision, yR, precision);
    }
  }

  if (plotCurve) {
    double x = roundToPrecision(curveX0, precision);
    double y = roundToPrecision(curveY0, precision);
    double step = roundToPrecision(curveStep, precision);
    buffer << "  \\draw[red, thick] ";
    bool first = true;
    for (; x <= xe + 1e-12; x = roundToPrecision(x + step, precision)) {
      if (y < y0 - 1e-12 || y > ye + 1e-12) {
        break;
      }
      if (first) {
        buffer << "plot coordinates {";
        first = false;
      }
      buffer << std::format(" ({:.{}f},{:.{}f})", x, precision, mapY(y),
                            precision);
      double m = roundToPrecision(slope(expr, x, y), precision);
      double dy = roundToPrecision(m * step, precision);
      y = roundToPrecision(y + dy, precision);
    }
    if (!first) {
      buffer << " };\n";
    }
  }

  buffer << "\\end{tikzpicture}%\n"
            "}\n"
            "\\end{center}\n";
  std::cout << buffer.str();
}

int main(int argc, char *argv[]) {
  bool latexout = false;
  bool csvout = false;
  bool csvsegmentsout = false;
  bool dfieldout = false;
  bool dfieldcurveout = false;
  if (argc == 8) {
    if (std::string("-l") == std::string(argv[7])) {
      latexout = true;
    } else if (std::string("-c") == std::string(argv[7])) {
      csvout = true;
    } else if (std::string("-cr") == std::string(argv[7])) {
      csvsegmentsout = true;
    } else {
      throw std::runtime_error(
          "unknown output flag; use -l, -c, -cr, -df, or -dfc");
    }
  } else if (argc == 10 && std::string("-df") == std::string(argv[9])) {
    dfieldout = true;
  } else if ((argc == 11 || argc == 13) &&
             std::string("-dfc") == std::string(argv[argc - 1])) {
    dfieldcurveout = true;
  }
  if (!(dfieldout || dfieldcurveout) && (argc < 7 || argc > 8)) {
    throw std::runtime_error("too many or too few arguments to eulers method");
  }
  if (dfieldout || dfieldcurveout) {
    // -df / -dfc mode:
    // ./euler "expr" x0 y0 xe ye xstep ystep precision -df
    // ./euler "expr" x0 y0 xe ye xgrid ygrid h precision -dfc
    // ./euler "expr" x0 y0 xe ye xgrid ygrid h x_init y_init precision -dfc
    std::string fn = argv[1];
    double x0 = std::stod(std::string(argv[2])),
           y0 = std::stod(std::string(argv[3])),
           xe = std::stod(std::string(argv[4])),
           ye = std::stod(std::string(argv[5])),
           xgrid = std::stod(std::string(argv[6])),
           ygrid = std::stod(std::string(argv[7]));
    double curveX0 = x0, curveY0 = y0;
    double curveStep = xgrid;
    int prec = 0;
    if (dfieldcurveout && argc == 13) {
      curveStep = std::stod(std::string(argv[8]));
      curveX0 = std::stod(std::string(argv[9]));
      curveY0 = std::stod(std::string(argv[10]));
      prec = std::stoi(std::string(argv[11]));
    } else if (dfieldcurveout && argc == 11) {
      curveStep = std::stod(std::string(argv[8]));
      prec = std::stoi(std::string(argv[9]));
    } else {
      prec = std::stoi(std::string(argv[8]));
    }
    printDField(fn, x0, y0, xe, ye, xgrid, ygrid, prec, dfieldcurveout,
                curveStep, curveX0, curveY0);
  } else {
    std::string fn = argv[1];
    double step = std::stod(std::string(argv[2])),
           x = std::stod(std::string(argv[3])),
           y = std::stod(std::string(argv[4])),
           end = std::stod(std::string(argv[5]));
    int prec = std::stoi(std::string(argv[6]));
    eulersMethod(fn, step, x, y, end, prec, latexout, csvout, csvsegmentsout);
  }
}

// euler "y^2 = yx^2 -2" 0.1 0 1 3
