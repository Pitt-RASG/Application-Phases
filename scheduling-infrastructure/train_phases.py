import pandas as pd
import numpy as np
from sklearn.svm import SVC
from sklearn.preprocessing import StandardScaler
from sklearn.model_selection import train_test_split
import pickle

# Read dataset
dataset = pd.read_csv('master_dataset.csv')
labels = dataset.iloc[:,-1:]
features = dataset.iloc[:,:-1]

# Split dataset into training and testing
X_train, X_test, y_train, y_test = train_test_split(features, labels, test_size=0.25)

# Scale data
scaler = StandardScaler()
scaler.fit(X_train)
X_train = scaler.transform(X_train)
X_test = scaler.transform(X_test)

# Train SVM model
model = SVC(kernel="linear")
model.fit(X_train, y_train.values.ravel())

# Test SVM model
test_score = model.score(X_test, y_test.values.ravel())
print(test_score)

# Output model to pickle file
pickle.dump(model, open("phases-model.pkl", "wb"))

# Output scalr to pickle file
pickle.dump(scaler, open("phases-scaler.pkl", "wb"))

