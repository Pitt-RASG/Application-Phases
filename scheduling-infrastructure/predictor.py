#!/usr/bin/env python3
import sys
import numpy as np 
from pickle import load

def main():
    flag = True
    while flag:
        try:
            pmc1, pmc2, pmc3, pmc4, cluster = input().split(',') 
            p1 = int(pmc1)
            p2 = int(pmc2)
            p3 = int(pmc3)
            p4 = int(pmc4)
            p4= int(cluster)
            pmcs = [[p1, p2, p3, p4, cluster]]

            model = load(open("phases-model.pkl", "rb"))
            scaler = load(open("phases-scaler.pkl", "rb"))
            scaled_pmcs = scaler.transform(pmcs)
            print(scaled_pmcs)

            print(model.predict(scaled_pmcs))
            
        except EOFError:
            flag = False

if __name__ == "__main__":
    main()
