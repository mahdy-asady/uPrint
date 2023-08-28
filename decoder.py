import csv
import math
import re


def readRecordsFromDB(fileName):
    records = {}
    with open(fileName, 'r') as csvFile:
        dataReader = csv.reader(csvFile)
        for row in dataReader:
            id = int(row.pop(0))
            records[id] = (row[0], re.findall(r'(%[^%])', row[0]))
    return records

def getIdByteLength(dict):
    maxValue = max(dict.keys())
    return math.ceil(math.log2(maxValue) / 8)

def decryptMessage(encMsg, idLength, db):
    keyIndex = encMsg.pop(0)
    print("Log Index: " + str(keyIndex))
    print("Print Text: " + db[keyIndex][0])
    print(type(db[keyIndex]))

##########################################################################
db = readRecordsFromDB('sample.db')
print(db)
idLength = getIdByteLength(db)
print("Bytes needed for ID field: " + str(idLength))

sampleLog = bytearray(b'\x01\xFF\x00\x00\x00Salam\x00')
decryptMessage(sampleLog, idLength, db)
