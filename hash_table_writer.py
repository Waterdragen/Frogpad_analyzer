import json
import os
import re
from collections import OrderedDict
from typing import List, Dict


class Frogpad:
    with open("n2gram_frequency.json", "r") as f:
        bigrams: dict = json.loads(f.read())
    with open("n3gram_frequency.json", "r") as f:
        trigrams: dict = json.loads(f.read())

    @classmethod
    def keyboard_location_map(cls, keys: List[str]) -> Dict[str, List[int]]:
        kb_map: Dict[str, List[int]] = {}
        for pos_num, key in enumerate(keys):
            if key == "-":
                continue
            layer = pos_num // 15
            row = pos_num % 15 // 5
            col = pos_num % 15 % 5
            kb_map[key] = [layer, row, col]
        return kb_map

    @classmethod
    def get_sfb_lsb_and_list(cls, keys: List[str]):
        bigram: str
        freq: float
        sfb_counter: float = 0.0
        sfb_list: List[str] = []
        lsb_counter: float = 0.0
        lsb_list: List[str] = []
        kb_map: Dict[str, List[int]] = cls.keyboard_location_map(keys)

        for bigram, freq in cls.bigrams.items():
            pos0 = kb_map[bigram[0]]
            pos1 = kb_map[bigram[1]]
            row1 = pos0[1]
            row2 = pos1[1]
            col1 = pos0[2]
            col2 = pos1[2]

            if col1 == col2 or (col1, col2) in ((0, 1), (1, 0)):
                sfb_list.append(bigram)
                sfb_counter += freq
            if abs(row2 - row1) == 2 or ((col1 == 0 and row1 != 1) or (col2 == 0 and row2 != 1) and
                                         abs(row2 - row1) >= 1):
                lsb_list.append(bigram)
                lsb_counter += freq
        return sfb_counter, lsb_counter, sfb_list, lsb_list

    @classmethod
    def get_trigram_rolls_and_list(cls, keys: List[str]):
        trigram: str
        freq: int
        inward_counter: float = 0.0
        outward_counter: float = 0.0
        inward_roll_list: List[str] = []
        outward_roll_list: List[str] = []
        kb_map: Dict[str, List[int]] = cls.keyboard_location_map(keys)
        for trigram, freq in cls.trigrams.items():
            col0 = kb_map[trigram[0]][2]
            col1 = kb_map[trigram[1]][2]
            col2 = kb_map[trigram[2]][2]

            # Exclude same column and column 0, 1 combination
            if 0 in (col0, col1, col2) and 1 in (col0, col1, col2):
                continue
            if col0 > col1 > col2:
                inward_counter += freq
                inward_roll_list.append(trigram)
            if col0 < col1 < col2:
                outward_counter += freq
                outward_roll_list.append(trigram)
        return (inward_counter, outward_counter, inward_counter - outward_counter,
                inward_counter + outward_counter, inward_roll_list, outward_roll_list)

    @classmethod
    def get_keyboard_effort(cls, keys: List[str]) -> float:
        with open("n1gram_frequency.json", "r") as f:
            kb_freq_table: Dict[str, float] = json.loads(f.read())
        with open("keyboard_effort_table.json", "r") as f:
            kb_effort_table: Dict[str, float] = json.loads(f.read())

        keyboard_effort_counter: float = 0.0
        for i in range(30):
            if keys[i] == "-":
                continue
            keyboard_effort_counter += kb_freq_table[keys[i]] * kb_effort_table[f"key{i}"]
        return keyboard_effort_counter


def keyboard_effort_table():
    base_layer_effort: List[float] = [3.6, 2.2, 2.0, 2.4, 3.7,
                                      2.9, 1.0, 1.0, 1.2, 1.7,
                                      2.6, 1.6, 2.5, 2.8, 3.7]
    multiplier = 2.5
    base_layer_effort.extend([i * multiplier for i in base_layer_effort])
    total = sum(base_layer_effort)
    base_layer_effort = [i / total * 30 for i in base_layer_effort]
    print("Average: ", end="")
    print(float(sum(base_layer_effort) / len(base_layer_effort)))

    effort_table: dict = {}
    for v, i in enumerate(base_layer_effort):
        effort_table[f"key{v}"] = i
    effort_table_sorted = dict(sorted(effort_table.items(),
                                      key=lambda item: int("".join(re.findall("[0-9]+", item[0])))))
    effort_table_json = json.dumps(effort_table_sorted, indent=4)
    print(effort_table_sorted)
    with open("keyboard_effort_table.json", "w") as f:
        f.write(effort_table_json)


