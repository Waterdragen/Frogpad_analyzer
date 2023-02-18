import json
import numpy as np
import os
import re
from collections import OrderedDict
from itertools import chain
from typing import List, Dict


def filter_ngram():
    with open(f"english.json", "r") as f:
        english_ngram_dict: dict[str, dict[str, float]] = json.load(f)
    # Map the keys for english.json to n-grams
    ngram_count: dict[str, int] = {"characters": 1, "bigrams": 2, "trigrams": 3}
    for i in ("characters", "bigrams", "trigrams"):
        new_ngram_dict: dict[str, float] = {}
        for ngram, freq in english_ngram_dict[i].items():
            # Check if the keys are the 30 alpha keys (exclude /, include ')
            if any(ch not in "abcdefghijklmnopqrstuvwxyz;,.'" for ch in ngram):
                continue
            # Convert to percentage
            new_ngram_dict[ngram] = freq * 100
        with open(f"n{ngram_count[i]}gram_frequency.json", "w", encoding="utf8") as f:
            json.dump(new_ngram_dict, f, ensure_ascii=True, indent=4)


def make_hash_table(filename: str, chunk_width: int, mode: str, include_blanks: bool = False):
    if mode not in ("row", "finger"):
        raise ValueError("mode must be in: row, finger")

    line_num: int
    line_text: str
    hash_table: Dict[str, dict] = {}
    sub_hash_table_num = {"row": range(3), "finger": range(1, 5)}
    for i in sub_hash_table_num[mode]:
        hash_table[f"{mode}{i}"] = {}

    for line_num, line_text in enumerate(open(filename, "r")):
        if line_num % chunk_width == 0:
            keys_text = line_text.split(": ")[1].strip()
            for v, c in enumerate(keys_text):
                if not include_blanks and c == "-":
                    continue
                if mode == "row":
                    key_name = f"{mode}{v % 15 // 5}"
                else:
                    finger = v % 15 % 5
                    if not finger:
                        finger = 1
                    key_name = f"{mode}{finger}"
                if c not in hash_table[key_name]:
                    hash_table[key_name][c] = 1
                else:
                    hash_table[key_name][c] += 1
    hash_table_sorted = {}
    for i in sub_hash_table_num[mode]:
        hash_table_sorted[f"{mode}{i}"] = dict(sorted(hash_table[f"{mode}{i}"].items(),
                                                      key=lambda item: item[1], reverse=True))
    hash_table_json = json.dumps(hash_table_sorted, indent=4)
    with open(f"{os.path.splitext(filename)[0]}_hash_table_{mode}"
              f"{'_with_blanks' if include_blanks else ''}.json", "w") as f:
        f.write(hash_table_json)


def make_letter_hash_table_finger(gen: int):
    line_num: int
    line_text: str
    finger_hash_table: dict[str, dict[str, int]] = dict((f"finger{i}", {}) for i in range(10))
    for line_num, line_text in enumerate(open(f"gen{gen}.txt", "r")):
        if line_num % 10 != 0:
            continue
        keyboard_layout: str = line_text.split(": ")[1].strip()
        for v, ch in enumerate(keyboard_layout):
            finger = v % 15 % 5
            if not finger:
                finger = 1
            if ch in finger_hash_table[f"finger{finger}"]:
                finger_hash_table[f"finger{finger}"][ch] += 1
            else:
                finger_hash_table[f"finger{finger}"][ch] = 1

    letter_hash_table: dict[str, list[int] | str] = {}
    for k, finger_vals in finger_hash_table.items():
        for ch in chain(range(97, 123), (39, 44, 46, 59)):
            freq: int = finger_vals.get(chr(ch), 0)
            if chr(ch) in letter_hash_table:
                letter_hash_table[chr(ch)].append(freq)
            else:
                letter_hash_table[chr(ch)] = [freq]
    letter_hash_table = dict(sorted(letter_hash_table.items(),  # NoQA
                                    key=lambda itm: sum(sorted(itm[1], reverse=True)[:4]), reverse=True))  # NoQA
    letter_hash_table = dict((k, repr(finger_vals)) for (k, finger_vals) in letter_hash_table.items())
    with open(f"gen{gen}_letter_hash_table_finger.json", "w") as f:
        json.dump(letter_hash_table, f, indent=4)


