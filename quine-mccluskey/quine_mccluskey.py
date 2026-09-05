"""
Quine–McCluskey Boolean Minimisation (with don't-cares)
Author: Kartike Sharan (3008620s)

Description:
- Prompts the user for number of variables n
- Builds a full truth table (2^n rows) and collects outputs (0/1/X)
- Runs Quine–McCluskey combining to generate prime implicants
- Selects essential prime implicants, then covers remaining minterms greedily
- Prints the simplified sum-of-products (SOP) expression
"""

# ------------------ Imports ------------------
import pandas as pd
import string

# ------------------ Variables / Data Stores ------------------
Num_Var = 0

# Truth table storage: each row is [bits..., OUT]
Input_Array = []

# Term representation used throughout:
# term = (bits, mask, covers)
# - bits: integer bit pattern
# - mask: integer bitmask where 1 = "don't-care position"
# - covers: set of minterm indices represented by this term
Minterms = []
DontCares = []
Initial_Terms = []

groups = {}             # groups[k] = list of terms with k ones in their bit pattern
used_terms = set()      # (bits, mask) pairs that were combined in the current round
next_terms = {}         # (bits, mask) -> covers set (merges duplicates)
prime_implicants = []   # list of terms that can no longer be combined

pi_covers = {}          # minterm index -> list of prime implicants that cover it
key_prime_implicants = []   # selected implicants (essential + greedy picks)

Minterm_Index = set()
covered_minterms = set()
uncovered_minterms = set()

expression_terms = []

# ------------------ Input Section ------------------
# Ask for number of variables and validate input.
while True:
    print()
    Num_Var_String = input("Enter number of variables: ")
    print()

    if Num_Var_String.isdigit():
        Num_Var = int(Num_Var_String)

        if Num_Var <= 0:
            print("Number of variables must be greater than 0.")
            continue

        # For n <= 4, greedy cover is typically fine and results are usually minimal.
        if Num_Var <= 4:
            break

        # For larger n, algorithm still works, but greedy cover may be non-minimal.
        if 4 < Num_Var <= 6:
            print("Note: Greedy PI selection may not be optimal for more than 4 variables.")
            input("Press Enter to continue...")
            break

        if 6 < Num_Var <= 10:
            print("Warning: This is a large truth table to enter manually.")
            print("Note: Greedy PI selection may not be optimal for more than 4 variables.")
            input("Press Enter to continue...")
            break

        if Num_Var >= 10:
            print("Warning: Runtime/memory can grow rapidly for large n (exponential scaling).")
            print("Note: Greedy PI selection may not be optimal for more than 4 variables.")
            input("Press Enter to continue...")
            break

    else:
        print("Please enter a valid integer.")

print("Number of Variables =", Num_Var)
print()

# ------------------ Helper Functions ------------------
def ones_count(x):
    """Count number of 1s in binary representation of x."""
    return bin(x).count("1")


def one_bit(x):
    """True if x has exactly one bit set (i.e., x is a power of two)."""
    if bin(x).count("1") == 1: 
        return True
    else:
        return False


def can_terms_be_combined(term_1, term_2):
    """
    Two terms can be combined if:
    - They have the same mask (same don't-care positions), and
    - They differ in exactly ONE unmasked bit position.
    """
    bits_1, mask_1, covered_1 = term_1
    bits_2, mask_2, covered_2 = term_2

    # Must be in the same "don't-care pattern" class.
    if mask_1 != mask_2:
        return False

    # XOR identifies bit positions that differ; ignore masked positions.
    diff = (bits_1 ^ bits_2) 

    # Turns all don't care positions to 0 using AND with inverse of mask.
    diff = diff & ~mask_1


    # Combine only if exactly one unmasked bit differs.
    return one_bit(diff)


def combine_terms(t1, t2):
    """
    Combine two compatible terms into a new term by turning the differing bit into a don't-care.
    """
    bits_1, mask_1, covered_1 = t1
    bits_2, mask_2, covered_2 = t2

    # XOR identifies bit positions that differ; ignore masked positions.
    diff = (bits_1 ^ bits_2) 

    # Turns all don't care positions to 0 using AND with inverse of mask.
    diff = diff & ~mask_1

    new_bits = bits_1 & ~diff           # force differing position to 0 (value becomes irrelevant due to mask)
    new_mask = mask_1 | diff            # mark differing position as don't-care
    new_covered = covered_1.union(covered_2)

    return (new_bits, new_mask, new_covered)


def implicant_to_expr(bits: int, mask: int, var_names) -> str:
    """
    Convert (bits, mask) to a SOP literal string.

    var_names is ordered left-to-right as displayed in the truth table (e.g., ['D','C','B','A']).
    For each variable:
    - if masked -> omit it
    - if bit=1 -> include X
    - if bit=0 -> include X'
    """
    n = len(var_names)
    parts = []

    # i=0 corresponds to MSB variable in var_names
    for i in range(n):
        bitpos = n - 1 - i  # numeric bit position (LSB is position 0)

        if ((mask >> bitpos) & 1) == 1: 
            continue  # variable is don't-care

        if ((bits >> bitpos) & 1) == 1:
            parts.append(var_names[i])
        elif (bits >> bitpos) & 1 == 0:                           
            parts.append(var_names[i] + "'") 

    if len(parts) == 0:
        return '1'  
    else:
        return ''.join(parts)

# ------------------ Truth Table Construction ------------------
Num_Rows = 2 ** Num_Var

