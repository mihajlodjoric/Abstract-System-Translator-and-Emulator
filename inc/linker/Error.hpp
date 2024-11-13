#include <iostream>
using namespace std;

class MultipleDefinitions : public exception{
private:
  string msg;
public:
  MultipleDefinitions(const string& symbol) : msg("Symbol defined multiple times: " + symbol){}
  const char* what() const throw() override {
    return msg.c_str();
  }
};

class NotDefined : public exception{
private:
  string msg;
public:
  NotDefined(const string& symbol) : msg("Symbol not defined: " + symbol){}
  const char* what() const throw() override {
    return msg.c_str();
  }
};

class SectionOverlapping : public exception{
private:
  string msg;
public:
  SectionOverlapping(const string& section) : msg("Section overlapping: " + section){}
  const char* what() const throw() override {
    return msg.c_str();
  }
};