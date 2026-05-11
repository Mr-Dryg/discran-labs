#!/usr/bin/env python3
import argparse
import random
import string


def gen_key(rng: random.Random, length: int) -> str:
    alphabet = string.ascii_lowercase
    return "".join(rng.choice(alphabet) for _ in range(length))


def main() -> None:
    p = argparse.ArgumentParser(description="Generate benchmark input for RBTree program")
    p.add_argument("--seed", type=int, default=1)
    p.add_argument("--n_insert", type=int, default=200000)
    p.add_argument("--n_find", type=int, default=400000)
    p.add_argument("--n_remove", type=int, default=100000)
    p.add_argument("--key_len", type=int, default=16)
    args = p.parse_args()

    rng = random.Random(args.seed)

    keys: list[str] = []
    used: set[str] = set()

    # Inserts (unique keys).
    while len(keys) < args.n_insert:
        k = gen_key(rng, args.key_len)
        if k in used:
            continue
        used.add(k)
        keys.append(k)
        v = rng.getrandbits(64)
        print(f"+ {k} {v}")

    # Finds (mix of hits and misses).
    for _ in range(args.n_find):
        if rng.random() < 0.8:
            k = keys[rng.randrange(len(keys))]
        else:
            k = gen_key(rng, args.key_len)
        print(k)

    # Removes (mostly existing keys).
    remove_keys = rng.sample(keys, k=min(args.n_remove, len(keys)))
    for k in remove_keys:
        print(f"- {k}")


if __name__ == "__main__":
    main()

