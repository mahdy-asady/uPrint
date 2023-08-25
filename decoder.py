import csv
import math

def readRecordsFromDB(fileName):
    records = {}
    with open(fileName, 'r') as csvFile:
        dataReader = csv.reader(csvFile)
        for row in dataReader:
            id = int(row.pop(0))
            records[id] = row
    return records

def getIdByteLength(dict):
    maxValue = max(dict.keys())
    return math.ceil(math.log2(maxValue) / 8)

##########################################################################
db = readRecordsFromDB('sample.db')
print(db)
idLength = getIdByteLength(db)
print("Bytes needed for ID field: " + str(idLength))
