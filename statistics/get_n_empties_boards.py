n_empties_min = 13
n_empties_max = 13

max_n_data = 1000

with open('data/records15/0000000.txt', 'r') as f:
    data = f.read().splitlines()
with open('data/n_empties/' + str(n_empties_min) + '_' + str(n_empties_max) + '.txt', 'w') as f:
    n_data = 0
    for datum in data:
        if n_data == max_n_data:
            break
        datum = datum[:64]
        n_empties = datum.count('.')
        if n_empties_min <= n_empties <= n_empties_max:
            f.write(datum + '\n')
            n_data += 1