if Num_Var > 4:
    print("\n************** WARNING **************")
    print("Large truth tables become slow/tedious to enter manually.")
    print("************** WARNING **************\n")

# Variable naming:
# For n <= 26, use letters. Reverse to show MSB on the left (e.g., D C B A).
if Num_Var < 26:
    Col_Name = list(string.ascii_uppercase[:Num_Var][::-1]) + ["OUT"]
else:
    # Fallback naming for >26 variables (rare/unrealistic for this lab).
    Col_Name = (
        list(string.ascii_uppercase[:Num_Var][::-1])
        + [f"Var{27+i}" for i in range(Num_Var)][::-1]
        + ["OUT"]
    )

# Build all input combinations, OUT initially unknown ('_').
for i in range(Num_Rows):
    binary_string = format(i, f"0{Num_Var}b")
    row = []                    
    for bit in binary_string:    
        row.append(int(bit))     
    row.append('_')          
    Input_Array.append(row)

df = pd.DataFrame(Input_Array, columns=Col_Name)
print(df)

# Names used for final expression generation (exclude output column).
var_names = Col_Name[:-1]

# Collect user outputs row-by-row with validation.
for i in range(Num_Rows):
    while True:
        print("\n")
        Q = input(f"Enter output for row {i} | [0, 1 or X]: ")

        if Q in ("x", "X"):
            Input_Array[i][Num_Var] = "X"
            break
        elif Q in ("0", "1"):
            Input_Array[i][Num_Var] = int(Q)
            break
        else:
            print("Invalid input. Please enter 0, 1, or X.")

    df = pd.DataFrame(Input_Array, columns=Col_Name)
    print("\n", df)

# ------------------ Extract Minterms and Don't-Cares ------------------
# Convert truth table outputs into term lists.
for i in range(len(Input_Array)):
    Q = Input_Array[i][Num_Var]

    if Q == 1:
        Minterms.append((i, 0, {i}))
    elif Q == "X":
        DontCares.append((i, 0, {i}))

Initial_Terms = Minterms + DontCares

# ------------------ Quine–McCluskey Combining ------------------
current_terms = Initial_Terms

while True:
    groups = {}       # reset groups each round
    next_terms = {}   # reset next round terms
    used_terms = set()  # reset used terms each round

    # Group by number of ones in 'bits' (mask ignored for grouping).
    for term in current_terms:
        bits, mask, covers = term
        ones = ones_count(bits)
        groups.setdefault(ones, []).append(term)

    # Sort groups by ones-count so we can compare adjacent groups.
    groups = dict(sorted(groups.items()))

    # Compare adjacent groups only: k with k+1
    for ones in groups.keys():
        if (ones + 1) not in groups:
            continue

        for a in groups[ones]:
            for b in groups[ones + 1]:
                if can_terms_be_combined(a, b):
                    used_terms.add((a[0], a[1]))
                    used_terms.add((b[0], b[1]))

                    c = combine_terms(a, b)
                    new_bits, new_mask, new_covers = c
                    key = (new_bits, new_mask)

                    # Merge duplicates by key to avoid repeated terms.
                    if key not in next_terms:
                        next_terms[key] = set(new_covers)
                    else:
                        next_terms[key].update(new_covers)

    # Any term not combined in this round becomes a prime implicant.
    for term in current_terms:
        if (term[0], term[1]) not in used_terms:
            prime_implicants.append(term)

    # Stop if no new combinations were produced.
    if len(next_terms) == 0:
        break

    # Build next round list from merged dictionary.
    current_terms = []
    for key, covers in next_terms.items():   
        bits = key[0]                        
        mask = key[1]                        
        current_terms.append((bits, mask, covers)) 


# ------------------ Select Essential + Greedy Cover Implicants ------------------
# Map each minterm to the prime implicants that cover it.
for term in Minterms:
    m = term[0]
    pi_covers[m] = []
    Minterm_Index.add(m)

    for pi in prime_implicants:
        if m in pi[2]:
            pi_covers[m].append(pi)

# Essential prime implicants: minterms covered by exactly one PI.
for m, pis in pi_covers.items():
    if len(pis) == 1:
        pi = pis[0]
        if pi not in key_prime_implicants:
            key_prime_implicants.append(pi)

# Mark minterms covered by essential prime implicants.
for pi in key_prime_implicants:
    for m in pi[2]:
        if m in Minterm_Index:
            covered_minterms.add(m)

# Determine which minterms remain uncovered.
for m in Minterm_Index:
    if m not in covered_minterms:
        uncovered_minterms.add(m)

# Greedy cover: repeatedly choose the PI that covers the most remaining minterms.
while len(uncovered_minterms) > 0:
    best_pi = None
    best_new_cover = set()

    for pi in prime_implicants:
        if pi in key_prime_implicants:
            continue

        new_cover = uncovered_minterms.intersection(pi[2])

        if len(new_cover) > len(best_new_cover):
            best_new_cover = new_cover
            best_pi = pi

    if best_pi is None:
        print("ERROR: Could not cover all minterms with available prime implicants.")
        break

    key_prime_implicants.append(best_pi)
    uncovered_minterms = uncovered_minterms - best_new_cover

# ------------------ Convert Selected Implicants to Boolean Expression ------------------
for pi in key_prime_implicants:
    bits = pi[0]
    mask = pi[1]
    expression_terms.append(implicant_to_expr(bits, mask, var_names))

final_expression = " + ".join(expression_terms) if expression_terms else "0"

print("\nSimplified Expression:")
print(final_expression)
