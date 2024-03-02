import subprocess
import sys

phase = str(sys.argv[1])
hour = '0'
minute = '3'
second = '0'
alpha = '20'
n_patience = '1'

model_dir = './../../../model/nomodel/'


# 6.6
train_data_nums = [6, 7, 8, 9, 10, 11, 15, 16, 17, 18, 19, 20, 21, 22, 24, 25, 26]
if phase == '11':
    train_data_nums.remove(20)
    train_data_nums.remove(21)
    train_data_nums.remove(22)
train_root_dir = './../../../train_data/bin_data/20240223_1/'
#model_dir = './../../../model/20240226_3/'
executable = 'eval_optimizer_cuda_12_2_0.exe'
'''
# 6.6 move ordering end nws
train_data_nums = [24, 25]
train_root_dir = './../../../train_data/bin_data/20240301_1_move_ordering_end_nws/'
executable = 'eval_optimizer_cuda_12_2_0_move_ordering_end_nws.exe'
#model_dir = './../../../model/20240226_3/'
'''

train_data = [str(elem) + '.dat' for elem in train_data_nums]

train_dirs = [train_root_dir + str(int(phase)) + '/']

additional_params = ''
for tfile in train_data:
    for train_dir in train_dirs:
        additional_params += ' ' + train_dir + tfile

cmd = executable + ' ' + phase + ' ' + hour + ' ' + minute + ' ' + second + ' ' + alpha + ' ' + n_patience + ' ' + model_dir + phase + '.txt' + additional_params
print(cmd, file=sys.stderr)
p = subprocess.Popen(cmd.split(), stdout=subprocess.PIPE)
result = p.stdout.readline().decode().replace('\r\n', '\n').replace('\n', '')
print(result)
param = p.stdout.read().decode().replace('\r\n', '\n')
with open('trained/' + phase + '.txt', 'w') as f:
    f.write(param)
