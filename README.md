# Frogpad_optimizer
Keyboard layout optimizer for Frogpad (one-handed) keyboard layout. <br>

<img src="https://github.com/Waterdragen/Frogpad_optimizer/blob/main/images/frogpad_right_hand_keyboard.png" width="600">

*Standard Frogpad keybaord (right handed)* <br>

### Terms and definitions:

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
The Chord layer is 2.5 times the effort of Base layer. The average effort is 1.00 so a random keyboard layout is expected to have around 100 effort. (Which is the median of Generation 1)

### Pseudocode for keyboard layout performance criteria:

Sfb: `col0 == col1 or (col0, col1) in ((0, 1), (1, 0))`  <br>
Lsb: `abs(row0 - row1) == 2 or ((col0 == 0 and row0 != 1) or (col1 == 0 and row1 != 1) and abs(row0 - row1) >= 1))`  <br>
Inroll: `col0 > col1 > col2`  <br>
Outroll: `col0 < col1 < col2`  <br>
I/O diff.: `inroll - outroll`  <br>
Rolls: `inroll + outroll`  <br>
Effort: `Monogram.frequency * keyboard_effort`

## Generation 1

I ran 1 million random keyboard combinations and gets the Best 10% then I generated the hash tables of the generation. The hash tables are divided into row, col, layer, and key. The row_diff, col_diff, and layer_diff hash tables is based on the most frequent location minus the 2nd most frequent location. The higher the difference, the more confidence we have for a key to be in a certain position.The most frequent and highest diff keys are constrained for the next generation.

Results:

|  | Best Quartile | Best 1% | Frogpad Reference |
| ----- | ----- | ----- | ----- |
| Sfb | 23.7686 | 16.8344 | 14.0079 |
| Lsb | 29.6013 | 15.6122 | 18.9169 |
| I/O diff. | 1.43112 | 5.60616 | -1.2270 |
| Rolls | 13.0083 | 18.4849 | 18.9485 |
| Effort | 92.3998 | 77.8503 | 58.5 |

Frogpad top 10 Inrolls: 'the', 'tha', 'pro', 'thi', 'pre', 'tra', 'whi', 'tho', 'cha', 'whe'
Frogpad top 10 Outrolls: 'and', 'ing', 'ent', 'ons', 'ers', 'int', 'enc', 'ant', 'ity', 'art'
This shows Frogpad is excellent at low Sfb and low effort while maintaining high rolls. However, it performs poorly in I/O diff. Even though Frogpad got the most frequent trigram to be an Inroll, and the h -> vowel Inroll motion, the vowel -> n Outroll motion hit its I/O diff hard.  <br>

Row / Col / Layer constriants:

| Constraint | Key location |
| ---------- | ------------ |
| row | e<sub>1</sub> t<sub>1</sub> r<sub>1</sub> n<sub>1</sub> |
| col | t<sub>4</sub> e<sub>2</sub> q<sub>0</sub> s<sub>1</sub> |
| layer | t<sub>0</sub> e<sub>0</sub> o<sub>0</sub> s<sub>0</sub> |

#### Key constraints for the next generation:

| Letter | Key location |
| ------ | ------------ |
| e | 7 |
| t | 9 |
| s | 1, 6, 11 |
| q | 0, 5, 10, 15, 20, 25 |
| nr | 5, 6, 8, 20, 21, 22, 23, 24 |
| o | `layer1` |

## Generation 2

Results:

|  | Best Quartile | Best 1% | Frogpad Reference |
| ----- | ----- | ----- | ----- |
| Sfb | 21.4028 | 16.1287 | 14.0079 |
| Lsb | 17.11 | 9.46559 | 18.9169 |
| I/O diff. | 3.22773 | 8.5716 | -1.2270 |
| Rolls | 15.5269 | 21.0065 | 18.9485 |
| Effort | 72.2655 | 64.2417 | 58.5 |

Row / Col / Layer constriants:

