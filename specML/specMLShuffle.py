# Reads in the HPC data collected with hpc-profiler for training

from keras.models import Sequential
from keras.layers import Dense
from keras.layers import LSTM
from keras.callbacks import EarlyStopping

import glob
import pandas as pd
import numpy as np

# Inputs to the neural network
inputs = ['PAPI_TOT_INS', 'PAPI_L3_TCM', 'PAPI_L3_TCA', 'PAPI_BR_MSP', 'PAPI_TOT_CYC', 'PAPI_L2_TCM']

# Build neural network
model = Sequential()
num_inputs = len(inputs)

# Add model layers
model.add( Dense(10, activation='relu', input_shape=(num_inputs,)) )
model.add( Dense(32, activation='relu') )
model.add( Dense(1, activation='sigmoid') )

# Compile model
model.compile(optimizer='adam', loss='binary_crossentropy', metrics=['accuracy'])

# If model stops improving, stop the training
early_stopping_monitor = EarlyStopping(patience=3)

# combined at from all .csv files
df_all_data = None

# Read in HPC data from .csv files and train the model
for csv_file in glob.glob('data/*.csv'):
    
    print('Opening %s for training' % csv_file)

    # create a dataframe
    df = pd.read_csv(csv_file)

    # create a column for labels
    num_rows = len(df)
    is_spectre = 0
    if 'spectre' in csv_file:
        is_spectre = 1 
    df['spectre'] = np.full((num_rows, 1), is_spectre)

    # combine data
    df_all_data = pd.concat([df_all_data, df])

# shuffle data
df_all_data = df_all_data.sample(frac=1)
print('df_all_data', df_all_data)

# get training data (inputs into the neural network)
drop_data = []
for column in list(df):
    if column not in inputs:
        drop_data.append(column)

train_x = df.drop(columns=drop_data)
train_y = df['spectre']

# train the network!
model.fit(train_x, train_y, validation_split=0.2, epochs=30, callbacks=[early_stopping_monitor])
   
# Read in HPC data from .csv files and test the model
for csv_file in glob.glob('data/test/*.csv'):
   
    print('Opening %s for testing' % csv_file)
 
    df = pd.read_csv(csv_file)
    
    # get test data (inputs into the neural network)
    drop_data = []
    for column in list(df):
        if column not in inputs:
            drop_data.append(column)
    
    test_x = df.drop(columns=drop_data)
    
    # create labels
    labels = None
    num_rows = len(df)
    is_spectre = 0
    if 'spectre' in csv_file:
        is_spectre = 1 
    labels = np.full((num_rows, 1), is_spectre)
    
    test_y = pd.DataFrame(labels, columns=['spectre'])
    score = model.evaluate(test_x, test_y, verbose=1)
    print('Test accuracy: ', score[1])
