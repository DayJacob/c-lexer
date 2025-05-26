int main() {
  const float PI = 3.14;

  int *arr = malloc(sizeof(int) * 5);
  arr[2] = PI / 3;

  if (arr[1] <= 5) {
    char less = 80;
  } else if (~arr[3] > (int)PI) {
    short clang = (long)&PI;
  }

  char *str = "string";

  return 0;
}
