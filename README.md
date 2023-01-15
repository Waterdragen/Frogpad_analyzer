# Frogpad_optimizer
Keyboard layout optimizer for Frogpad (one-handed) keyboard layout. <br>

![alt text](https://github.com/Waterdragen/Frogpad_optimizer/blob/main/images/frogpad_right_hand_keyboard.png?raw=true) <br>
*Standard Frogpad keybaord (right handed)* <br>

Terms and definitions:  <br>
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

This optimizer uses the right handed version to compute performance.  <br>
The keyboard layout will be stored as a string:  <br>
`farwpoehtduinsy--bmjqzlcv--kgx`  <br>
And the keyboard layout will be represented as a grid (left: Base layer, right: Chord layer):  <br>
```
farwp  --bmj
oehtd  qzlcv
uinsy  --kgx
```

Implementation:
