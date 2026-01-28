#include "tinyexpr.h"
#include <format>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
/* This is a small cli utility to run eulers method
 *  it uses tinyexpr to parse the function string for y'
 *  the step size, initial x and y, and the final x,
 *  the precision and finally two optional flags.
 *  with no flags it will print a pretty printed table to
 *  the terminal. with -l it will output a latex document
 *  in text form to the terminal, and -c will do the same
 *  but in csv format. with the flags just cat the output
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

void eulersMethod(std::string fn, double step, double x0, double y0, double end,
                  int precision, bool latexout, bool csvout) {
  double x = x0, y = y0, yp;
  te_parser tep;
  std::vector<stepValues> values;
  tep.set_variables_and_functions({{"x", &x}, {"y", &y}});
  auto result = tep.evaluate(fn);
  if (!tep.success()) {
    throw std::runtime_error("error parsing expr" + fn);
  }
  while (!(x > end)) {
    yp = tep.evaluate();
    values.push_back(stepValues(x, y, yp, yp * step, precision));
    x += step;
    y += yp * step;
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
  } else if (csvout) {
    printCsv(values, precision);
  } else if (csvout) {
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

void printLatex(std::vector<stepValues> values, colwidth maxwidths,
                int precision) {
  std::ostringstream buffer;
  std::string xc = "x", yc = "y", ypc = "y'", dyc = "$\\Delta$y";
  buffer << "\\documentclass{article}\n\\usepackage[margin=1in]{geometry}"
            "\n\\usepackage{longtable}\n\\begin{document}\n";
  buffer << "\\begin{center} \n  \\begin{longtable}{|c|c|c|c|}\n    \\hline\n";
  buffer << std::format("{}{} & {} & {} & {} \\\\\n    \\hline\n", "    ", xc,
                        yc, ypc, dyc);
  for (stepValues value : values) {
    buffer << std::format("{}{} & {} & {} & {}\\\\\n    \\hline\n", "   ",
                          value.x, value.y, value.yp, value.dy);
  }
  buffer << "  \\end{longtable} \n\\end{center}\n\\end{document}";
  std::cout << buffer.str();
}

void printTable(std::vector<stepValues> values, colwidth maxwidths,
                int precision) {
  std::string xc = "x ", yc = "y ", ypc = "y' ", dyc = "Δy ";
  int xm = maxwidths.ind, ym = maxwidths.dep, yp = maxwidths.der,
      dy = maxwidths.dy;
  std::ostringstream buffer;
  xc = std::format("{:>{}}", xc, xm), yc = std::format("{:>{}}", yc, ym),
  ypc = std::format("{:>{}}", ypc, yp), dyc = std::format("{:>{}}", dyc, dy);
  buffer << xc << '|' << yc << '|' << ypc << '|' << dyc << std::endl;
  for (stepValues value : values) {
    buffer << std::format("{:>{}}", value.x, xm) << '|'
           << std::format("{:>{}}", value.y, ym) << '|'
           << std::format("{:>{}}", value.yp, yp) << '|'
           << std::format("{:>{}}", value.dy, dy) << std::endl;
  }
  std::cout << buffer.str();
}

int main(int argc, char *argv[]) {
  bool latexout = false;
  bool csvout = false;
  if (argc == 8) {
    if (std::string("-l") == std::string(argv[7])) {
      latexout = true;
    } else if (std::string("-c") == std::string(argv[7])) {
      csvout = true;
    }
  }
  if (argc < 7 || argc > 8) {
    throw std::runtime_error("too many or too few arguments to eulers method");
  }
  std::string fn = argv[1];
  double step = std::stod(std::string(argv[2])),
         x = std::stod(std::string(argv[3])),
         y = std::stod(std::string(argv[4])),
         end = std::stod(std::string(argv[5]));
  int prec = std::stoi(std::string(argv[6]));
  eulersMethod(fn, step, x, y, end, prec, latexout, csvout);
}

// euler "y^2 = yx^2 -2" 0.1 0 1 3
