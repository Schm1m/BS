#!/bin/bash

# awk {regex} {file} will return all lines, where the line matches the given regex in the given file
# the regex "[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,4}" is a standart email regex used all over the place in the internet
# cut {option} {option}... will cut these inputs at the given delimiter (-d{delimiter}) and return the second field (-f{field})
# sort sorts all lines (if not present, uniq will have trouble counting up the lines correctly)
# uniq {option} will return only unique lines and counts (-c) how often the same line appears
# sort {option} will sort the lines by a number (-n)

awk '/From [a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,4}/' gcc_2023.08.txt | cut -d" " -f2 | sort | uniq -c | sort -n

# solution to not include the domain ending ( switches from 89 different to 84 different domains )

awk '/From [a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,4}/' gcc_2023.08.txt | cut -d" " -f2 | cut -d@ -f2 | cut -d. -f1 | sort | uniq -c | sort -n