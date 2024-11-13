#include <iostream>
using namespace std;

class DivisionByZero : public exception{
private:
  string msg;
public:
  DivisionByZero() : msg("Error: Divison by zero"){}
  const char* what() const throw() override {
    return msg.c_str();
  }
};

class InvalidCSRRegister : public exception{
private:
  string msg;
public:
  InvalidCSRRegister(int i) : msg("Error: Invalid register csr[" + i + ']'){}
  const char* what() const throw() override {
    return msg.c_str();
  }
};

class InvalidGPRRegister : public exception{
private:
  string msg;
public:
  InvalidGPRRegister(int i) : msg("Error: Invalid register gpr[" + i + ']'){}
  const char* what() const throw() override {
    return msg.c_str();
  }
};

class StackOverflow : public exception{
private:
  string msg;
public:
  StackOverflow() : msg("Error: Stack overflow"){}
  const char* what() const throw() override {
    return msg.c_str();
  }
};

class InvalidCode : public exception{
private:
  string msg;
public:
  InvalidCode() : msg("Error: Invalid instruction codeeeeee"){}
  const char* what() const throw() override {
    return msg.c_str();
  }
};