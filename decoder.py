import csv
import math
import re
from collections import namedtuple

recordRow = namedtuple('Record', ['formatStr', 'specifiers'])


def popN(byteList, n):
    data = bytearray()
    for i in range(n):
        data.append(byteList.pop(0))
    return data


def readRecordsFromDB(fileName):
    records = {}
    with open(fileName, 'r') as csvFile:
        dataReader = csv.reader(csvFile)
        for row in dataReader:
            id = int(row.pop(0))
            records[id] = recordRow(row[0], re.findall(r'(%[^%])', row[0]))
    return records

def getIdByteLength(dict):
    maxValue = max(dict.keys())
    return math.ceil(math.log2(maxValue) / 8)

def decryptMessage(encMsg, idLength, db):
    keyIndex = int.from_bytes(popN(encMsg, idLength), 'little')
    dbRecord = db[keyIndex]
    print("Log Index: " + str(keyIndex))
    print("Print Text: " + dbRecord.formatStr)
    print(type(db[keyIndex]))

##########################################################################
db = readRecordsFromDB('sample.db')
print(db)
idLength = getIdByteLength(db)
print("Bytes needed for ID field: " + str(idLength))

sampleLog = bytearray(b'\x01\xFF\x00\x00\x00Salam\x00')
decryptMessage(sampleLog, idLength, db)
