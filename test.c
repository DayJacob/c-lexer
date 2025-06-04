int add(int x, int y) { return x + y; }

int main() {
  int x = 2;
  int y = x + 5;

  return add(y - x, y);
}
