# Frogpad_optimizer
Keyboard layout analyzer for Frogpad (one-handed) keyboard layout. <br>

<img src="https://github.com/Waterdragen/Frogpad_optimizer/blob/main/images/frogpad_right_hand_keyboard.png" width="600">

*Standard Frogpad keybaord (right handed)* <br>

### Terms and definitions:

| Criterion | Description |
| --------- | ----------- |
| Monogram | 1-letter combination |
| **Bigram** | 2-letter combination |
| **Trigram** | 3-letter combination |
| Base Layer | the 15 keys at the middle that can be pressed directly. |
| Chord Layer | the Base layer pressed together with the space. |
| **Effort** | The typing effort of the keyboard layout. Less weight for stronger fingers and more comfortable position. |
| Same finger bigram (**Sfb**) | Bigrams typed with the same finger |
| Lateral stretch bigram (**Lsb**) | Bigrams require uncomfortable finger stretching (ex.: top row -> bottom row). |
| **Alternates** | The trigrams typed by switching fingers (ex.: finger A->B->C or A->B->A) |
| Strong alternates | Alternates using stronger fingers. Suppose Index=2, Middle=1, Ring=3, Pinky=4. The sum of the three fingers' indices <= 6 |
| Weak alternates | The sum of the three fingers' indices > 6 |
| **Strong alt ratio** | The ratio of Strong Alternates to Total Alternates |
| **OneLayer** | The trigram typed entirely on the Base Layer |
| **OneDirection** | OneLayer trigrams that go one direction (either rolls inward or outward) |


row1 (as variable): row of the 2nd key pressed  <br>
`row1` (as json key): keys 5, 6, 7, 8, 9, 20, 21, 22, 23, 24  <br>

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
The Chord layer effort is 2.5 times the effort of Base layer. The average effort is 1.00 so a random keyboard layout is expected to have around 100 effort. (Which is the median of Generation 1)
