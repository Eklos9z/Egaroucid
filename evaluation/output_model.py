from copy import deepcopy
import tensorflow as tf
from tensorflow.keras.datasets import boston_housing
from tensorflow.keras.layers import Activation, Add, BatchNormalization, Conv2D, Dense, GlobalAveragePooling2D, Input, concatenate, Flatten, Dropout, Lambda #, LeakyReLU
from tensorflow.keras.models import Sequential, Model, load_model
from tensorflow.keras.callbacks import EarlyStopping, LearningRateScheduler, LambdaCallback, ModelCheckpoint
from tensorflow.keras.optimizers import Adam
#from keras.layers.advanced_activations import LeakyReLU
from tensorflow.keras.regularizers import l2
from tensorflow.keras.utils import plot_model
import numpy as np
import matplotlib.pyplot as plt
from tqdm import trange
from random import randrange
import subprocess
import datetime
import os
import sys

def LeakyReLU(x):
    return tf.math.maximum(0.01 * x, x)

def get_layer_index(model, layer_name, not_found=None):
    for i, l in enumerate(model.layers):
        if l.name == layer_name:
            return i
    return not_found

def my_loss(y_true, y_pred):
    return tf.keras.backend.square(y_true - y_pred) * (tf.keras.backend.exp(-tf.keras.backend.abs(10.0 * y_true)) + 1)

for stone_strt in [0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56]:
    for black_white in range(2):
        stone_end = stone_strt + 4

        model = load_model('learned_data/' + str(black_white) + '_' + str(stone_strt) + '_' + str(stone_end) + '.h5', custom_objects={'my_loss': my_loss})

        layer_names = ['line2', 'line3', 'line4', 'diagonal5', 'diagonal6', 'diagonal7', 'diagonal8', 'edge2X', 'triangle', 'edgeblock', 'cross']
        names = []
        for name in layer_names:
            names.append(name + '_dense0')
            names.append(name + '_dense1')
            names.append(name + '_out')
        names.append('add_dense0')
        names.append('add_dense1')
        names.append('all_dense0')

        with open('learned_data/' + str(black_white) + '_' + str(stone_strt) + '_' + str(stone_end) + '.txt', 'w') as f:
            for name in names:
                i = get_layer_index(model, name)
                try:
                    #print(i, model.layers[i])
                    dammy = model.layers[i]
                    j = 0
                    while True:
                        try:
                            print(model.layers[i].weights[j].shape)
                            if len(model.layers[i].weights[j].shape) == 4:
                                for ll in range(model.layers[i].weights[j].shape[3]):
                                    for kk in range(model.layers[i].weights[j].shape[2]):
                                        for jj in range(model.layers[i].weights[j].shape[1]):
                                            for ii in range(model.layers[i].weights[j].shape[0]):
                                                f.write('{:.14f}'.format(model.layers[i].weights[j].numpy()[ii][jj][kk][ll]) + '\n')
                            elif len(model.layers[i].weights[j].shape) == 2:
                                for ii in range(model.layers[i].weights[j].shape[0]):
                                    for jj in range(model.layers[i].weights[j].shape[1]):
                                        f.write('{:.14f}'.format(model.layers[i].weights[j].numpy()[ii][jj]) + '\n')
                            elif len(model.layers[i].weights[j].shape) == 1:
                                for ii in range(model.layers[i].weights[j].shape[0]):
                                    f.write('{:.14f}'.format(model.layers[i].weights[j].numpy()[ii]) + '\n')
                            j += 1
                        except:
                            break
                except:
                    break

data = ''
for stone_strt in [0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56]:
    for black_white in range(2):
        stone_end = stone_strt + 4
        
        with open('learned_data/' + str(black_white) + '_' + str(stone_strt) + '_' + str(stone_end) + '.txt', 'r') as f:
            data += f.read()

with open('raw_param.txt', 'w') as f:
    f.write(data)