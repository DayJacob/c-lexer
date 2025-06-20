int main() {
  int ret_val = 0;

  int x = 2;
  if (x > 0)
    ret_val = x * 3;

  int y = 5;
  if (y < x)
    ret_val = y / 3;

  return ret_val;
}
