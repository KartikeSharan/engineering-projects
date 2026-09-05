# Quine–McCluskey Boolean Minimiser (Python)

A complete implementation of the **Quine–McCluskey algorithm** with
**don't-care support**, minimising an arbitrary *n*-variable Boolean function
entered as a truth table and returning a simplified sum-of-products expression.

Written as the final assignment for ENG2083 *Introductory Programming 2*
(University of Glasgow). Full write-up:
[Quine\_McCluskey\_Report.pdf](docs/Quine_McCluskey_Report.pdf).

## How it works

1. **Truth table construction** — prompts for the number of variables *n*,
   builds all 2ⁿ input combinations and collects each output as `0`, `1` or
   `X`, with input validation and warnings once the table gets large.
2. **Implicant representation** — each implicant is a triple
   `(bits, mask, covers)`: an integer bit pattern, a bitmask marking
   don't-care positions, and the set of minterms it covers. Tracking coverage
   explicitly is what makes essential-prime-implicant detection reliable later.
3. **Grouping and combination** — implicants are grouped by population count
   and only *adjacent* groups are compared, since terms that differ by one
   variable must differ by one in their number of ones. This prunes the
   comparison space without affecting correctness. Pairs differing in exactly
   one unmasked bit are merged, and the differing position becomes a new
   don't-care; duplicates are merged by `(bits, mask)` key.
4. **Prime implicants** — any term surviving a round uncombined is a prime
   implicant. The loop repeats until no further combination is possible.
5. **Cover selection** — minterms covered by exactly one prime implicant give
   the **essential** prime implicants; the remainder are covered greedily,
   repeatedly taking the implicant covering the most still-uncovered minterms.
6. **Expression generation** — selected implicants are rendered back to
   literals (`A`, `A'`, omitted where masked) and joined into an SOP
   expression.

## Usage

```bash
pip install pandas
python quine_mccluskey.py
```

Enter the number of variables, then the output (`0`, `1` or `X`) for each row;
the truth table is re-displayed after every entry.

## Verification

Tested against a four-variable function with don't-cares, producing

```
F = C'B + DC' + DB
```

which was independently confirmed by minimising the same function by hand with
a Karnaugh map — the two are algebraically equivalent.

## Known limitations

* The **greedy cover is not guaranteed minimal** for ≥ 5 variables; Petrick's
  method would be the correct replacement for a guaranteed-minimal solution.
* Quine–McCluskey is exponential in *n*, so runtime and memory grow rapidly on
  large functions.
* Guided row-by-row entry was chosen over a compact input format to reduce user
  error, at the cost of convenience on large tables.
