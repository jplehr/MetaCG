// Tests for c++ style constructor init
int foo();
int boo();
using FType = decltype(foo);

class B {
 public:
  B(FType* arg) { f1 = arg; };
  FType* f1;
};

class C : B {
 public:
  C(FType* arg1, FType* arg2) : B(arg1), f2(arg2) {}
  FType* f2;
  void work() {
    f1();
    f2();
  }
};

int main() {
  C c(foo, boo);
  c.work();
}