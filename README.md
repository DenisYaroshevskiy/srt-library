Small library where I experiment with flat containers.

IMPORTANT NOTE IF YOU ATTENDED THE TALK:
I found a bug in my implementation of set_union - the standard demands a
certain behavior for repeating duplicates (like: [2] [2, 2, 2] => [2, 2, 2])
I haven't tested for that and it's broken. For the purpose of flat_sets it
doesn't matter, since we merge sets without duplicates.
But now set_union is renamed to be set_union_unique.

This library was primarily created to help with the standardization process of flat_sets,
concentrating on different algorithms and their performance.
(Consider, for example, implementing: flat_set::insert(first, last); - it's
 quite tricky).

Google document with the results (the effort is ongoing):
https://docs.google.com/document/d/1Q5hO0tf1Z4HCL6eArtsNP8RjcKD0g3mggxJc0D_Ulu4/edit?usp=sharing

Main, one header library, that can be easily included in things like Quick-Bench,
with the latest versions of all of the required algorithms.
It also contains an experimental version of flat_set.
https://github.com/DenisYaroshevskiy/srt-library/blob/master/srt.h

slides on the insert(first, last) (in Russian):
https://github.com/DenisYaroshevskiy/srt-library/blob/master/presentation_rus.pdf