def frogpad_hash_table(filename: str, chunk_width: int, mode: str, include_blanks: bool = False):
    if mode not in ("row", "col", "layer", "key"):
        raise ValueError("mode must be in: row, col, layer, key")

    line_num: int
    line_text: str
    hash_table: Dict[str, dict] = {}
    sub_hash_table_num = {"row": 3, "col": 5, "layer": 2, "key": 30}
    for i in range(sub_hash_table_num[mode]):
        hash_table[f"{mode}{i}"] = {}

    for line_num, line_text in enumerate(open(filename, "r")):
        if line_num % chunk_width == 0:
            keys_text = line_text.split(": ")[1].strip()
            for v, c in enumerate(keys_text):
                pos_mode = {"row": v % 15 // 5, "col": v % 15 % 5, "layer": v // 15, "key": v}
                if not include_blanks and c == "-":
                    continue
                key_name = f"{mode}{pos_mode[mode]}"
                if c not in hash_table[key_name]:
                    hash_table[key_name][c] = 1
                else:
                    hash_table[key_name][c] += 1
    hash_table_sorted = {}
    for i in range(sub_hash_table_num[mode]):
        hash_table_sorted[f"{mode}{i}"] = dict(sorted(hash_table[f"{mode}{i}"].items(),
                                                      key=lambda item: item[1], reverse=True))
    hash_table_json = json.dumps(hash_table_sorted, indent=4)
    with open(f"{os.path.splitext(filename)[0]}_hash_table_{mode}"
              f"{'_with_blanks' if include_blanks else ''}.json", "w") as f:
        f.write(hash_table_json)


def frogpad_key_position_hash_table(file_path, chunk_width: int):
    key_pos_hash_table: dict[str, dict] = {}
    for i in range(97, 123):
        key_pos_hash_table[chr(i)] = {}
    for line_num, line_text in enumerate(open(file_path)):
        if line_num % chunk_width == 0:
            line_text_split = line_text.split(": ")
            keys_text = "".join(re.findall("[a-z:-]", line_text_split[1]))
            for v, i in enumerate(keys_text):
                if i == "-":
                    continue
                if v not in key_pos_hash_table[i]:
                    key_pos_hash_table[i][v] = 1
                else:
                    key_pos_hash_table[i][v] += 1
    key_pos_hash_table_sorted = {}
    for i in range(97, 123):
        key_pos_hash_table_sorted[chr(i)] = dict(sorted(key_pos_hash_table[chr(i)].items(),
                                                        key=lambda item: item[1], reverse=True))
    key_pos_hash_map_json = json.dumps(key_pos_hash_table_sorted, indent=4)
    with open(f"{os.path.splitext(file_path)[0]}_key_pos_hash_table.json", "w") as f:
        f.write(key_pos_hash_map_json)


def frogpad_hash_table_max_difference(file_path, mode: str):
    if mode not in ("row", "col", "layer"):
        raise ValueError("mode must be in: row, col, layer")
    total_items = {"row": 3, "col": 5, "layer": 2}

    with open(file_path, "r") as f:
        hash_table: Dict[str, Dict[str, int]] = json.loads(f.read())
        difference_hash_table: dict = {}

        for i in range(97, 123):
            temp_val = [0] * total_items[mode]
            for item_num, (item_str, item_table) in enumerate(hash_table.items()):
                if chr(i) in item_table:
                    temp_val[item_num] = item_table[chr(i)]
            temp_val.sort(reverse=True)
            difference_hash_table[chr(i)] = temp_val[0] - temp_val[1]

    difference_hash_table_sorted = dict(sorted(difference_hash_table.items(),
                                               key=lambda item: item[1], reverse=True))
    difference_hash_table_json = json.dumps(difference_hash_table_sorted, indent=4)
    with open(f"{os.path.splitext(file_path)[0]}_difference.json", "w") as f:
        f.write(difference_hash_table_json)


def frogpad_compare_hash_table_rank_difference(file_path1, file_path2):
    with open(file_path1, "r") as f:
        hash_table1: Dict[str, int] = json.loads(f.read())
    with open(file_path2, "r") as f:
        hash_table2: Dict[str, int] = json.loads(f.read())

    ordered_hash_table1 = OrderedDict()
    ordered_hash_table2 = OrderedDict()
    for v, k in enumerate(hash_table1):
        ordered_hash_table1[k] = v
    for v, k in enumerate(hash_table2):
        ordered_hash_table2[k] = v

    difference_hash_table: dict = {}
    for v, k in enumerate(ordered_hash_table1.keys()):
        difference_hash_table[k] = v + list(ordered_hash_table2.keys()).index(k)

    difference_hash_table_sorted = dict(sorted(difference_hash_table.items(),
                                               key=lambda item: item[1]))
    difference_hash_table_json = json.dumps(difference_hash_table_sorted, indent=4)
    with open(f"{os.path.splitext(file_path1)[0]}_compare.json", "w") as f:
        f.write(difference_hash_table_json)


def frogpad_gen_hash_table(gen: int):
    frogpad_hash_table(f"gen{gen}.txt", 7, "row")
    frogpad_hash_table(f"gen{gen}.txt", 7, "col")
    frogpad_hash_table(f"gen{gen}.txt", 7, "layer")
    frogpad_hash_table(f"gen{gen}.txt", 7, "key")
    frogpad_key_position_hash_table(f"gen{gen}.txt", 7)
    frogpad_hash_table_max_difference(f"gen{gen}_hash_table_row.json", "row")
    frogpad_hash_table_max_difference(f"gen{gen}_hash_table_col.json", "col")
    frogpad_hash_table_max_difference(f"gen{gen}_hash_table_layer.json", "layer")
    frogpad_compare_hash_table_rank_difference(f"gen{gen}_hash_table_row_difference.json",
                                               f"gen{gen}_hash_table_col_difference.json")


def frogpad_best_keyboard(gen: int):
    temp_text = ""
    line_text: str
    categories = {1: [], 2: [], 4: [], 5: [], 6: []}
    for line_num, line_text in enumerate(open(f"gen{gen}.txt", "r")):
        temp_text += line_text
        if line_num % 7 in (1, 2, 4, 5, 6):
            value = float(line_text.split(": ")[1])
            categories[line_num % 7].append(value)
    for k, i in categories.items():
        if k in (1, 2, 6):
            i.sort()
        else:
            i.sort(reverse=True)

    ranking: dict = {}

    curr_kb_layout = ""
    temp_rank = 0
    for line_num, line_text in enumerate(open(f"gen{gen}.txt", "r")):
        if line_num % 7 == 0:
            curr_kb_layout = line_text
        if line_num % 7 in (1, 2, 4, 5, 6):
            value = float(line_text.split(": ")[1])
            temp_rank += categories[line_num % 7].index(value)
            if line_num % 7 == 6:
                ranking[curr_kb_layout.split(": ")[1].strip()] = temp_rank
                temp_rank = 0
    ranking_sorted = dict(sorted(ranking.items(), key=lambda item: item[1]))
    print(ranking_sorted)


def usage_analyzer(keys_str: str):
    with open("n1gram_frequency.json", "r") as f:
        freq_table = json.loads(f.read())
    base_layer_usage = 0
    fingers_usage = [0, 0, 0, 0, 0]
    fingers_name = ["thumb", "index", "middle", "ring", "pinky"]
    for keys_char in keys_str[:15]:
        base_layer_usage += freq_table[keys_char]
    print(f"Base layer usage: {base_layer_usage}%")
    effort = Frogpad.get_keyboard_effort([c for c in keys_str])
    print(f"Effort: {effort}")

    home_row_usage = 0
    for keys_num, keys_char in enumerate(keys_str):
        if keys_char == "-":
            continue
        if keys_num in (6, 7, 8, 9, 21, 22, 23, 24):
            home_row_usage += freq_table[keys_char]
        if keys_num // 15 == 1:
            fingers_usage[0] += freq_table[keys_char]
        if keys_num % 15 % 5 == 0:
            fingers_usage[1] += freq_table[keys_char]
            continue
        fingers_usage[keys_num % 15 % 5] += freq_table[keys_char]
    for usage, name in zip(fingers_usage, fingers_name):
        print(f"{name}: {usage}%")
    print(f"Home row usage: {home_row_usage}%")

    roll_performance = Frogpad.get_trigram_rolls_and_list([c for c in keys_str])
    print(f"Inward roll: {roll_performance[0]} Outward roll: {roll_performance[1]}")
    print(f"I/O difference: {roll_performance[2]}")
    print(f"Top 50 inward roll list:\n{roll_performance[4][:50]}")
    print(f"Top 50 outward roll list:\n{roll_performance[5][:50]}")

    sfb_lsb_performance = Frogpad.get_sfb_lsb_and_list([c for c in keys_str])
    print(f"Sfb: {sfb_lsb_performance[0]}")
    print(f"Top 50 sfb list: {sfb_lsb_performance[2][:50]}")
    print(f"Lsb:\n{sfb_lsb_performance[1]}")
    print(f"Top 50 lsb list:\n{sfb_lsb_performance[3][:50]}\n")

if __name__ == '__main__':
    ...
    # keyboard_effort_table()
    # frogpad_gen_hash_table(1)
    # frogpad_gen_hash_table(2)
    # frogpad_gen_hash_table(3)
    # frogpad_gen_hash_table(4)
    # frogpad_hash_table("gen4.txt", 7, "key", True)
    # frogpad_gen_hash_table(5)
    # frogpad_best_keyboard(5)
    # frogpad_hash_table("gen5.txt", 7, "key", True)
    # frogpad_gen_hash_table(6)
    # frogpad_best_keyboard(6)
    # frogpad_hash_table("gen6.txt", 7, "key", True)
    # frogpad_best_keyboard(7)
    usage_analyzer("farwpoehtduinsy--bmjqzlcv--kgx")  # Frogpad standard layout
    usage_analyzer("qdofmrneitysuhc-j-bvxlapw-z-kg")  # Gen 7 best keyboard layout
