import random, sys
n, m, R, seed = map(int, sys.argv[1:5])
random.seed(seed)
print(n, m)
for _ in range(n):
    l = random.randint(-R, R - 1)
    r = random.randint(l + 1, R)
    print(l, r, random.randint(-R, R))
for _ in range(m):
    print(random.randint(-R, R), random.randint(-R, R))
