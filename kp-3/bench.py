import subprocess, time, sys, statistics

def run(exe, path, reps):
    ts = []
    for _ in range(reps):
        t = time.perf_counter()
        subprocess.run(exe, stdin=open(path), stdout=subprocess.DEVNULL, check=True)
        ts.append(time.perf_counter() - t)
    return statistics.median(ts)

sizes = [int(a) for a in sys.argv[1:]] or [1000, 2000, 5000, 10000, 20000, 50000, 100000]
print(f"{'n=m':>8} {'sol, c':>10} {'brute, c':>10}")
for n in sizes:
    path = f"bench_{n}.txt"
    with open(path, "w") as f:
        subprocess.run(["python3", "gen.py", str(n), str(n), "1000000000", "1"], stdout=f, check=True)
    ts = run("./sol", path, 3)
    tb = run("./brute", path, 1 if n >= 50000 else 3)
    print(f"{n:>8} {ts:>10.4f} {tb:>10.4f}", flush=True)