def hash_table_max_difference(file_path, mode: str):
    if mode not in ("row", "finger"):
        raise ValueError("mode must be in: row, finger")
    total_items = {"row": 3, "finger": 4}

    with open(file_path, "r") as f:
        hash_table: Dict[str, Dict[str, int]] = json.load(f)
        difference_hash_table: dict = {}

        for i in chain(range(97, 123), (39, 44, 46, 59)):
            temp_val = [0] * total_items[mode]
            for item_num, (item_str, item_table) in enumerate(hash_table.items()):
                if chr(i) in item_table:
                    temp_val[item_num] = item_table[chr(i)]
            temp_val.sort(reverse=True)
            if temp_val[1]:
                difference_hash_table[chr(i)] = round((temp_val[0] - temp_val[1]) / temp_val[1], 5)
            else:
                difference_hash_table[chr(i)] = 69  # Nice

    difference_hash_table_sorted = dict(sorted(difference_hash_table.items(),
                                               key=lambda item: item[1], reverse=True))
    difference_hash_table_json = json.dumps(difference_hash_table_sorted, indent=4)
    with open(f"{os.path.splitext(file_path)[0]}_difference.json", "w") as f:
        f.write(difference_hash_table_json)


def make_hash_tables(gen: int):
    # for i in ("row", "col", "hand"):
    #     make_hash_table(f"gen{gen}.txt", 14, i)
    #     hash_table_max_difference(f"gen{gen}_hash_table_{i}.json", i)
    make_hash_table(f"gen{gen}.txt", 10, "row")
    make_hash_table(f"gen{gen}.txt", 10, "finger")
    hash_table_max_difference(f"gen{gen}_hash_table_row.json", "row")
    hash_table_max_difference(f"gen{gen}_hash_table_finger.json", "finger")


def row_col_intersection(row: dict[str, int], col: dict[str, int | tuple[int, ...]], layer: dict[str, int]):
    intersection_table: dict = {}
    # Initialize Layer
    for k, i in layer.items():
        intersection_table[k] = list(range(i * 15, (i+1) * 15))
    # init Row if not init, else remove some Row in intersections
    for k, i in row.items():
        if k not in intersection_table:
            intersection_table[k] = list(range(i * 5, (i + 1) * 5))
            intersection_table[k].extend(list(range((i + 3) * 5, (i + 4) * 5)))
        else:
            j = len(intersection_table[k]) - 1
            while j >= 0:
                if intersection_table[k][j] % 15 // 5 != i:
                    intersection_table[k].pop(j)
                j -= 1
    # init Col if not init, else remove some Col in intersections
    for k, i in col.items():
        if isinstance(i, int):
            if k not in intersection_table:
                intersection_table[k] = list(range(i, i + 30, 5))
            else:
                j = len(intersection_table[k]) - 1
                while j >= 0:
                    if intersection_table[k][j] % 15 % 5 != i:
                        intersection_table[k].pop(j)
                    j -= 1
        if isinstance(i, tuple):
            if k not in intersection_table:
                intersection_table[k] = []
                for j in i:
                    intersection_table[k].extend(list(range(j, j + 30, 5)))
            else:
                j = len(intersection_table[k]) - 1
                while j >= 0:
                    if all(m != (intersection_table[k][j] % 15 % 5) for m in i):
                        intersection_table[k].pop(j)
                    j -= 1

    # Find fixed keys, pop the rest containing these keys
    fixed_keys: List[int] = []
    for k, i in intersection_table.items():
        if len(i) == 1:
            fixed_keys.append(i[0])
            continue
        i.sort()
    for i in intersection_table.values():
        if len(i) == 1:
            continue
        j = 0
        while j < len(i):
            if i[j] in fixed_keys:
                i.pop(j)
                continue
            j += 1

    # Sort the Table by the length of Lists
    intersection_table = dict(sorted(intersection_table.items(), key=lambda item: len(item[1])))

    # Print the Table
    for k, i in intersection_table.items():
        print(f"\"{k}\": {i},")
    remaining_keys = ""
    for j in range(97, 123):
        if chr(j) not in intersection_table:
            remaining_keys += chr(j)
    print(remaining_keys)


if __name__ == '__main__':
    ...

    make_hash_tables(4)
    GenRow = dict(Item for Item in zip("et", (1, 1)))
    GenCol = dict(Item for Item in zip("aenorith", (1, 1, 2, (0, 1), 2, (0, 1), 3, 2)))
    GenLayer = dict(Item for Item in zip("etaoinsrhld", (0, 0, 0, 0, 0, 0,
                                                         0, 0, 0, 0, 0)))
    row_col_intersection(GenRow, GenCol, GenLayer)