| Constraint | Key location |
| ---------- | ------------ |
| row | [e<sub>1</sub> n<sub>1</sub> r<sub>1</sub> t<sub>1</sub>] a<sub>1</sub> i<sub>1</sub> |
| col | [e<sub>2</sub> q<sub>0</sub> s<sub>1</sub> t<sub>4</sub>] h<sub>3</sub> a<sub>2</sub> n<sub>1</sub> o<sub>2</sub> |
| layer | [e<sub>0</sub> o<sub>0</sub> s<sub>0</sub> t<sub>0</sub>] h<sub>0</sub> x<sub>0</sub> |

#### Key constraints for the next generation:

| Letter | Key location |
| ------ | ------------ |
| e | 7 |
| t | 9 |
| *a* | 22 |
| *n* | 6, 21 |
| o | 2, 12 |
| s | 1, 6, 11 |
| h | 3, 8, 13 |
| q | 0, 5, 10, 15, 20, 25 |
| *i*r | 5, 6, 8, 20, 21, 22, 23, 24 |
| *x* | `layer1` |

## Generation 3

Results:

|  | Best Quartile | Best 1% | Frogpad Reference |
| ----- | ----- | ----- | ----- |
| Sfb | 16.1199 | 12.5505 | 14.0079 |
| Lsb | 11.7902 | 7.358 | 18.9169 |
| I/O diff. | 8.8888 | 11.4487 | -1.2270 |
| Rolls | 21.7751 | 24.6741 | 18.9485 |
| Effort | 67.0121 | 61.0877 | 58.5 |

Row / Col / Layer constriants:

| Constraint | Key location |
| ---------- | ------------ |
| row | [a<sub>1</sub> e<sub>1</sub> i<sub>1</sub> n<sub>1</sub> r<sub>1</sub> t<sub>1</sub>] |
| col | [a<sub>2</sub> e<sub>2</sub> h<sub>3</sub> n<sub>1</sub> o<sub>2</sub> q<sub>0</sub> s<sub>1</sub> t<sub>4</sub>] i<sub>3</sub> x<sub>0</sub> c<sub>4</sub> w<sub>4</sub> l<sub>1</sub> |
| layer | [e<sub>0</sub> h<sub>0</sub> o<sub>0</sub> s<sub>0</sub> t<sub>0</sub> x<sub>0</sub>] j<sub>1</sub> k<sub>1</sub> z<sub>1</sub> |

#### Key constraints for the next generation:

| Letter | Key location |
| ------ | ------------ |
| e | 7 |
| t | 9 |
| a | 22 |
| n | 6, 21 |
| *i* | 8, 23 |
| o | 2, 12 |
| s | 1, 6, 11 |
| h | 3, 8, 13 |
| *x* | 15, 20, 25 |
| q | 0, 5, 10, 15, 20, 25 |
| r | 5, 6, 8, 20, 21, 22, 23, 24 |
| *cw* | 4, 9, 14, 19, 24, 29 |
| *l* | 1, 6, 11, 16, 21, 26 |
| *jkz* | `layer1` |

## Generation 4

Results:

|  | Best Quartile | Best 1% | Frogpad Reference |
| ----- | ----- | ----- | ----- |
| Sfb | 14.0984 | 11.4681 | 14.0079 |
| Lsb | 11.163 | 7.61232 | 18.9169 |
| I/O diff. | 11.3891 | 13.0336 | -1.2270 |
| Rolls | 24.1768 | 25.8826 | 18.9485 |
| Effort | 61.247 | 57.5032 | 58.5 |

Row / Col / Layer constriants:

| Constraint | Key location |
| ---------- | ------------ |
| row | [a<sub>1</sub> e<sub>1</sub> i<sub>1</sub> n<sub>1</sub> r<sub>1</sub> t<sub>1</sub>] x<sub>1</sub>  |
| col | [a<sub>2</sub> c<sub>4</sub> e<sub>2</sub> h<sub>3</sub> i<sub>3</sub> l<sub>1</sub> n<sub>1</sub> o<sub>2</sub> q<sub>0</sub> s<sub>1</sub> t<sub>4</sub> w<sub>4</sub> x<sub>0</sub>] u<sub>2</sub> m<sub>4</sub> p<sub>3</sub> d<sub>1</sub> r<sub>0</sub> |
| layer | [a<sub>1</sub> e<sub>0</sub> h<sub>0</sub> j<sub>1</sub> k<sub>1</sub> o<sub>0</sub> s<sub>0</sub> t<sub>0</sub> x<sub>0</sub> z<sub>1</sub>] i<sub>0</sub> q<sub>0</sub> r<sub>0</sub> u<sub>0</sub> d<sub>0</sub> n<sub>0</sub>|

#### Key constraints for the next generation:

| Letter | Key location |
| ------ | ------------ |
| *r* | 5 |
| *n* | 6 |
| e | 7 |
| *i* | 8 |
| t | 9 |
| *x* | 20 |
| a | 22 |
| *d*s | 1, 11 |
| *q* | 0, 10 |
| o*u* | 2, 12 |
| h | 3, 13 |
| c*m*w | 4, 14, 19, 24, 29 |
| *p* | 3, 8, 13, 18, 23, 28 |
| l | 1, 6, 11, 16, 21, 26 |
| jkz | `layer1` |

## Generation 5

Results:

|  | Best Quartile | Best 1% | Frogpad Reference |
| ----- | ----- | ----- | ----- |
| Sfb | 12.5758 | 11.3736 | 14.0079 |
| Lsb | 10.4074 | 8.15633 | 18.9169 |
| I/O diff. | 11.3891 | 14.1317 | -1.2270 |
| Rolls | 26.453 | 27.3638 | 18.9485 |
| Effort | 61.1999 | 57.4238 | 58.5 |

At this point, no keyboard layouts got Best 10% for each category. So I have to adjust the threshold to Best 15%.  <br>

Row / Col / Layer constriants:

| Constraint | Key location |
| ---------- | ------------ |
| row | [a<sub>1</sub> e<sub>1</sub> i<sub>1</sub> n<sub>1</sub> r<sub>1</sub> t<sub>1</sub>] l<sub>1</sub> o<sub>0</sub> u<sub>2</sub>|
| col | [a<sub>2</sub> c<sub>4</sub> d<sub>1</sub> e<sub>2</sub> h<sub>3</sub> i<sub>3</sub> l<sub>1</sub> m<sub>4</sub> n<sub>1</sub> o<sub>2</sub> p<sub>3</sub> q<sub>0</sub> r<sub>0</sub> s<sub>1</sub> t<sub>4</sub> u<sub>2</sub> w<sub>4</sub> x<sub>0</sub>] b<sub>3</sub> g<sub>4</sub> y<sub>0</sub> |
| layer | [a<sub>1</sub> d<sub>0</sub> e<sub>0</sub> h<sub>0</sub> i<sub>0</sub> j<sub>1</sub> k<sub>1</sub> n<sub>0</sub> o<sub>0</sub> q<sub>0</sub> r<sub>0</sub> s<sub>0</sub> t<sub>0</sub> u<sub>0</sub> x<sub>0</sub> z<sub>1</sub>] l<sub>1</sub> w<sub>1</sub> p<sub>1</sub> y<sub>0</sub> |

#### Key constraints for the next generation:

| Letter | Key location |
| ------ | ------------ |
| *o* | 2 |
| r | 5 |
| n | 6 |
| e | 7 |
| i | 8 |
| t | 9 |
| *u* | 12 |
| *-* | 17 |
| x | 20 |
| *l* | 21 |
| a | 22 |
| *-* | 27 |
| ds | 1, 11 |
| *y*q | 0, 10 |
| h | 3, 13 |
| *w* | 19, 24, 29 |
| cm*g* | 4, 14, 19, 24, 29 |
| *p* | 18, 23, 28 |
| *b* | 3, 13, 18, 23, 28 |
| jkz | `layer1` |
















