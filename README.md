# Frogpad_optimizer
Keyboard layout optimizer for Frogpad (one-handed) keyboard layout. <br>

<img src="https://github.com/Waterdragen/Frogpad_optimizer/blob/main/images/frogpad_right_hand_keyboard.png" width="600">

*Standard Frogpad keybaord (right handed)* <br>

<h3>Terms and definitions:</h3>

Monogram: 1-letter combination  <br>
**Bigram**: 2-letter combination  <br>
**Trigram**: 3-letter combination  <br>
Base layer: the 15 keys at the middle that can be pressed directly.
Chord layer: the Base layer pressed together with the space.
Same finger bigram (**Sfb**): Bigrams typed with the same finger. Should be minimized.  <br>
Lateral stretch bigram (**Lsb**): Bigrams require uncomfortable finger stretching (ex.: top row -> bottom row). Should be minimized.  <br>
Inward roll (Inrolls): Trigrams typed with 3 separate fingers from the outer column to the inner column. Should be maximized.  <br>
Outward roll (Outrolls): Trigrams typed with 3 separate fingers from the innder column to the outer column.  <br>
Inward/Outward roll difference (**I/O diff.**): Name. Positive if inward rolls are more frequent than outward rolls and vice versa. Should be maximized.  <br>
Total rolls (**Rolls**): Sum of inrolls and outrolls. Should be maximized.  <br>
**Effort**: The typing effort of the keyboard layout. Less weight for stronger fingers and more comfortable position. Should be minimized.  <br>
row0 (as variable): row of the 0th key pressed  <br>
`row0` (as json key): keys 0, 5, 10, 15, 20, 25  <br>

This optimizer uses the right handed version to compute performance.  <br>
The keyboard layout will be stored as a string:  <br>
`farwpoehtduinsy--bmjqzlcv--kgx`  <br>
And the keyboard layout will be represented as a grid (left: Base layer, right: Chord layer):  <br>
```
farwp  --bmj
oehtd  qzlcv
uinsy  --kgx
```
Notes: Home row is placed at `row1`; index finger presses `col0` and `col1`

<h3>Keyboard effort model</h3>

<table>
  <tr>
    <td valign="top"><img src="https://github.com/Waterdragen/Frogpad_optimizer/blob/main/images/keyboard_effort_model_a.png" width="400">
    <td valign="top"><img src="https://github.com/Waterdragen/Frogpad_optimizer/blob/main/images/keyboard_effort_model_b.png" width="400">
  </tr>
</table>

*Base layer effort (left) and Chord layer effort (right)*  <br>

<h3>Pseudocode for keyboard layout performance criteria:</h3>

Sfb: `col0 == col1 or (col0, col1) in ((0, 1), (1, 0))`  <br>
Lsb: `abs(row0 - row1) == 2 or ((col0 == 0 and row0 != 1) or (col1 == 0 and row1 != 1) and abs(row0 - row1) >= 1))`  <br>
Inroll: `col0 > col1 > col2`  <br>
Outroll: `col0 < col1 < col2`  <br>
I/O diff.: `inroll - outroll`  <br>
Rolls: `inroll + outroll`  <br>

<h3>Generation 1</h3>

First, I ran 1 million random keyboard combinations 
